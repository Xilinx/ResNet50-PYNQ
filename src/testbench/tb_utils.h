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

#pragma once

#include "ap_int.h"
#include "hls_stream.h"
#include <iostream>
using namespace hls;
using namespace std;

template <unsigned int LINE_SIZE, unsigned int ELEM_SIZE> void fillInputBuffer(unsigned int *data, unsigned int nchannels, unsigned int nrows, unsigned int ncols, ap_uint<LINE_SIZE> *buf){
  
    ap_uint<LINE_SIZE> word;
    unsigned int address = 0;
    unsigned int elem_index = 0;
    for(unsigned int i=0; i < nrows; i++)
    {
        for(unsigned int j=0; j < ncols; j++)
        {
            for(unsigned int k=0; k < nchannels; k++)
            {
                word(ELEM_SIZE*(elem_index+1)-1,ELEM_SIZE*elem_index) = data[k*nrows*ncols+i*ncols+j];
                elem_index++;
                if(elem_index == LINE_SIZE/ELEM_SIZE){
                    elem_index = 0;
                    buf[address] = word;
                    address++;
                }
            }
        }
    }
    if(elem_index != 0){
        buf[address] = word;
    }
}

template <unsigned int LINE_SIZE, unsigned int ELEM_SIZE> unsigned int spillOutputBuffer(unsigned int *data, unsigned int nchannels, unsigned int nrows, unsigned int ncols, ap_uint<LINE_SIZE> *buf){

    ap_uint<LINE_SIZE> word;
    unsigned int address = 0;
    unsigned int elem_index = 0;
    unsigned int ndiff = 0;
    for(unsigned int i=0; i < nrows; i++)
    {
        for(unsigned int j=0; j < ncols; j++)
        {
            for(unsigned int k=0; k < nchannels; k++)
            {
                if(elem_index == 0){
                    word = buf[address];
                    address++;
                }
                data[k*nrows*ncols+i*ncols+j] = word(ELEM_SIZE*(elem_index+1)-1,ELEM_SIZE*elem_index);
                elem_index++;
                if(elem_index == LINE_SIZE/ELEM_SIZE){
                    elem_index = 0;
                }
            }
        }
    }
    return ndiff;
}

template <int StreamSize, int ElementSize> void loadStreamFromFile(FILE * file, stream< ap_uint<StreamSize> > & strm, int nchannels, int nrows, int ncols){

    int data[nchannels][nrows][ncols];
    cout << "Input parameters: " << nchannels << "," << nrows << "," << ncols << endl;
    
    for(unsigned int i=0; i < nchannels; i++)
    {
        for(unsigned int j=0; j < nrows; j++)
        {
            for(unsigned int k=0; k < ncols; k++)
            {
                int d;//largest data type is 8 bits in the whole resnet
                fscanf(file, "%d,", &d);
                data[i][j][k] = d;
            }
        }
    }

    //assemble the resblock input by concatenating channel data for each pixel
    //pixels themselves are traversed in raster order
    for(unsigned int i=0; i < nrows; i++)
    {
        for(unsigned int j=0; j < ncols; j++)
        {
            ap_uint<StreamSize> in_data = 0;
            for(unsigned int k=0; k < nchannels; k++)
            {
                in_data((k+1)*ElementSize-1,k*ElementSize) = data[k][i][j];
            }
            strm.write(in_data);
        }
    }
}

template <int StreamSize, int ElementSize> int checkStreamAgainstFile(FILE * file, stream< ap_uint<StreamSize> > & strm, int nchannels, int nrows, int ncols){

    ap_uint<ElementSize> data[nchannels][nrows][ncols];
    cout << "Output parameters: " << nchannels << "," << nrows << "," << ncols << endl;
    int ndiff = 0;
    
    //read in expected results
    for(unsigned int i=0; i < nchannels; i++)
    {
        for(unsigned int j=0; j < nrows; j++)
        {
            for(unsigned int k=0; k < ncols; k++)
            {
                int d;//largest data type is 8 bits in the whole resnet
                fscanf(file, "%d,", &d);
                data[i][j][k] = d;
            }
        }
    }
    //get results and compare
    for(unsigned int i=0; i < nrows; i++)
    {
        for(unsigned int j=0; j < ncols; j++)
        {
            ap_uint<StreamSize> out_data = strm.read();
            for(unsigned int k=0; k < nchannels; k++)
            {
                if(out_data((k+1)*ElementSize-1,k*ElementSize) != data[k][i][j]){
                    cout << "Data mismatch at "<< k << " "<< i << " "<< j << endl ;
                    cout << "Expected " << hex << data[k][i][j] << " got " << out_data((k+1)*ElementSize-1,k*ElementSize) << dec << endl;
                    ndiff++;    
                }
            }
        }
    }
    
    return ndiff;
}

