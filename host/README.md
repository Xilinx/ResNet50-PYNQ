# FPGA Inference from Python

We provide a Jupyter Notebook illustrating synthetic performance and power measurement of the ResNet50 accelerator, and Python-based inference code illustrating a multithreaded inference application.
All of the example code is pure Python, using PYNQ for accelerator memory allocation and interaction with the ResNet50 kernel in the Alveo card.

## Multithreaded Inference

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
Throughput          | --max_bs 100 --preprocess_workers 16 | 200                   | 2000

### Calculating Accuracy

The script optionally takes a CSV file of expected predictions for each of the image(s) provided as input.
The format of the file is space-delimited, as follows, where `<filename>` denotes the name and extension of each input image (not the full path):

```
<filename> <predicted label> 
```

If provided with this file, the script will calculate the total accuracy on the input images.
