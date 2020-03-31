/*
  Copyright (c) 2019, Xilinx
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this
     list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.

  3. Neither the name of the copyright holder nor the names of its
     contributors may be used to endorse or promote products derived from
     this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#define AP_INT_MAX_W 8192
#include "ap_int.h"
#include "hls_stream.h"
#include "preres_custom.h"
#include "rpnn-library.h"
#include "config.h"
#include "params.h"
#include <iostream>
using namespace hls;

void preres(stream<ap_uint<L0_IFMC*L0_INBITS> > &input, stream<ap_uint<pool0_CHANNELS*pool0_PRECISION> > &output){
#pragma HLS INTERFACE axis register both port=output
#pragma HLS INTERFACE axis register both port=input
#pragma HLS INTERFACE ap_ctrl_none port=return
#include "pragma.h"
#pragma HLS DATAFLOW
    stream<ap_uint<L0_OFMC*L0_ACTBITS> > inter("inter");
#pragma HLS RESOURCE variable=inter core=FIFO_LUTRAM
#pragma HLS STREAM variable=inter depth=32
    ConvolutionalLayerMMV_Same_Batch_kernel_stride_dsp_packed<L0_KERNELDIM, L0_IFMC, L0_IFMDIM, L0_OFMC, L0_STRIDE, 
           L0_SIMD, L0_PE, L0_WMEM, L0_TMEM, L0_WBITS, L0_THBITS, L0_MACBITS, L0_INBITS, L0_ACTBITS, L0_MMV, FULL_THRESHOLDS>
                                        (input, inter, weights_conv0, thres_conv0, 1);

    MaxPoolStride_Same_Batch<pool0_IMDIM, pool0_KERNELDIM, pool0_STRIDE, pool0_CHANNELS, pool0_PRECISION, 1>(inter, output, 1);
}