template <int StreamSize, int ElementSize> int checkStreamAgainstFileBypass(FILE * file, stream< ap_uint<StreamSize> > & istrm, stream< ap_uint<StreamSize> > & ostrm, int nchannels, int nrows, int ncols){

    int data[nchannels][nrows][ncols];
    cout << "Monitor parameters: " << nchannels << "," << nrows << "," << ncols << endl;
    int ndiff = 0;
    
    //read in expected results
    for(unsigned int i=0; i < nchannels; i++)
    {
        for(unsigned int j=0; j < nrows; j++)
        {
            for(unsigned int k=0; k < ncols; k++)
            {
                int d;//largest data type is 8 bits in the whole resnet
                fscanf(file, "%d,", &d);
                data[i][j][k] = d;
            }
        }
    }
    //get results and compare
    for(unsigned int i=0; i < nrows; i++)
    {
        for(unsigned int j=0; j < ncols; j++)
        {
            ap_uint<StreamSize> out_data = istrm.read();
            ostrm.write(out_data);
            for(unsigned int k=0; k < nchannels; k++)
            {
                if(out_data((k+1)*ElementSize-1,k*ElementSize) != data[k][i][j]){
                    cout << "Data mismatch at "<< k << " "<< i << " "<< j << endl ;
                    cout << "Expected " << data[k][i][j] << " got " << hex << out_data((k+1)*ElementSize-1,k*ElementSize) << endl;
                    ndiff++;
                }
            }
        }
    }
    
    return ndiff;
}

template <unsigned int LINE_SIZE, unsigned int ELEM_SIZE> void fillInputBuffer(FILE *file, unsigned int nchannels, unsigned int nrows, unsigned int ncols, ap_uint<LINE_SIZE> *buf){

    unsigned int data[nchannels][nrows][ncols];
    for(unsigned int i=0; i < nchannels; i++)
    {
        for(unsigned int j=0; j < nrows; j++)
        {
            for(unsigned int k=0; k < ncols; k++)
            {
                unsigned int d;
                fscanf(file, "%d,", &d);
                data[i][j][k] = d;
            }
        }
    }
  
    fillInputBuffer <LINE_SIZE,ELEM_SIZE>((unsigned int *)data,nchannels,nrows,ncols,buf);

}

template <unsigned int LINE_SIZE, unsigned int ELEM_SIZE> unsigned int checkOutputBuffer(unsigned int *expected_data, unsigned int nchannels, unsigned int nrows, unsigned int ncols, ap_uint<LINE_SIZE> *buf){

    ap_uint<LINE_SIZE> word;
    unsigned int address = 0;
    unsigned int elem_index = 0;
    unsigned int ndiff = 0;
    for(unsigned int i=0; i < nrows; i++)
    {
        for(unsigned int j=0; j < ncols; j++)
        {
            for(unsigned int k=0; k < nchannels; k++)
            {
                if(elem_index == 0){
                    word = buf[address];
                    address++;
                }
                if(word(ELEM_SIZE*(elem_index+1)-1,ELEM_SIZE*elem_index) != expected_data[k*nrows*ncols+i*ncols+j])
                {
                    cout << "Data mismatch at "<< k << " "<< i << " "<< j << endl ;
                    cout << "Expected " << expected_data[k*nrows*ncols+i*ncols+j] << " got " << hex << word(ELEM_SIZE*(elem_index+1)-1,ELEM_SIZE*elem_index) << dec << endl;
                    ndiff++;    
                }
                elem_index++;
                if(elem_index == LINE_SIZE/ELEM_SIZE){
                    elem_index = 0;
                }
            }
        }
    }
    return ndiff;
}

template <unsigned int LINE_SIZE, unsigned int ELEM_SIZE> unsigned int checkOutputBuffer(FILE *file, unsigned int nchannels, unsigned int nrows, unsigned int ncols, ap_uint<LINE_SIZE> *buf){

    unsigned int expected_data[nchannels][nrows][ncols];
    for(unsigned int i=0; i < nchannels; i++)
    {
        for(unsigned int j=0; j < nrows; j++)
        {
            for(unsigned int k=0; k < ncols; k++)
            {
                unsigned int d;
                fscanf(file, "%d,", &d);
                expected_data[i][j][k] = d;
            }
        }
    }

    return checkOutputBuffer <LINE_SIZE,ELEM_SIZE>((unsigned int *)expected_data,nchannels,nrows,ncols,buf);
}

