#  Copyright (c) 2019, Xilinx
#  All rights reserved.
#  
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#  
#  1. Redistributions of source code must retain the above copyright notice, this
#     list of conditions and the following disclaimer.
#  
#  2. Redistributions in binary form must reproduce the above copyright notice,
#     this list of conditions and the following disclaimer in the documentation
#     and/or other materials provided with the distribution.
#  
#  3. Neither the name of the copyright holder nor the names of its
#     contributors may be used to endorse or promote products derived from
#     this software without specific prior written permission.
#  
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
#  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
#  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
#  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
#  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
#  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import os
import sys
import pynq
import numpy as np
import cv2
import argparse
from os import listdir
from os.path import isfile, join, split
import time
import threading
from queue import Queue
from concurrent.futures import ThreadPoolExecutor
import pandas as pd
from mpi4py import MPI

def setup_accelerator(xclbin, wfile):

    localrank = os.getenv('OMPI_COMM_WORLD_LOCAL_RANK')
    if not localrank:
        raise Exception("OpenMPI required")    
    local_alveo = pynq.Device.devices[int(localrank)%len(pynq.Device.devices)]
    print("Rank "+localrank+": device "+local_alveo.name)
    
    fpga_configured = False
    i = 0
    ol = None
    while not fpga_configured and i<len(xclbin):
        try:
            ol=pynq.Overlay(xclbin[i], device=local_alveo)
            print("FPGA configured!")
            fpga_configured = True
        except:
            print("XCLBIN does not match device, trying next")
            i += 1

    if ol is None:
        print("FPGA not configured, no viable XCLBIN for platform "+local_alveo.name)
        sys.exit()

    print("Programming accelerator weights")
    if local_alveo.name == 'xilinx_u250_xdma_201830_2':
        fcbuf = pynq.allocate((1000,2048), dtype=np.int8, target=ol.PLRAM0)
    elif local_alveo.name == 'xilinx_u280_xdma_201920_3':
        fcbuf = pynq.allocate((1000,2048), dtype=np.int8, target=ol.HBM18)

    #csv reader erroneously adds one extra element to the end, so remove, then reshape
    fcweights = np.genfromtxt(wfile, delimiter=',', dtype=np.int8)
    fcweights = fcweights[:-1].reshape(1000,2048)
    fcbuf[:] = fcweights
    fcbuf.sync_to_device()
    print("Accelerator ready!")

    return ol, fcbuf

def enumerate_images():
    global num_processed
    for f in files:
       yield f
       num_processed += 1

def process_image(filename):
    img = cv2.imread(filename)
    return cv2.resize(img, (224,224))

def scatter_images():
    #in rank 0, get image, assemble a batch equal to comm world size
    batch = np.empty((world_size, 224, 224, 3), dtype=np.int8)
    img = np.empty((224, 224, 3), dtype=np.int8)
    nimages = 0
    while nimages < nfiles//(world_size-1):
        if rank == 0:
            for i in range(world_size-1):
                img = next(images, None)
                #import pdb; pdb.set_trace()
                if img is None:
                    print(str(rank)+": Scatter done")
                    return
                else:
                    batch[i][:] = img
        # on all ranks, scatter and yield
        comm.Scatter(batch, img, root=0)
#        print("Put image "+str(nimages))
        if rank != 0:
            images_q.put(img)
        nimages += 1
    images_q.put(None)
    print(str(rank)+": Scatter done")

def consume_images():

    accelerator = ol.resnet50_1
    max_bs = args.max_bs

    #allocate 3 buffers for inputs and outputs; each buffer can accomodate the maximum batch
    input_buf = []
    output_buf = []
    buf_bs = [] #simple synchronization using a flag; if flag is >0 set buffer is in use and the value of the flag is the (actual) batch size

    for i in range(3):
        if ol.device.name == 'xilinx_u250_xdma_201830_2':
            print(str(rank)+": allocate IO memory for U250")
            input_buf += [pynq.allocate((max_bs,224,224,3), dtype=np.int8, target=ol.bank0)]
            output_buf += [pynq.allocate((max_bs,5), dtype=np.uint32, target=ol.bank0)]
        elif ol.device.name == 'xilinx_u280_xdma_201920_3':
            print(str(rank)+": allocate IO memory for U280")
            input_buf += [pynq.allocate((max_bs,224,224,3), dtype=np.int8, target=ol.HBM0)]
            output_buf += [pynq.allocate((max_bs,5), dtype=np.uint32, target=ol.HBM6)]
        else:
            raise Exception("Unexpected device")
            
        buf_bs += [-1]

    buf_idx = 0
    bs = 0
    none_count = 0
    sync_to_device = False
    last_idx = -1

    print(str(rank)+": Starting inference")
    print(str(rank)+": Max BS = "+str(max_bs))

    while True:
        #start inference on current batch (if it exists)
        if buf_bs[buf_idx] > 0:
            context = accelerator.start(input_buf[buf_idx], output_buf[buf_idx], fcbuf, buf_bs[buf_idx])
        #get results of call on previous batch (if it exists)
        if buf_bs[buf_idx-1] > 0:
            output_buf[buf_idx-1].sync_from_device()
            #put results in the output queue (after copying to np array)
            for i in range(buf_bs[buf_idx-1]):
                results_q.put(np.copy(output_buf[buf_idx-1][i]))
