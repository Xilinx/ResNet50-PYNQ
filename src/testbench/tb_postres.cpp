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
#include "outpipe/config.h"
#include "res5c/config.h"
#include <string>
#include <iostream>
#include "assert.h"
#include <stdio.h>
#include <stdlib.h>
#include "tb_utils.h"
using namespace hls;
using namespace std;

FILE * file_input;
FILE * file_output;
FILE * fc_weights_file;

void postres(stream<ap_uint<RES_2C_PE*RES_2C_ACTBITS> > &input, stream<ap_uint<32> > &output, stream<ap_uint<L53_SIMD*L53_PE*L53_WBITS> > &weights);
    
int test_postres(const std::string& infile, const std::string& outfile, const std::string& fcfile){
    file_input = fopen(infile.c_str(), "r");
    file_output = fopen(outfile.c_str(), "r");
    fc_weights_file = fopen(fcfile.c_str(), "r");
    if (file_output==NULL) {cout << "Output file error" << endl; exit (1);}
    if (file_input==NULL) {cout << "Input file error" << endl; exit (1);}
    if (fc_weights_file==NULL) {cout << "Weights file error" << endl; exit (1);}

    stream<ap_uint<RES_2C_OFMC*RES_2C_ACTBITS> > input("input");
    stream<ap_uint<1*L53_WBITS> > weights("weights");
    stream<ap_uint<RES_2C_PE*RES_2C_ACTBITS> > in_postres("in_postres");
    stream<ap_uint<L53_SIMD*L53_PE*L53_WBITS> > wfc_postres("wfc_postres");
    
    stream<ap_uint<32> > output("output");

    loadStreamFromFile <RES_2C_OFMC*RES_2C_ACTBITS, RES_2C_ACTBITS>(file_input, input, RES_2C_OFMC, RES_2C_OFMDIM, RES_2C_OFMDIM);
    StreamingDataWidthConverter_Batch<RES_2C_OFMC * RES_2C_ACTBITS, RES_2C_PE * RES_2C_ACTBITS , RES_2C_OFMDIM*RES_2C_OFMDIM>(input, in_postres, 1);
    
    loadStreamFromFile <1*L53_WBITS, L53_WBITS>(fc_weights_file, weights, 1, L53_MW, L53_MH);
    StreamingDataWidthConverter_Batch<1*L53_WBITS, L53_SIMD*L53_PE*L53_WBITS, L53_MW*L53_MH>(weights, wfc_postres, 1);
    
    postres(in_postres, output, wfc_postres);

    int nerrors = checkStreamAgainstFile <1*32, 32>(file_output, output, 1, 5, 1);
    if(nerrors != 0) { cout << nerrors << endl; return 1; }
    
    fclose(file_input);
    fclose(file_output);
    fclose(fc_weights_file);

    return 0;
}


int main(int argc, char **argv)
{
    if(argc < 4){
        cout << "Too few arguments, please provide input files for: input output fcweights" << endl;
        return 1;
    }
    return test_postres(argv[1],argv[2],argv[3]);
}
