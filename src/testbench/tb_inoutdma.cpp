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
#include <string>
#include <iostream>
#include "assert.h"
#include <stdio.h>
#include <stdlib.h>
#include "tb_utils.h"
#include "inpipe/config.h"
#include "outpipe/config.h"
using namespace hls;
using namespace std;

FILE * file_input;
FILE * file_output;
FILE * fc_weights_file;

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


void inoutdma(ap_uint<512> * in, ap_uint<32> * out, ap_uint<512> * weightsmem, stream<ap_uint<32> > &input, stream<ap_uint<512> > &output, stream<ap_uint<512> > &weights, unsigned int numReps);

int test_inoutdma(const std::string& infile, const std::string& outfile, const std::string& fcfile){
    file_input = fopen(infile.c_str(), "r");
    file_output = fopen(outfile.c_str(), "r");
    fc_weights_file = fopen(fcfile.c_str(), "r");
    if (file_output==NULL) {cout << "Output file error" << endl; exit (1);}
    if (file_input==NULL) {cout << "Input file error" << endl; exit (1);}
    if (fc_weights_file==NULL) {cout << "Weights file error" << endl; exit (1);}

    ap_uint<512> in[RSN50_OFFLOAD_INWORDS];
    ap_uint<32> out[RSN50_OFFLOAD_OUTWORDS];
    ap_uint<512> * wfc = new ap_uint<512>[RSN50_OFFLOAD_FCWORDS];
    
    fillInputBuffer <512, L0_INBITS> (file_input, L0_IFMC, L0_IFMDIM, L0_IFMDIM, in);
    fseek(file_input,0,SEEK_SET);

    fillInputBuffer <512, L53_WBITS> (fc_weights_file, 1, L53_MH, L53_MW, wfc);
    fseek(fc_weights_file,0,SEEK_SET);
    
    stream<ap_uint<512> > instrm("instrm");
    stream<ap_uint<512*L0_IFMC> > instrm_wide("instrm_wide");
    stream<ap_uint<L0_INBITS*L0_IFMC> > instrm_narrow("instrm_narrow");
    stream<ap_uint<512> > weights("weights");
    stream<ap_uint<1*L53_WBITS> > weights_narrow("weights_narrow");
    stream<ap_uint<32> > results("results");

    loadStreamFromFile <32, 32>(file_output, results, 1, 5, 1);
    fseek(file_output,0,SEEK_SET);

    inoutdma(in, out, wfc, results, instrm, weights, 1);
    StreamingDataWidthConverter_Batch<512, L0_IFMC*512, RSN50_OFFLOAD_INWORDS>(instrm, instrm_wide, 1);
    StreamingDataWidthConverter_Batch<L0_IFMC*512, L0_IFMC*L0_INBITS, RSN50_OFFLOAD_INWORDS/L0_IFMC>(instrm_wide, instrm_narrow, 1);
    StreamingDataWidthConverter_Batch<512, 1*L53_WBITS, RSN50_OFFLOAD_FCWORDS>(weights, weights_narrow, 1);

    int nerrors = checkStreamAgainstFile <L0_IFMC*L0_INBITS, L0_INBITS>(file_input, instrm_narrow, L0_IFMC, L0_IFMDIM, L0_IFMDIM);
    nerrors += checkStreamAgainstFile <1*L53_WBITS, L53_WBITS>(fc_weights_file, weights_narrow, 1, L53_MW, L53_MH);
    nerrors += checkOutputBuffer <32, 32> (file_output, 1, 5, 1, out);
    if(nerrors != 0) { cout << nerrors << endl; return 1; }
    
    fclose(file_input);
    fclose(file_output);
    fclose(fc_weights_file);
    delete[] wfc;

    return 0;
}


int main(int argc, char **argv)
{
    if(argc < 4){
        cout << "Too few arguments, please provide input files for: input output fcweights" << endl;
        return 1;
    }
    return test_inoutdma(argv[1],argv[2],argv[3]);
}
