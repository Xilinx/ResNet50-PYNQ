# Linking the ResNet50 Accelerator

Linking denotes the process of combining the accelerator circuit, build in the compile phase and packaged as a Vitis object file, `resnet50.xo`, and the Alveo shell to create a functional design.
We use *Vitis (2019.2)* for linking.

## Build Automation

To link the accelerator, use the provided `Makefile` with the following (optional) configuration parameters:

Parameter         | Possible Values                    | Default Value
----------------- | -----------------                  | -----------------
PLATFORM          | Any Vitis platform for Alveo U250  | xilinx_u250_xdma_201830_2
XO_INPUT          | Any path to a Vitis object file    | ../compile/resnet50.xo
XCLBIN_FREQ_MHZ   | Any integer value                  | 250
XCLBIN_OPTIMIZE   | 0, 1, 2, 3, s, quick               | Default value is 2. See Vitis documentation for optimization levels
FLOORPLAN         | Path to a floorplanning TCL script | floorplan.tcl (see below)
CHIPSCOPE         | Any Vitis Chipscope related option | No Chipscope insertion by default.
PROFILE_OPTS      | Any Vitis profiling option         | No profiling by default.

### Chipscope Debugging

Vitis enables the user to attach bus protocol analyzers to kernel interfaces. To do so for the ResNet50, users can set the `CHIPSCOPE` parameter to the appropriate values, as described in Vitis documentation.
For example, the following command builds the accelerator with Chipscope debug attached to all interfaces:

```
make CHIPSCOPE="--dk chipscope:$(KERNEL)_1:s_axi_control --dk chipscope:$(KERNEL)_1:m_axi_gmem0 --dk chipscope:$(KERNEL)_1:m_axi_gmem1"
```

### Profiling

To enable profiling of the ResNet50 kernel, pass the following string to PROFILE_OPTS during `make` invocation:

```
make PROFILE_OPTS="--profile_kernel data:all:all:all"
```

Note that enabling profiling reduces the achievable frequency of the ResNet50 design.

### Floorplanning

The *floorplan.tcl* script defines a floorplan for the ResNet50 by assigning each of the IPs in the accelerator structure to a specific SLR.
This is required to minimize the number of SLR crossings and ensure proper reset pipelining, both of which increase the top frequency of the design.


## Resource Utilization

The resource utilization on Alveo U250 is as follows:

Resource Type              |  Used  | Available | Utilization (%) 
---------------------------|--------|-----------|----------------
CLB LUTs                   | 986892 |   1727040 | 57.14 
CLB Registers              | 826784 |   3454080 | 23.94 
Block RAM Tile             | 1975   |      2688 | 73.47 
URAM                       |    0   |      1280 |  0.00 
DSPs                       | 1611   |     12288 | 13.11 
