# FPGA Inference from Python

We provide a Jupyter Notebook and Python script illustrating synthetic performance and power measurement of the ResNet50 accelerator, and Python-based inference code illustrating a multithreaded inference application.
All of the example code is pure Python, using PYNQ for accelerator memory allocation and interaction with the ResNet50 kernel in the Alveo card.
Before running any of the Python applications, make sure to have downloaded the weights file for the fully connected layer with:
```
wget -O fcweights.csv https://www.xilinx.com/bin/public/openDownload?filename=resnet50pynq.fcweights.csv
```

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

Optimization Target | Parameter Settings                   
-------             | ------------------                   
Latency             | --max_bs 1 --preprocess_workers 4    
Throughput          | --max_bs 100 --preprocess_workers 16 

### Demo command 

```
python infer_pynq_threaded.py --xclbin ./resnet50.xclbin --fcweights ./fcweights.csv --img_path ./pictures --max_bs 1 --preprocess_workers 4 --outfile ./out.txt --labels labels.pkl
```

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

### Demo command 

```
python synth_bench_power.py --xclbin ./resnet50.xclbin --fcweights ./fcweights.csv --shell xilinx_u250_xdma_201830_2 --bs 1 --reps 100
```

Some performance measurements for 100 repetitions:

Batch size       | 1              | 10             | 100            | 1000           |
---------------  |--------------- |--------------- |--------------- |--------------- |
Throughput (FPS) | 527            | 1895           | 2605           | 2703           |
Latency (ms)     | 1.9            | 5.3            | 38.4           | 369.9          |
FPGA Power (W)   | 16.6           | 33.7           | 50.6           | 54.6           |
Board Power (W)  | 29.5           | 50.4           | 68.8           | 70.8           |
