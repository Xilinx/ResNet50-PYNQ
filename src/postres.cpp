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
#include <iostream>
#include "outpipe/params.h"
using namespace hls;


template<unsigned int SIMDWidth, 		// number of SIMD lanes per PE
		unsigned int PECount,			// number of PEs
		unsigned int WeightsPrecision, 	// Number of bits in thresholds
		unsigned int MatrixW,			// width of matrix, multiple of SIMDWidth
		unsigned int MatrixH,			// height of matrix, multiple of PECount
		unsigned int WMemCount,			// entries in weight memory
		unsigned int Precision,			// Input data bitwidth
		unsigned int ActivationPrecision, // Precisions for the activation (Output precision)
		unsigned int MacPrecision,		// Precision of MAC registers
		template<int> class type_input = ap_int		// For first layer use int value
>
void MatrixVector_Precision_NoActivation_Stream_Batch(stream<ap_uint<SIMDWidth * Precision> > & in,
		stream<ap_uint<PECount * ActivationPrecision> > & out,
		stream<ap_uint<SIMDWidth * PECount * WeightsPrecision> > &weightStream, const ap_int<ActivationPrecision> biasMem[PECount][MatrixH/PECount],
		const unsigned int numReps)
{
	CASSERT_DATAFLOW(MatrixW % SIMDWidth == 0);
	CASSERT_DATAFLOW(MatrixH % PECount == 0);

	// how many different rows each neuron will compute
	// alternatively: number of vertical matrix chunks
	const unsigned int neuronFold = MatrixH / PECount;

	// how many synapse groups each row is split into
	// alternatively: number of horizontal matrix chunks
	const unsigned int synapseFold = MatrixW / SIMDWidth;

	// input vector buffer
	ap_uint<Precision * SIMDWidth> inputBuf[synapseFold];
#pragma HLS RESOURCE variable=inputBuf core=RAM_1P_LUTRAM

	// PE accumulator registers, initialized to zero on first call to function
	// why not defined as static? then different calls to StreamingMatrixVector
	// with the same template parameters would share these accumulator registers
	ap_int<MacPrecision> macRegisters[PECount];
#pragma HLS ARRAY_PARTITION variable=macRegisters complete dim=1


	for(unsigned int i = 0; i < PECount; i++) {
#pragma HLS UNROLL
	  macRegisters[i] = 0;
  	}
	unsigned int nm = 0;
	unsigned int sf = 0;
	const unsigned int totalFold = neuronFold * synapseFold;

	for (unsigned int i = 0; i < totalFold * numReps; i++)
	{
#pragma HLS PIPELINE II=1
		ap_uint<SIMDWidth * Precision> inElem;
		if (nm == 0) {
			// read input from stream
			inElem = in.read();
			// buffer for reuse
			inputBuf[sf] = inElem;
		} else {
			// reuse buffered input
			inElem = inputBuf[sf];
		}
                ap_int<WeightsPrecision * SIMDWidth * PECount> memWeights = weightStream.read();
		// compute matrix-vector product for each processing element
		for (unsigned int pe = 0; pe < PECount; pe++) {
#pragma HLS UNROLL

			ap_int<WeightsPrecision * SIMDWidth> memWeight = memWeights((pe+1)*(WeightsPrecision * SIMDWidth)-1,pe*(WeightsPrecision * SIMDWidth));
			ap_int<MacPrecision> tmpMac = macRegisters[pe];

			for(unsigned int simd = 0; simd < SIMDWidth; simd++){
#pragma HLS UNROLL
				// Fetch weights
				// ap_int<WeightsPrecision * SIMDWidth> weightArray = weightMem[pe][nm * synapseFold + sf];
				ap_int<WeightsPrecision * SIMDWidth> weightArray = memWeight;

				// Low and high bit for each input channel
				unsigned int lowBit = simd * Precision;
				unsigned int highBit = (simd+1) * Precision - 1;

				// Low and high bit for weight channel
				unsigned int lowBitWeight = simd * WeightsPrecision;
				unsigned int highBitWeight = (simd+1) * WeightsPrecision - 1;

				// Get weight for the channel
				type_input<Precision> dataUnsigned = inElem(highBit, lowBit);
				ap_int<WeightsPrecision> weight = weightArray(highBitWeight, lowBitWeight);

				// MAC Operation
				ap_int<MacPrecision> tmpMul = dataUnsigned * weight;
#pragma HLS RESOURCE variable=tmpMul core=DSP48		//Implement in DSPs

				tmpMac += tmpMul;
			}

			macRegisters[pe] = tmpMac;
		}
		sf++;
		if(sf == synapseFold) {

			ap_uint<PECount * ActivationPrecision> outElem = 0;
			for (unsigned int pe = 0; pe < PECount; pe++) {
		#pragma HLS UNROLL
				ap_uint<ActivationPrecision> outputPe;
				// Output MAC register, no threshold
				outputPe(ActivationPrecision-1, 0) = macRegisters[pe](ActivationPrecision-1, 0) + biasMem[pe][nm];
				// Assign to right bits of output buffers
				unsigned int lowBit = pe * ActivationPrecision;
				unsigned int highBit = (pe+1) * ActivationPrecision - 1;
				outElem(highBit, lowBit) = outputPe(ActivationPrecision-1, 0);

				macRegisters[pe] = 0;	// clear the accumulator
			}
			out.write(outElem);
			sf = 0;
			nm++;
		}
		if (nm == neuronFold) {
			// next image
			nm = 0;
		}
	}
}



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
ReLU_Batch<RES_2C_OFMDIM, RES_2C_OFMC, ap_uint<RES_2C_ACTBITS>, RES_2C_PE, 8>(input, reluout, 1);
AccPool_Batch<RES_2C_OFMDIM, RES_2C_OFMC, ap_uint<RES_2C_ACTBITS>, RES_2C_PE, ap_uint<L53_INBITS>>(reluout, poolout, 1);
StreamingDataWidthConverter_Batch<RES_2C_PE*L53_INBITS, L53_SIMD*L53_INBITS, RES_2C_OFMC/RES_2C_PE>(poolout, fcin,1);
MatrixVector_Precision_NoActivation_Stream_Batch<L53_SIMD, L53_PE, L53_WBITS, L53_MW, L53_MH, L53_WMEM, L53_INBITS, L53_ACTBITS, L53_MACBITS>(fcin, fcout, weights, bias_fc1000, 1);
LabelSelect_Batch<L53_MH,L53_PE,5,ap_int<L53_ACTBITS>,ap_uint<32>>(fcout, output, 1);   
}