#                print("Put result")
        if last_idx == (buf_idx+3-1)%3 and sync_to_device:
            results_q.put(None)
#            print(str(rank)+": Inference done")
            return
        #assemble and upload next batch
        if not sync_to_device:
            bs=0
            for i in range(max_bs):
                image = images_q.get()#next(scattered_images,None)
#                print("Get image")
                if image is None:
                    last_idx = buf_idx if bs==0 else (buf_idx+1)%3
                    sync_to_device = True
                    continue
                input_buf[(buf_idx+1)%3][bs][:] = image
                bs += 1
            if bs > 0:
                input_buf[(buf_idx+1)%3].sync_to_device()
                buf_bs[(buf_idx+1)%3] = bs
        #wait for inference on current batch
        if buf_bs[buf_idx] > 0:
            context.wait()
        #increment buffer index
        buf_idx = (buf_idx+1)%3


def gather_results():
    results = np.empty((world_size, 5), dtype=np.int32)
    result = np.empty((5,), dtype=np.int32)
    result_count = 0
    while True:
        if rank != 0:
            try:
                result = results_q.get_nowait()
            except:
                continue
            if result is None:
                print(str(rank)+": Gather done")
                return
#        print(str(rank)+" Gather result")
        comm.Gather(result, results, root=0)
        if rank == 0:
            for i in range(world_size-1):
#                print(str(rank)+": enqueue result ",i)
                gathered_results_q.put(np.copy(results[i+1]))
            result_count += world_size-1
            if result_count == (len(files)//(world_size-1))*(world_size-1):
                print(str(rank)+": Gather done")
                return


def log_results():

    results = pd.DataFrame({'File Name':files})
    results['Prediction'] = ""

    print("Expecting ",(len(files)//(world_size-1))*(world_size-1)," results")

    #pull from the timestamp queues and add to the dataframe
    for i in range((len(files)//(world_size-1))*(world_size-1)):
#        print("Register result")
        results.at[results.index[i],'Prediction'] = gathered_results_q.get()

    if args.outfile != None:
        results.to_csv(args.outfile)
    else:
        print(results)

    if args.labels != None:
        labels = pd.read_csv(args.labels, sep=' ', header=None, index_col=0, squeeze=True).to_dict()
        top1_count = 0
        top5_count = 0
        total_count = 0
        for index, row in results.iterrows():
            fpath, fname = split(row['File Name'])
            true_label = labels.get(fname,None)
            predicted_labels = row['Prediction']
            if true_label != None:
                total_count += 1
                if true_label == predicted_labels[0]:
                    top1_count += 1
                if true_label in predicted_labels:
                    top5_count += 1
        print("Top-1 Accuracy:",top1_count/total_count)
        print("Top-5 Accuracy:",top5_count/total_count)

parser = argparse.ArgumentParser(description='ResNet50 inference with FINN and PYNQ on Alveo')
parser.add_argument('--xclbin', type=str, nargs='+', default=None, help='Accelerator image file (xclbin)', required=True)
parser.add_argument('--fcweights', type=str, default=None, help='FC weights file (CSV)', required=True)
parser.add_argument('--img_path', type=str, default=None, help='Path to image or image folder', required=True)
parser.add_argument('--max_bs', type=int, default=1, help='Batch size (images processed per accelerator invocation)')
parser.add_argument('--preprocess_workers', type=int, default=1, help='Number of workers used for preprocessing')
parser.add_argument('--outfile', type=str, default=None, help='File to dump outputs', required=False)
parser.add_argument('--labels', type=str, default=None, help='File containing ground truth labels. If present, accuracy is calculated', required=False)
args = parser.parse_args()

comm = MPI.COMM_WORLD
rank = comm.Get_rank()
world_size = comm.Get_size()

ol = None
fcbug = None
if rank != 0:
    ol, fcbuf = setup_accelerator(args.xclbin, args.fcweights)

nfiles = np.empty(1, dtype=np.int)

#set up queues and processes
images_q = Queue(10000)
results_q = Queue(10000)
if rank == 0:
    gathered_results_q = Queue(10000)

if rank == 0:
    #determine if we're processing a file or folder
    if isfile(args.img_path):
        files = [args.img_path]
    else:
        files = [join(args.img_path,f) for f in listdir(args.img_path) if isfile(join(args.img_path,f))]

    nfiles = len(files)
    print(str(nfiles)+" to process")

    num_processed = 0

    pool = ThreadPoolExecutor(max_workers=args.preprocess_workers)

    images = pool.map(process_image, enumerate_images())

#wait for every rank to finish programming their accelerators
comm.Barrier()

starttime = time.monotonic()
nfiles = comm.bcast(nfiles, root=0)

scattered_images = scatter_images()

threads = []
threads.append(threading.Thread(target=scatter_images))
if rank != 0:
    threads.append(threading.Thread(target=consume_images))
threads.append(threading.Thread(target=gather_results))
if rank == 0:
    threads.append(threading.Thread(target=log_results))

[t.start() for t in threads]
[t.join() for t in threads]

if rank == 0:
#    log_results()
    endtime = time.monotonic()
    print("Duration for ",num_processed," images: ",endtime - starttime)
    print("FPS: ",num_processed / (endtime - starttime))

print(str(rank)+": Done")
MPI.Finalize()
