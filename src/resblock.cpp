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
#include "reslayer.h"
#include "config.h"
#include "params_thresholds.h"
using namespace hls;

#ifdef RES_WSTREAM // TODO cleaner merge?
  void resblock(stream<ap_uint<RES_2A_SIMD*RES_BYPINBITS> > &input, stream<ap_uint<RES_2C_PE*RES_2C_ACTBITS> > &output,
  #ifdef RES2BR
                stream<ap_uint<RES_1_SIMD*RES_1_PE*RES_1_WBITS> > &weights1,
  #endif
                stream<ap_uint<RES_2A_SIMD*RES_2A_PE*RES_2A_WBITS> > &weights2a,
                stream<ap_uint<RES_2B_SIMD*RES_2B_PE*RES_2B_WBITS> > &weights2b,
                stream<ap_uint<RES_2C_SIMD*RES_2C_PE*RES_2C_WBITS> > &weights2c
  ){
  #pragma HLS INTERFACE axis register both port=output
  #pragma HLS INTERFACE axis register both port=input
  #pragma HLS INTERFACE axis register both port=weights2a
  #pragma HLS INTERFACE axis register both port=weights2b
  #pragma HLS INTERFACE axis register both port=weights2c
  #ifdef RES2BR
  #pragma HLS INTERFACE axis register both port=weights1
  #endif
  #pragma HLS INTERFACE ap_ctrl_chain port=return
  #include "pragma_thresholds.h"
  #pragma HLS DATAFLOW

  #ifdef RES1BR
    Residual_1branch_WStream< RES_BYPINBITS, RES_BYPTHBITS, RES_BYPTHPE, RES_BYPTHTMEM, 
                    RES_2A_IFMC, RES_2A_OFMC, RES_2A_IFMDIM, RES_2A_OFMDIM, RES_2A_STRIDE, RES_2A_SIMD, RES_2A_PE, RES_2A_WMEM, RES_2A_TMEM, RES_2A_WBITS, RES_2A_WINTERPRET, RES_2A_THBITS, RES_2A_MACBITS, RES_2A_INBITS, RES_2A_ACTBITS, 
                    RES_2B_IFMC, RES_2B_OFMC, RES_2B_IFMDIM, RES_2B_OFMDIM, RES_2B_STRIDE, RES_2B_SIMD, RES_2B_PE, RES_2B_WMEM, RES_2B_TMEM, RES_2B_WBITS, RES_2B_WINTERPRET, RES_2B_THBITS, RES_2B_MACBITS, RES_2B_INBITS, RES_2B_ACTBITS, 
                    RES_2C_IFMC, RES_2C_OFMC, RES_2C_IFMDIM, RES_2C_OFMDIM, RES_2C_STRIDE, RES_2C_SIMD, RES_2C_PE, RES_2C_WMEM, RES_2C_TMEM, RES_2C_WBITS, RES_2C_WINTERPRET, RES_2C_THBITS, RES_2C_MACBITS, RES_2C_INBITS, RES_2C_ACTBITS>
            (input, output, thres_FPGAThresholdLayer_top, 
              weights2a, thres_FPGABipolarConvThresholdLayer_br2a, 
              weights2b, thres_FPGABipolarConvThresholdLayer_br2b, 
              weights2c, thres_FPGABipolarConvThresholdLayer_br2c, 1);
  #elif RES2BR
    Residual_2branches_WStream< RES_BYPINBITS, RES_BYPTHBITS, RES_BYPTHPE, RES_BYPTHTMEM, 
                            RES_1_IFMC, RES_1_OFMC, RES_1_IFMDIM, RES_1_OFMDIM, RES_1_STRIDE, RES_1_SIMD, RES_1_PE, RES_1_WMEM, RES_1_TMEM, RES_1_WBITS, RES_1_WINTERPRET, RES_1_THBITS, RES_1_MACBITS, RES_1_INBITS, RES_1_ACTBITS,
                            RES_2A_IFMC, RES_2A_OFMC, RES_2A_IFMDIM, RES_2A_OFMDIM, RES_2A_STRIDE, RES_2A_SIMD, RES_2A_PE, RES_2A_WMEM, RES_2A_TMEM, RES_2A_WBITS, RES_2A_WINTERPRET, RES_2A_THBITS, RES_2A_MACBITS, RES_2A_INBITS, RES_2A_ACTBITS, 
                            RES_2B_IFMC, RES_2B_OFMC, RES_2B_IFMDIM, RES_2B_OFMDIM, RES_2B_STRIDE, RES_2B_SIMD, RES_2B_PE, RES_2B_WMEM, RES_2B_TMEM, RES_2B_WBITS, RES_2B_WINTERPRET, RES_2B_THBITS, RES_2B_MACBITS, RES_2B_INBITS, RES_2B_ACTBITS, 
                            RES_2C_IFMC, RES_2C_OFMC, RES_2C_IFMDIM, RES_2C_OFMDIM, RES_2C_STRIDE, RES_2C_SIMD, RES_2C_PE, RES_2C_WMEM, RES_2C_TMEM, RES_2C_WBITS, RES_2C_WINTERPRET, RES_2C_THBITS, RES_2C_MACBITS, RES_2C_INBITS, RES_2C_ACTBITS>
            (input, output, thres_FPGAThresholdLayer_top, 
                            weights1, thres_FPGABipolarConvThresholdLayer_br1,
                            weights2a, thres_FPGABipolarConvThresholdLayer_br2a, 
                            weights2b, thres_FPGABipolarConvThresholdLayer_br2b, 
                            weights2c, thres_FPGABipolarConvThresholdLayer_br2c, 1);
  #endif

  }

