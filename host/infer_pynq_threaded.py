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

def setup_accelerator(xclbin, wfile):
    ol=pynq.Overlay(xclbin)

    fcweights = np.genfromtxt(wfile, delimiter=',', dtype=np.int8)
    fcbuf = pynq.allocate((1000,2048), dtype=np.int8, target=ol.bank0)

    #csv reader erroneously adds one extra element to the end, so remove, then reshape
    fcweights = fcweights[:-1].reshape(1000,2048)
    fcbuf[:] = fcweights
    fcbuf.sync_to_device()

    return ol, fcbuf

def enumerate_images():
    global num_processed
    for f in files:
       yield f
       num_processed += 1
       if args.profile:
           start_ts_q.put(time.monotonic())

def process_image(filename):
    img = cv2.imread(filename)
    return cv2.resize(img, (224,224))

def consume_images():

    accelerator = ol.resnet50_1
    max_bs = args.max_bs

    #allocate 3 buffers for inputs and outputs; each buffer can accomodate the maximum batch
    input_buf = []
    output_buf = []
    buf_bs = [] #simple synchronization using a flag; if flag is >0 set buffer is in use and the value of the flag is the (actual) batch size
    timestamps = np.empty((3,max_bs,6))
    for i in range(3):
        input_buf += [pynq.allocate((max_bs,224,224,3), dtype=np.int8, target=ol.bank0)]
        output_buf += [pynq.allocate((max_bs,5), dtype=np.uint32, target=ol.bank0)]
        buf_bs += [-1]

    buf_idx = 0
    bs = 0
    none_count = 0
    sync_to_device = False
    last_idx = -1

    while True:
        #start inference on current batch (if it exists)
        if buf_bs[buf_idx] > 0:
            if args.profile:
                ts = time.monotonic()
                for i in range(buf_bs[buf_idx]):
                    timestamps[buf_idx][i][2] = ts
            context = accelerator.start(input_buf[buf_idx], output_buf[buf_idx], fcbuf, buf_bs[buf_idx])
        #get results of call on previous batch (if it exists)
        if buf_bs[buf_idx-1] > 0:
            if args.profile:
                ts = time.monotonic()
                for i in range(buf_bs[buf_idx-1]):
                    timestamps[buf_idx-1][i][4] = ts
            output_buf[buf_idx-1].sync_from_device()
            #put results in the output queue (after copying to np array)
            for i in range(buf_bs[buf_idx-1]):
                results_q.put(np.copy(output_buf[buf_idx-1][i]))
                if args.profile:
                    ts = time.monotonic()
                    timestamps[buf_idx-1][i][5] = ts
                    inference_ts_q.put(tuple(np.copy(timestamps[buf_idx-1][i])))
        if last_idx == (buf_idx+3-1)%3 and sync_to_device:
            return
        #assemble and upload next batch
        if not sync_to_device:
            bs=0
            for i in range(max_bs):
                image = next(images,None)
                if image is None:
                    last_idx = buf_idx if bs==0 else (buf_idx+1)%3
                    sync_to_device = True
                    continue
                if args.profile:
                    ts = time.monotonic()
                    timestamps[(buf_idx+1)%3][i][0] = ts
                input_buf[(buf_idx+1)%3][bs][:] = image
                bs += 1
            if bs > 0:
                input_buf[(buf_idx+1)%3].sync_to_device()
                buf_bs[(buf_idx+1)%3] = bs
                if args.profile:
                    ts = time.monotonic()
                    for i in range(buf_bs[(buf_idx+1)%3]):
                        timestamps[(buf_idx+1)%3][i][1] = ts
        #wait for inference on current batch
        if buf_bs[buf_idx] > 0:
            context.wait()
            if args.profile:
                ts = time.monotonic()
                for i in range(buf_bs[buf_idx]):
                    timestamps[buf_idx][i][3] = ts
        #increment buffer index
        buf_idx = (buf_idx+1)%3

