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
#include "config.h"
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

void preres(stream<ap_uint<L0_IFMC*L0_INBITS> > &input, stream<ap_uint<pool0_CHANNELS*pool0_PRECISION> > &output);
    
int test_preres(const std::string& infile, const std::string& outfile){
    file_input = fopen(infile.c_str(), "r");
    file_output = fopen(outfile.c_str(), "r");
    if (file_output==NULL) {cout << "Output file error" << endl; exit (1);}
    if (file_input==NULL) {cout << "Input file error" << endl; exit (1);}
    
    stream<ap_uint<L0_IFMC*L0_INBITS> > input("input");
    stream<ap_uint<pool0_CHANNELS*pool0_PRECISION> > output("output");

    loadStreamFromFile <L0_IFMC*L0_INBITS, L0_INBITS>(file_input, input, L0_IFMC, L0_IFMDIM, L0_IFMDIM);

    preres(input, output);

    int nerrors = checkStreamAgainstFile <pool0_CHANNELS*pool0_PRECISION, pool0_PRECISION>(file_output, output, pool0_CHANNELS, pool0_IMDIM/pool0_STRIDE, pool0_IMDIM/pool0_STRIDE);
    if(nerrors != 0) { cout << nerrors << endl; return 1; }
    
    fclose(file_input);
    fclose(file_output);

    return 0;
}


int main(int argc, char **argv)
{
    if(argc < 3){
        cout << "Too few arguments, please provide: input output" << endl;
        return 1;
    }
    return test_preres(argv[1],argv[2]);
}
