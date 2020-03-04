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
#include <hls_stream.h>
#include "bnn-library.h"
#include "ap_int.h"
using namespace hls;
#include "inpipe/config.h"
#include "outpipe/config.h"

#define RSN50_OFFLOAD_INBITS (L0_IFMC*L0_IFMDIM*L0_IFMDIM*L0_INBITS)
#define RSN50_OFFLOAD_INBITSPADDED (512*(RSN50_OFFLOAD_INBITS/512 + ((RSN50_OFFLOAD_INBITS%512 != 0) ? 1 : 0)))//pad to a multiple of 512 bits
#define RSN50_OFFLOAD_INBYTESPADDED (RSN50_OFFLOAD_INBITSPADDED / 8)
#define RSN50_OFFLOAD_INWORDS (RSN50_OFFLOAD_INBITSPADDED/512)

#define RSN50_OFFLOAD_OUTBITS (5*32)
#define RSN50_OFFLOAD_OUTBITSPADDED (32*(RSN50_OFFLOAD_OUTBITS/32 + ((RSN50_OFFLOAD_OUTBITS%32 != 0) ? 1 : 0)))
#define RSN50_OFFLOAD_OUTBYTESPADDED (RSN50_OFFLOAD_OUTBITSPADDED/8)
#define RSN50_OFFLOAD_OUTWORDS (RSN50_OFFLOAD_OUTBITSPADDED/32)

#define RSN50_OFFLOAD_FCBITS (L53_MW*L53_MH*L53_WBITS)
#define RSN50_OFFLOAD_FCBITSPADDED (512*(RSN50_OFFLOAD_FCBITS/512 + ((RSN50_OFFLOAD_FCBITS%512 != 0) ? 1 : 0)))
#define RSN50_OFFLOAD_FCBYTESPADDED (RSN50_OFFLOAD_FCBITSPADDED/8)
#define RSN50_OFFLOAD_FCWORDS (RSN50_OFFLOAD_FCBITSPADDED/512)

void inoutdma(ap_uint<512> * in, ap_uint<32> * out, ap_uint<512> * weightsmem, stream<ap_uint<32> > &input, stream<ap_uint<512> > &output, stream<ap_uint<512> > &weights, unsigned int numReps) {
#pragma HLS INTERFACE axis register both port=input
#pragma HLS INTERFACE axis register both port=output
#pragma HLS INTERFACE axis register both port=weights
#pragma HLS INTERFACE s_axilite port=return bundle=control
#pragma HLS INTERFACE s_axilite port=numReps bundle=control
#pragma HLS INTERFACE m_axi offset=slave port=in bundle=gmem0 depth=2353
#pragma HLS INTERFACE m_axi offset=slave port=out bundle=gmem1 depth=6
#pragma HLS INTERFACE m_axi offset=slave port=weightsmem bundle=gmem2 depth=32010
#pragma HLS INTERFACE s_axilite port=in bundle=control
#pragma HLS INTERFACE s_axilite port=out bundle=control
#pragma HLS INTERFACE s_axilite port=weightsmem bundle=control
#pragma HLS DATAFLOW
    Mem2Stream_Batch<512, RSN50_OFFLOAD_INBYTESPADDED>(in, output, numReps);
    Mem2Stream_Batch_external_wmem<512, RSN50_OFFLOAD_FCBYTESPADDED>(weightsmem, weights, numReps);
    Stream2Mem_Batch<32, RSN50_OFFLOAD_OUTBYTESPADDED>(input, out, numReps);
}