def log_results():

    results = pd.DataFrame({'File Name':files})
    results['Prediction'] = ""
    if args.profile:
        results['Start TS'] = float('nan')
        results['Upload Start TS'] = float('nan')
        results['Upload End TS'] = float('nan')
        results['Inference Start TS'] = float('nan')
        results['Inference End TS'] = float('nan')
        results['Download Start TS'] = float('nan')
        results['Download End TS'] = float('nan')

    #pull from the timestamp queues and add to the dataframe
    for i in range(len(files)):
        results.at[results.index[i],'Prediction'] = results_q.get()
        if args.profile:
            results.at[results.index[i],'Start TS'] = start_ts_q.get()
            ul_s_ts, ul_e_ts, i_s_ts, i_e_ts, dl_s_ts, dl_e_ts = inference_ts_q.get()
            results.at[results.index[i],'Upload Start TS'] = ul_s_ts
            results.at[results.index[i],'Upload End TS'] = ul_e_ts
            results.at[results.index[i],'Inference Start TS'] = i_s_ts
            results.at[results.index[i],'Inference End TS'] = i_e_ts
            results.at[results.index[i],'Download Start TS'] = dl_s_ts
            results.at[results.index[i],'Download End TS'] = dl_e_ts

    if args.profile:
        #compute compound measures
        results['Total Latency'] = results['Download End TS'] - results['Start TS']
        results['Preprocessing Time'] = results['Upload Start TS'] - results['Start TS']
        results['Upload Time'] = results['Upload End TS'] - results['Upload Start TS']
        results['Accelerator Time'] = results['Inference End TS'] - results['Inference Start TS']
        results['Download Time'] = results['Download End TS'] - results['Download Start TS']
        results['Inference Time'] = results['Download End TS'] - results['Upload Start TS']

    if args.outfile != None:
        results.to_csv(args.outfile)
    else:
        print(results)

    if args.profile:
        print(results[['Total Latency','Preprocessing Time','Upload Time','Accelerator Time','Download Time','Inference Time']].describe())

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
parser.add_argument('--xclbin', type=str, default=None, help='Accelerator image file (xclbin)', required=True)
parser.add_argument('--fcweights', type=str, default=None, help='FC weights file (CSV)', required=True)
parser.add_argument('--img_path', type=str, default=None, help='Path to image or image folder', required=True)
parser.add_argument('--max_bs', type=int, default=1, help='Batch size (images processed per accelerator invocation)')
parser.add_argument('--preprocess_workers', type=int, default=1, help='Number of workers used for preprocessing')
parser.add_argument('--profile', action='store_true', default=False, help='Captures timestamps at various points in the pipeline')
parser.add_argument('--outfile', type=str, default=None, help='File to dump outputs', required=False)
parser.add_argument('--labels', type=str, default=None, help='File containing ground truth labels. If present, accuracy is calculated', required=False)
args = parser.parse_args()

ol, fcbuf = setup_accelerator(args.xclbin, args.fcweights)

#determine if we're processing a file or folder
if isfile(args.img_path):
    files = [args.img_path]
else:
    files = [join(args.img_path,f) for f in listdir(args.img_path) if isfile(join(args.img_path,f))]

#set up queues and processes
inference_ts_q = Queue(len(files))
start_ts_q = Queue(len(files))
results_q = Queue(len(files))

num_processed = 0

pool = ThreadPoolExecutor(max_workers=args.preprocess_workers)

starttime = time.monotonic()

images = pool.map(process_image, enumerate_images())

threads = []
threads.append(threading.Thread(target=consume_images))
threads.append(threading.Thread(target=log_results))

[t.start() for t in threads]
[t.join() for t in threads]

endtime = time.monotonic()

print("Duration for ",num_processed," images: ",endtime - starttime)
print("FPS: ",num_processed / (endtime - starttime))





