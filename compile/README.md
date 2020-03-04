# Compiling the ResNet50 Accelerator

The accelerator is constructed from FINN-style convolutional layers (MVTUs) in [FINN-HLSLib](https://github.com/Xilinx/finn-hlslib) and [FINN](https://github.com/Xilinx/finn) (version 0.1), as well as some custom layers.
The compilation process converts the HLS-compatible C++ definition of the accelerator into a Vitis object file which can subsequently be linked into an Alveo platform.

## Accelerator Structure

The accelerator has a modular structure consisting of several IPs connected in a pipeline:
* One DMA IP providing IO functionality for input images, classification outputs, and weight reads for the FC layer
* One processing IP per residual block, each implementing either 3 or 4 convolutional layers plus elementwise addition
* One IP implementing the layers before the first residual block (top layers)
* One IP implementing the layers after the last residual block, including the FC layer (bottom layers)

This modular structure enables parallel synthesis in Vivado and Vivado-HLS, which reduces the compilation time.

## Build Automation

To compile the accelerator, use the provided `Makefile` with the following (optional) configuration parameters:

Parameter         | Possible Values          | Default Value
----------------- | -----------------        | -----------------
DEFAULT_FREQ_MHZ  | Any integer value        | 250
DEVICE            | Any Xilinx device        | xcu250-figd2104-2L-e
COMMAND           | sim, syn, ip, cosim, all | ip
NET               | w1a2_v1.0                | w1a2_v1.0

The `COMMAND` parameter configures the Vivado HLS flow for each of the accelerator sub-IPs, as follows:
* **sim** - Runs only C simulation of the IP blocks. Intended for functional verification of C++ code
* **syn** - Runs synthesis only. Intended for analyzing resource utilization
* **ip** - Synthesizes and exports IPs. The default flow
* **cosim** - Runs a cosimulation of the synthesized code. Intended for functional verification of generated RTL code
* **all** - Runs all components of the flow (Csim, synthesis, Cosim, export)

The `NET` parameter indicates which version of the ResNet50 we should build.
Currently, the only supported version is 1-bit weights, 2- and 4-bit activations, using the original ResNet50 topology (i.e. not the updated v1.5 topology).
This configuration is denoted `w1a2_v1.0`. Additional configurations may be added later.

## Build Flow

The following briefly describes the steps of the build flow:

### IP-Level HLS Synthesis

The IPs are synthesized with Vivado HLS.
The TCL scripts `resblock.tcl`, `inoutdma.tcl`, `preres.tcl`, `postres.tcl` correspond to the IPs for residual blocks, DMA, top layers and bottom layers respectively.

If simulation (csim or cosim) is part of the flow (i.e. `COMMAND` was set to `sim`, `cosim`, or `all`) then simulation input/output files need to be downloaded before the build is started. From the repo root, do the following:

```
cd src/
wget -O resnet50sim.tgz https://www.xilinx.com/bin/public/openDownload?filename=resnet50sim.tgz
tar -xf resnet50sim.tgz
cd w1a2_v1.0/outpipe/
wget -O fcweights.csv https://www.xilinx.com/bin/public/openDownload?filename=resnet50pynq.fcweights.csv
```

### Block Design Assembly

After IPs are constructed, a block design is assembled in Vivado IP Integrator.
The `ipi.tcl` script instructs Vivado to instantiate and connect all IPs appropriately using AXI-Stream interfaces.
Where necessary, AXI Data Width Converters are instantiated between IPs to widen or narrow the bus width.
Finally, the block design is synthesized.

### Object File Generation

A DCP is exported from the synthesized block design as an IP, and the IP is converted to a Vitis object file.
For this, the `package_ip.tcl`, `package_dcp.tcl`, and `gen_xo.tcl` scripts are called in sequence from Vivado.
The programming interface of the object file is defined in `kernel.xml`.
