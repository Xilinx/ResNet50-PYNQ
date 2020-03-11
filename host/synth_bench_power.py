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
from pynq import pmbus
from pynq import Device

import numpy as np
import pandas as pd
import time
import argparse

# Set up data acquisition using PYNQ's PMBus API
def setup_power_recording():
    rails = pmbus.get_xrt_sysfs_rails()

    #We create a recorder monitoring the three rails that have power measurement on Alveo. 
    #Total board power is obtained by summing together the PCI Express and Auxilliary 12V rails. 
    #While some current is also drawn over the PCIe 5V rail this is negligible compared to the 12V rails and isn't recorded. 
    #We also measure the VCC_INT power which is the primary supply to the FPGA.
    recorder = pmbus.DataRecorder(rails["12v_aux"].power,
                                  rails["12v_pex"].power,
                                  rails["vccint"].power)

    return recorder


# ## Synthetic Throughput Test
# We execute inference of a configurable-size batch of images, without data movement. We measure the latency, throughput, and power
def benchmark_synthetic(bs, nreps):
    ibuf = pynq.allocate((bs,3,224,224), dtype=np.int8, target=ol.bank0)
    obuf = pynq.allocate((bs,5), dtype=np.uint32, target=ol.bank0)

    # Start power monitoring
    pwr_rec = setup_power_recording()
    pwr_rec.record(0.1)

    total_duration = time.monotonic()
    for i in range(nreps):
        accelerator.call(ibuf, obuf, fcbuf, bs)
    total_duration = time.monotonic() - total_duration

    # Stop the power monitoring
    pwr_rec.stop()

    latency = total_duration/nreps
    fps = int((nreps/total_duration)*bs)

    # Aggregate board/fpga power into a Pandas dataframe
    f = pwr_rec.frame
    powers = pd.DataFrame(index=f.index)
    powers['board_power'] = f['12v_aux_power'] + f['12v_pex_power']
    powers['fpga_power'] = f['vccint_power']

    return fps, latency, powers

if __name__== "__main__":

    parser = argparse.ArgumentParser(description='ResNet50 inference with FINN and PYNQ on Alveo')
    parser.add_argument('--xclbin', type=str, default='resnet50.xclbin', help='Accelerator image file (xclbin)')
    parser.add_argument('--fcweights', type=str, default='fcweights.csv', help='FC weights file (CSV)')
    parser.add_argument('--shell', type=str, default='xilinx_u250_xdma_201830_2', help='Name of compatible shell')
    parser.add_argument('--bs', type=int, default=1, help='Batch size (images processed per accelerator invocation)')
    parser.add_argument('--reps',type=int, default=100, help='Number of batches to run')
    args = parser.parse_args()

    # discover a compatible shell if there are multiple
    devices = Device.devices
    if len(devices) > 1:
        for i in range(len(devices)):
            print("{}) {}".format(i, devices[i].name))
            if devices[i].name == args.shell:
                print("Compatible shell found, using device",i)
                Device.active_device = devices[i]
                break

    ol=pynq.Overlay(args.xclbin)
    accelerator=ol.resnet50_1

    #allocate a buffer for FC weights, targeting the Alveo DDR Bank 0
    fcbuf = pynq.allocate((1000,2048), dtype=np.int8, target=ol.bank0)

    # Load the weight from a CSV file and push them to the accelerator buffer:
    fcweights = np.genfromtxt(args.fcweights, delimiter=',', dtype=np.int8)
    #csv reader erroneously adds one extra element to the end, so remove, then reshape
    fcweights = fcweights[:-1].reshape(1000,2048)
    fcbuf[:] = fcweights

    #Move the data to the Alveo DDR
    fcbuf.sync_to_device()

    fps, latency, power = benchmark_synthetic(args.bs,args.reps)

    print("Throughput:",fps,"FPS")
    print("Latency:",round(latency*1000,2),"ms")
    print("FPGA Power:",round(power.mean()['fpga_power'],2),"Watts")
    print("Board Power:",round(power.mean()['board_power'],2),"Watts")

