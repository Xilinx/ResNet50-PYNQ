# FPGA Inference from Python

We provide a Jupyter Notebook and Python script illustrating synthetic performance and power measurement of the ResNet50 accelerator, and Python-based inference code illustrating a multithreaded inference application.
All of the example code is pure Python, using PYNQ for accelerator memory allocation and interaction with the ResNet50 kernel in the Alveo card.

## Multithreaded Inference

The Python script `infer_pynq_threaded.py` is intended to provide an approximation of real-world inference workloads.
It utilizes multiple threads for preprocessing and accelerator invocation.
The configurable script parameters are:

Parameter            | Description                         									   | Default Value
-----------------    | -----------------                   									   | -----------------
--xclbin             | Path to accelerator xclbin file     									   | N/A
--fcweights          | Path to CSV file storing FC weights 									   | N/A
--img_path           | Path to image or image folder       									   | N/A
--max_bs             | Batch size (images processed per accelerator invocation                 | 1
--preprocess_workers | Number of preprocessing worker threads                                  | 1
--profile            | Captures timestamps at various points in the pipeline (beta)            | False
--outfile            | Output file                                                             | None, script prints to standard output
--labels             | File containing ground truth labels. If present, accuracy is calculated | None

The script will work with any image or folder of images in `JPEG` format.

### Tuning Performance

We recommend the following parameter settings when optimizing for low latency or high throughput respectively:

Optimization Target | Parameter Settings                   | Expected Latency (ms) | Expected Throughput (FPS)
-------             | ------------------                   | -------------         | -------------
Latency             | --max_bs 1 --preprocess_workers 4    | 2                     | 500
Throughput          | --max_bs 100 --preprocess_workers 16 | 50                    | 2000

### Calculating Accuracy

The script optionally takes a CSV file of expected predictions for each of the image(s) provided as input.
The format of the file is space-delimited, as follows, where `<filename>` denotes the name and extension of each input image (not the full path):

```
<filename> <predicted label> 
```

If provided with this file, the script will calculate the total accuracy on the input images.

## Synthetic Benchmark

The Python script `synth_bench_power.py` runs a synthetic benchmark on the accelerator, measuring the duration of the inference processing, without any data movement.
We measure latency, FPS, and also FPGA power and total board power using the PMBus module from PYNQ.
The configurable script parameters are:

Parameter            | Description                         									   | Default Value
-----------------    | -----------------                   									   | -----------------
--xclbin             | Path to accelerator xclbin file     									   | resnet50.xclbin
--fcweights          | Path to CSV file storing FC weights 									   | fcweights.csv
--shell              | Name of required Alveo shell                            | xilinx_u250_xdma_201830_2
--bs                 | Batch size                           									 | 1
--reps               | Number of batches                                       | 100

Some example invocations of the script:

```
user@host:/ResNet50-PYNQ/host$ python synth_bench_power.py --bs 1 --reps 100
Throughput: 495 FPS
Latency: 2.02 ms
FPGA Power: 17.15 Watts
Board Power: 30.18 Watts
user@host:/ResNet50-PYNQ/host$ python synth_bench_power.py --bs 100 --reps 100
Throughput: 1958 FPS
Latency: 51.06 ms
FPGA Power: 43.1 Watts
Board Power: 59.33 Watts
user@host:/ResNet50-PYNQ/host$ python synth_bench_power.py --bs 1000 --reps 100
Throughput: 2015 FPS
Latency: 496.14 ms
FPGA Power: 45.16 Watts
Board Power: 60.97 Watts
```
