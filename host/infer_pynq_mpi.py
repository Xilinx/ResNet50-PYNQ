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
import pandas as pd
from mpi4py import MPI
from multiprocessing import Pool

def setup_accelerator(xclbin, wfile, bs):

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
    
    #allocate buffers for inputs, outputs
    if ol.device.name == 'xilinx_u250_xdma_201830_2':
        print(str(rank)+": allocate IO memory for U250")
        input_buf = pynq.allocate((bs,224,224,3), dtype=np.int8, target=ol.bank0)
        output_buf = pynq.allocate((bs,5), dtype=np.uint32, target=ol.bank0)
    elif ol.device.name == 'xilinx_u280_xdma_201920_3':
        print(str(rank)+": allocate IO memory for U280")
        input_buf = pynq.allocate((bs,224,224,3), dtype=np.int8, target=ol.HBM0)
        output_buf = pynq.allocate((bs,5), dtype=np.uint32, target=ol.HBM6)
    else:
        raise Exception("Unexpected device")
    
    accelerator = ol.resnet50_1
    print("Accelerator ready!")

    return accelerator, fcbuf, input_buf, output_buf

def preprocess_image(filename):
    # preprocesses one image
    # JPEG decoding -> resize
    img = cv2.imread(filename)
    img = cv2.resize(img, (224,224))
    return img

def preprocess_minibatch(bs, filenames, workers):
    # processes a batch using multiple workers
    # returns a list of numpy arrays representing one image each
    # this needs vstacking!
    with Pool(processes=workers) as pool:
        return np.stack(pool.map(preprocess_image, filenames))

def infer_images(accelerator, input_buf, output_buf, fcbuf, bs, minibatch):
    input_buf[:] = minibatch
    input_buf.sync_to_device()
    context = accelerator.start(input_buf, output_buf, fcbuf, bs)
    context.wait()
    output_buf.sync_from_device()
    return np.copy(output_buf)

def log_results(df, start_idx, results):
    # get rid of outermost dimensions
    results = results.reshape((-1, 5))
    for i in range(len(results)):
        df.at[df.index[start_idx+i],'Prediction'] = np.copy(results[i])


parser = argparse.ArgumentParser(description='ResNet50 inference with FINN and PYNQ on Alveo')
parser.add_argument('--xclbin', type=str, nargs='+', default=None, help='Accelerator image file (xclbin)', required=True)
parser.add_argument('--fcweights', type=str, default=None, help='FC weights file (CSV)', required=True)
parser.add_argument('--img_path', type=str, default=None, help='Path to image or image folder', required=True)
parser.add_argument('--bs', type=int, default=1, help='Batch size (images processed per accelerator invocation)')
parser.add_argument('--preprocess_workers', type=int, default=1, help='Number of workers used for preprocessing')
parser.add_argument('--outfile', type=str, default=None, help='File to dump outputs', required=False)
parser.add_argument('--labels', type=str, default=None, help='File containing ground truth labels. If present, accuracy is calculated', required=False)
args = parser.parse_args()

comm = MPI.COMM_WORLD
rank = comm.Get_rank()
world_size = comm.Get_size()

accelerator = None
fcbuf = None
input_buf = None
output_buf = None
if rank != 0:
    accelerator, fcbuf, input_buf, output_buf = setup_accelerator(args.xclbin, args.fcweights, args.bs)

nfiles = np.empty(1, dtype=np.int)

if rank == 0:
    #determine if we're processing a file or folder
    if isfile(args.img_path):
        files = [args.img_path]
    else:
        files = [join(args.img_path,f) for f in listdir(args.img_path) if isfile(join(args.img_path,f))]

    nfiles = len(files)
    print(str(nfiles)+" to process")

    num_processed = 0
    
    results = pd.DataFrame({'File Name':files})
    results['Prediction'] = ""

batch = np.empty((world_size, args.bs, 224, 224, 3), dtype=np.int8)
minibatch = np.empty((args.bs, 224, 224, 3), dtype=np.int8)
result_batch = np.empty((world_size, args.bs, 5), dtype=np.int32)
result_minibatch = np.empty((args.bs, 5), dtype=np.int32)
result = np.empty((5,), dtype=np.int32)



nfiles = comm.bcast(nfiles, root=0)
num_processed = (world_size-1)*args.bs*(nfiles//((world_size-1)*args.bs))
print(nfiles, num_processed)
#wait for every rank to finish programming their accelerators
comm.Barrier()
starttime = time.monotonic()


# for all images, scatter, infer, gather
nimages = 0
while nimages < num_processed:
    if rank == 0:
        #assemble a batch from minibatches
        for i in range(world_size-1):
            batch[i+1] = preprocess_minibatch(args.bs, files[nimages+i*args.bs:nimages+(i+1)*args.bs], args.preprocess_workers)
    # on all ranks, scatter
    comm.Scatter(batch, minibatch, root=0)
    # on non-zero ranks, infer
    if rank != 0:
        result_minibatch = infer_images(accelerator, input_buf, output_buf, fcbuf, args.bs, minibatch)
    # on all ranks, gather
    comm.Gather(result_minibatch, result_batch, root=0)
    # on rank 0, log results
    if rank == 0:
        log_results(results, nimages, result_batch[1:])
    nimages += (world_size-1)*args.bs

if rank == 0:
    endtime = time.monotonic()
    print("Duration for ",num_processed," images: ",endtime - starttime)
    print("FPS: ",num_processed / (endtime - starttime))

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

print(str(rank)+": Done")
MPI.Finalize()
