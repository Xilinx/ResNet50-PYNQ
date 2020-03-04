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
#include "config.h"
#include <string>
#include <iostream>
#include "assert.h"
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <string>
#include <vector>
#include "tb_utils.h"
using namespace hls;
using namespace std;

FILE * file_input;
FILE * file_output;

void resblock(stream<ap_uint<RES_2A_SIMD*RES_BYPINBITS> > &input, stream<ap_uint<RES_2C_PE*RES_2C_ACTBITS> > &output);

int test_resblock(const std::string& infile, const std::string& outfile){
    file_input = fopen(infile.c_str(), "r");
    file_output = fopen(outfile.c_str(), "r");
    if (file_output==NULL) {cout << "Output file error" << endl; exit (1);}
    if (file_input==NULL) {cout << "Input file error" << endl; exit (1);}
    
    stream<ap_uint<RES_2A_IFMC*RES_BYPINBITS> > input("input");
    stream<ap_uint<RES_2C_OFMC*RES_2C_ACTBITS> > output("output");
    
    stream<ap_uint<RES_2A_SIMD*RES_BYPINBITS> > res_in("res_in");
    stream<ap_uint<RES_2C_PE*RES_2C_ACTBITS> > res_out("res_out");

    loadStreamFromFile <RES_2A_IFMC*RES_BYPINBITS, RES_BYPINBITS>(file_input, input, RES_2A_IFMC, RES_2A_IFMDIM, RES_2A_IFMDIM);

    StreamingDataWidthConverter_Batch<RES_2A_IFMC*RES_BYPINBITS, RES_2A_SIMD*RES_BYPINBITS , RES_2A_IFMDIM*RES_2A_IFMDIM>(input, res_in, 1);
    resblock(res_in, res_out);
    StreamingDataWidthConverter_Batch<RES_2C_PE*RES_2C_ACTBITS, RES_2C_OFMC*RES_2C_ACTBITS, RES_2C_OFMDIM*RES_2C_OFMDIM*(RES_2C_OFMC/RES_2C_PE)>(res_out, output, 1);

    int nerrors = checkStreamAgainstFile <RES_2C_OFMC*RES_2C_ACTBITS, RES_2C_ACTBITS>(file_output, output, RES_2C_OFMC, RES_2C_OFMDIM, RES_2C_OFMDIM);
    if(nerrors != 0) { cout << nerrors << endl; return 1; }
    
    fclose(file_input);
    fclose(file_output);

    return 0;
}


int main(int argc, char **argv)
{
    if(argc < 3){
        cout << "Too few arguments, please provide: type input output" << endl;
        return 1;
    }
    return test_resblock(argv[1],argv[2]);
}
