# Quantized ResNet50 Dataflow Acceleration on Alveo

This repository contains an implementations of a binary ResNet50 [FINN](https://github.com/Xilinx/finn)-style dataflow accelerator targeting Alveo boards.
It is intended as a showcase of achievable throughput and latency for ImageNet clasiffication on FPGA, using dataflow execution and on-chip weight storage.

## Repo organization

The repository is organized as follows:
- **src**: contains source code and submodules
  - **hls**: HLS custom building blocks and submodules to FINN librares ([FINN](https://github.com/Xilinx/finn) and [FINN-HLSLib](https://github.com/Xilinx/finn-hlslib))
  - **w1a2-v1.0**: pre-build weights, thresholds, directives and configuration files for Binary ResNet50
- **compile**: contains scripts for accelerator compilation (Vivado HLS CSynth + Vivado Synthesis)
- **link**: contains scripts for accelerator linking into the Alveo platform with Vitis
- **host**: python and Jupyter host code, using PYNQ for Alveo

## Building the Accelerator

The Accelerator is built using *Vitis 2019.2*. We recommend using this version,
otherwise changes might be required to source and/or Makefiles for things to 
work.

To build the accelerator, clone the repository (using `--recursive` to pull submodules), after which:

```console
cd ResNet50-PYNQ/compile
make NET=w1a2_v1.0
cd ../link
make
```

See the specific [Compile](./compile/README.md) and [Link](./link/README.md) documentation for further info.

## Running the Demo

After you have built the accelerator, you can `install` the required files in
the `host` folder. Move in the cloned repo and do `make install`

```console
cd ResNet50-PYNQ
make install
```

You can then run the included Jupyter notebook or the Python multithreaded inference example. If you 
want to use the distributed PYNQ python package, please read below.
If you want to run example Python inference code, please see the [host code](./host/README.md) documentation.

## PYNQ quick start

Install the `resnet50-pynq` package using `pip`:
   ```bash
   pip install resnet50-pynq
   ```

After the package is installed, to get your own copy of the available notebooks 
run:
   ```bash
   pynq get-notebooks ResNet50
   ```

You can then try things out by doing:
   ```bash
   cd pynq-notebooks
   jupyter notebook
   ```

There are a number of additional options for the `pynq get-notebooks` command,
you can list them by typing 
   ```bash
   pynq get-notebooks --help
   ```

You can also refer to the official 
[PYNQ documentation](https://pynq.readthedocs.io/) for more information 
regarding the *PYNQ Command Line Interface* and in particular the 
`get-notebooks` command.

### Supported Boards/Shells

Currently, we distribute the overlay only for the following Alveo boards and 
shells:

Shell                    | Board             
-------------------------|-----------------
xilinx_u250_xdma_201830_2|Xilinx Alveo U250

Designs are built using *Vitis 2019.2*.

## Author

Lucian Petrica @ Xilinx Research Labs.