#else

  #include "params_weights.h"
  void resblock(stream<ap_uint<RES_2A_SIMD*RES_BYPINBITS> > &input, stream<ap_uint<RES_2C_PE*RES_2C_ACTBITS> > &output){
  #pragma HLS INTERFACE axis register both port=output
  #pragma HLS INTERFACE axis register both port=input
  #pragma HLS INTERFACE ap_ctrl_chain port=return
  #include "pragma_thresholds.h"
  #include "pragma_weights.h"
  #pragma HLS DATAFLOW
  #ifdef RES1BR
      Residual_1branch<   RES_BYPINBITS, RES_BYPTHBITS, RES_BYPTHPE, RES_BYPTHTMEM,   
                          RES_2A_IFMC, RES_2A_OFMC, RES_2A_IFMDIM, RES_2A_OFMDIM, RES_2A_STRIDE, RES_2A_SIMD, RES_2A_PE, RES_2A_WMEM, RES_2A_TMEM, RES_2A_WINTERPRET, RES_2A_THBITS, RES_2A_MACBITS, RES_2A_INBITS, RES_2A_ACTBITS, 
                          RES_2B_IFMC, RES_2B_OFMC, RES_2B_IFMDIM, RES_2B_OFMDIM, RES_2B_STRIDE, RES_2B_SIMD, RES_2B_PE, RES_2B_WMEM, RES_2B_TMEM, RES_2B_WINTERPRET, RES_2B_THBITS, RES_2B_MACBITS, RES_2B_INBITS, RES_2B_ACTBITS, 
                          RES_2C_IFMC, RES_2C_OFMC, RES_2C_IFMDIM, RES_2C_OFMDIM, RES_2C_STRIDE, RES_2C_SIMD, RES_2C_PE, RES_2C_WMEM, RES_2C_TMEM, RES_2C_WINTERPRET, RES_2C_THBITS, RES_2C_MACBITS, RES_2C_INBITS, RES_2C_ACTBITS>
          (input, output, thres_FPGAThresholdLayer_top, 
                          weights_FPGABipolarConvThresholdLayer_br2a, thres_FPGABipolarConvThresholdLayer_br2a, 
                          weights_FPGABipolarConvThresholdLayer_br2b, thres_FPGABipolarConvThresholdLayer_br2b, 
                          weights_FPGABipolarConvThresholdLayer_br2c, thres_FPGABipolarConvThresholdLayer_br2c, 1);
  #elif RES2BR
      Residual_2branches< RES_BYPINBITS, RES_BYPTHBITS, RES_BYPTHPE, RES_BYPTHTMEM, 
                          RES_2A_IFMC, RES_2A_OFMC, RES_2A_IFMDIM, RES_2A_OFMDIM, RES_2A_STRIDE, RES_2A_SIMD, RES_2A_PE, RES_2A_WMEM, RES_2A_TMEM, RES_2A_WINTERPRET, RES_2A_THBITS, RES_2A_MACBITS, RES_2A_INBITS, RES_2A_ACTBITS, 
                          RES_2B_IFMC, RES_2B_OFMC, RES_2B_IFMDIM, RES_2B_OFMDIM, RES_2B_STRIDE, RES_2B_SIMD, RES_2B_PE, RES_2B_WMEM, RES_2B_TMEM, RES_2B_WINTERPRET, RES_2B_THBITS, RES_2B_MACBITS, RES_2B_INBITS, RES_2B_ACTBITS, 
                          RES_2C_IFMC, RES_2C_OFMC, RES_2C_IFMDIM, RES_2C_OFMDIM, RES_2C_STRIDE, RES_2C_SIMD, RES_2C_PE, RES_2C_WMEM, RES_2C_TMEM, RES_2C_WINTERPRET, RES_2C_THBITS, RES_2C_MACBITS, RES_2C_INBITS, RES_2C_ACTBITS,
                          RES_1_IFMC, RES_1_OFMC, RES_1_IFMDIM, RES_1_OFMDIM, RES_1_STRIDE, RES_1_SIMD, RES_1_PE, RES_1_WMEM, RES_1_TMEM, RES_1_WINTERPRET, RES_1_THBITS, RES_1_MACBITS, RES_1_INBITS, RES_1_ACTBITS>
          (input, output, thres_FPGAThresholdLayer_top, 
                          weights_FPGABipolarConvThresholdLayer_br2a, thres_FPGABipolarConvThresholdLayer_br2a, 
                          weights_FPGABipolarConvThresholdLayer_br2b, thres_FPGABipolarConvThresholdLayer_br2b, 
                          weights_FPGABipolarConvThresholdLayer_br2c, thres_FPGABipolarConvThresholdLayer_br2c, 
                          weights_FPGABipolarConvThresholdLayer_br1, thres_FPGABipolarConvThresholdLayer_br1, 1);
  #endif
  }
#endif
