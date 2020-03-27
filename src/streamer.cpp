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
#include "weights.hpp"
#include "activations.hpp"

#include "config.h" // arch parameters
#include "params_weights.h" //net conv weights

using namespace hls;

void streamer(
#ifdef RES2BR
              stream<ap_uint<RES_1_SIMD*RES_1_PE*RES_1_WBITS> > &weights1,
#endif
              stream<ap_uint<RES_2A_SIMD*RES_2A_PE*RES_2A_WBITS> > &weights2a,
              stream<ap_uint<RES_2B_SIMD*RES_2B_PE*RES_2B_WBITS> > &weights2b,
              stream<ap_uint<RES_2C_SIMD*RES_2C_PE*RES_2C_WBITS> > &weights2c
){
#pragma HLS INTERFACE axis register both port=weights2a
#pragma HLS INTERFACE axis register both port=weights2b
#pragma HLS INTERFACE axis register both port=weights2c
#ifdef RES2BR
#pragma HLS INTERFACE axis register both port=weights1
#endif
#pragma HLS INTERFACE ap_ctrl_none port=return
#include "pragma_weights.h"
#pragma HLS DATAFLOW

//instantiate streamers for 2a, 2b, 2c
GenParamStream<(RES_2A_IFMC/RES_2A_SIMD)*(RES_2A_OFMC/RES_2A_PE), RES_2A_SIMD, RES_2A_PE, RES_2A_WBITS>(weights_FPGABipolarConvThresholdLayer_br2a, weights2a, RES_2A_OFMDIM*RES_2A_OFMDIM);
GenParamStream<3*3*(RES_2B_IFMC/RES_2B_SIMD)*(RES_2B_OFMC/RES_2B_PE), RES_2B_SIMD, RES_2B_PE, RES_2B_WBITS>(weights_FPGABipolarConvThresholdLayer_br2b, weights2b, RES_2B_OFMDIM*RES_2B_OFMDIM);
GenParamStream<(RES_2C_IFMC/RES_2C_SIMD)*(RES_2C_OFMC/RES_2C_PE), RES_2C_SIMD, RES_2C_PE, RES_2C_WBITS>(weights_FPGABipolarConvThresholdLayer_br2c, weights2c, RES_2C_OFMDIM*RES_2C_OFMDIM);

#ifdef RES2BR

//instantiate streamer for bypass conv
GenParamStream<(RES_1_IFMC/RES_1_SIMD)*(RES_1_OFMC/RES_1_PE), RES_1_SIMD, RES_1_PE, RES_1_WBITS>(weights_FPGABipolarConvThresholdLayer_br1, weights1, RES_1_OFMDIM*RES_1_OFMDIM);

#endif
}
