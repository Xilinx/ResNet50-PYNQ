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
#include "bnn-library.h"
#include "activations.hpp"
#include "weights.hpp"
#include "interpret.hpp"
#include "outpipe/config.h"
#include "res5c/config.h"
#include <iostream>
#include "outpipe/params.h"
using namespace hls;

void postres(stream<ap_uint<RES_2C_PE*RES_2C_ACTBITS> > &input, stream<ap_uint<32> > &output, stream<ap_uint<L53_SIMD*L53_PE*L53_WBITS> > &weights){
#pragma HLS INTERFACE axis register both port=output
#pragma HLS INTERFACE axis register both port=input
#pragma HLS INTERFACE axis register both port=weights
#pragma HLS INTERFACE ap_ctrl_none port=return
#include "outpipe/pragma.h"
#pragma HLS DATAFLOW
    stream<ap_uint<RES_2C_PE*RES_2C_ACTBITS> > reluout("reluout");
#pragma HLS RESOURCE variable=reluout core=FIFO_LUTRAM
#pragma HLS STREAM variable=reluout depth=64
    stream<ap_uint<RES_2C_PE*L53_INBITS> > poolout("poolout");
#pragma HLS RESOURCE variable=poolout core=FIFO_LUTRAM
#pragma HLS STREAM variable=poolout depth=64
    stream<ap_uint<L53_SIMD*L53_INBITS> > fcin("fcin");
#pragma HLS RESOURCE variable=fcin core=FIFO_LUTRAM
#pragma HLS STREAM variable=fcin depth=64
    stream<ap_uint<L53_PE*L53_ACTBITS> > fcout("fcout");
#pragma HLS RESOURCE variable=fcout core=FIFO_LUTRAM
#pragma HLS STREAM variable=fcout depth=64
    stream<ap_uint<L53_PE*L53_ACTBITS> > bias("bias");
#pragma HLS RESOURCE variable=bias core=FIFO_LUTRAM
#pragma HLS STREAM variable=bias depth=64
    stream<ap_uint<L53_PE*L53_ACTBITS> > fcwbias("fcwbias");
#pragma HLS RESOURCE variable=fcwbias core=FIFO_LUTRAM
#pragma HLS STREAM variable=fcwbias depth=64
ReLU_Batch<RES_2C_OFMDIM, RES_2C_OFMC, ap_uint<RES_2C_ACTBITS>, RES_2C_PE, 8>(input, reluout, 1);
AccPool_Batch<RES_2C_OFMDIM, RES_2C_OFMC, ap_uint<RES_2C_ACTBITS>, RES_2C_PE, ap_uint<L53_INBITS>>(reluout, poolout, 1);
StreamingDataWidthConverter_Batch<RES_2C_PE*L53_INBITS, L53_SIMD*L53_INBITS, RES_2C_OFMC/RES_2C_PE>(poolout, fcin,1);

Matrix_Vector_Activate_Stream_Batch<L53_MW, L53_MH, L53_SIMD, L53_PE, Slice<ap_int<L53_INBITS>>, Slice<ap_int<L53_ACTBITS>>, Identity, ap_int<L53_WBITS> >
		(fcin, fcout, weights, PassThroughActivation<ap_int<L53_ACTBITS>>(), 1, ap_resource_dsp());

GenParamStream<L53_MH, 1, L53_PE, L53_ACTBITS>(bias_fc1000, bias, 1);
AddStreams_Batch<L53_PE, ap_int<L53_ACTBITS>, ap_int<L53_ACTBITS>, ap_int<L53_ACTBITS>, L53_MH>(fcout, bias, fcwbias, 1);
LabelSelect_Batch<L53_MH,L53_PE,5,ap_int<L53_ACTBITS>,ap_uint<32>>(fcwbias, output, 1);   
}
