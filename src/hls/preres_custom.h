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

//depends on original FINN hls library
#include "rpnn-library.h"

// Reshape input stream to output only useful data when padding is same:
// Might add 0s at left, right, upper, lower side of the input
// Pad with 0
template<       unsigned int ImgDim,
                        unsigned int KernelDim,
                        unsigned int Stride,
                        unsigned int NumChannels,
                        unsigned int Precision,
                        unsigned int PaddingStyle>
void SameResize(stream<ap_uint<NumChannels* Precision> > &in,
                stream<ap_uint<NumChannels* Precision> > &out){

        // Number of "same" windows over the input data
        constexpr unsigned int SameWindows = (ImgDim) / Stride + ((ImgDim % Stride) > 0);

        // Number of elements to generate as output per dimension
        constexpr unsigned int OutputDim = KernelDim + Stride * (SameWindows - 1);

        // Padding
        constexpr unsigned int Padding = OutputDim - ImgDim;

        // Padding Up and Left
        constexpr unsigned int PaddingUp = Padding/2 + ((PaddingStyle == 2) ? ((Padding % 2) > 0) : 0);
        constexpr unsigned int PaddingLeft = Padding/2 + ((PaddingStyle == 2) ? ((Padding % 2) > 0) : 0);

        // Padding Down and Right (might be 1 element more than up and left in case of odd padding)
        constexpr unsigned int PaddingDown = Padding - PaddingUp;
        constexpr unsigned int PaddingRight = Padding - PaddingLeft;

        ap_uint<NumChannels* Precision> outData, inData;

        for(unsigned int y = 0; y<OutputDim; y++){
                for(unsigned int x=0; x < OutputDim; x++){
#pragma HLS PIPELINE II=1

                        // Padding Rows
                        if(y < PaddingUp || y >= (OutputDim - PaddingDown)){
                                outData = 0;
                        }
                        // Padding Cols
                        else if(x < PaddingLeft || x >= (OutputDim - PaddingRight)){
                                outData = 0;
                        }
                        // No Padding
                        else{
                                inData = in.read();
                                outData = inData;
                        }

                        out.write(outData);
                }
        }
}

template<       unsigned int ImgDim,
                        unsigned int KernelDim,
                        unsigned int Stride,
                        unsigned int NumChannels,
                        unsigned int Precision,
                        unsigned int PaddingStyle>
void SameResize_Batch(stream<ap_uint<NumChannels* Precision> > &in,
                stream<ap_uint<NumChannels* Precision> > &out,
                const unsigned int numReps) {
        for (unsigned int rep = 0; rep < numReps; rep++) {
                SameResize<ImgDim, KernelDim, Stride, NumChannels, Precision, PaddingStyle>(in, out);
        }

}

template<       unsigned int ImgDim,
                        unsigned int KernelDim,
                        unsigned int Stride,
                        unsigned int NumChannels,
                        unsigned int Precision,
                        unsigned int PaddingStyle>
void MaxPoolStride_Same_Batch(stream<ap_uint<NumChannels * Precision> > & in,
                stream<ap_uint<NumChannels * Precision> > & out, const unsigned int numReps){
#pragma HLS INLINE

        stream<ap_uint<NumChannels * Precision> > paddingOut, resizeOut;
#pragma HLS RESOURCE variable=paddingOut core=FIFO_LUTRAM
#pragma HLS STREAM variable=paddingOut depth=32
#pragma HLS RESOURCE variable=resizeOut core=FIFO_LUTRAM
#pragma HLS STREAM variable=resizeOut depth=32

        // Number of output windows
        constexpr unsigned int outputWindows = (ImgDim) / Stride + ((ImgDim % Stride) > 0);

        // Output dimensions of the resize stage
        constexpr unsigned int resizeOutputDim = KernelDim + Stride * (outputWindows - 1);

        // Number of output elements per dimension (of padder + resize components)
        constexpr unsigned int ImgDimOut = outputWindows * KernelDim;

        SameResize_Batch<ImgDim, KernelDim, Stride, NumChannels, Precision, PaddingStyle>(in, paddingOut, numReps);
        MaxPool_InputGenerator_Batch<resizeOutputDim, KernelDim, Stride, NumChannels, Precision, 1>(paddingOut, resizeOut, numReps);
        MaxPool_ReducedPrecision_Batch<ImgDimOut, KernelDim, NumChannels, Precision>(resizeOut, out, numReps);

}


//specialized double-packed 8x8 multiplier
template<int MacPrecision, int SIMDWidth, int NumVecs>
void PE_dsp_packed(ap_uint<8> dataUnsigned[NumVecs][SIMDWidth],
        ap_int<MacPrecision> tmpMac[NumVecs],
        ap_int<8*SIMDWidth> memWeight) {

    CASSERT_DATAFLOW(NumVecs % 2 == 0);//ensure even vector size

    for(unsigned int v = 0; v < NumVecs; v++) {
#pragma HLS UNROLL
        tmpMac[v] = 0;
    }
    for(unsigned int simd = 0; simd < SIMDWidth; simd++){
#pragma HLS UNROLL
        // Low and high bit for weight channel
        unsigned int lowBitWeight = simd * 8;
        unsigned int highBitWeight = (simd+1) * 8 - 1;

        ap_int<8> weight = memWeight(highBitWeight, lowBitWeight);

        // MAC Operation
        for(unsigned int v = 0; v < NumVecs; v+=2) {
#pragma HLS UNROLL
            ap_int<27> data = 0;
            ap_int<35> adjust = 0;
            data(7,0) = dataUnsigned[v][simd];
            data(23,16) = dataUnsigned[v+1][simd];
            adjust(16,16) = weight(7,7) & ~(dataUnsigned[v][simd] == 0);
            ap_int<35> tmpMul;
#pragma HLS RESOURCE variable=tmpMul core=DSP48		//Implement in DSPs
            tmpMul = data * weight + adjust;
            ap_int<16> tmpMulA = tmpMul(15,0);
            ap_int<16> tmpMulB = tmpMul(31,16);
            tmpMac[v] += tmpMulA;
            tmpMac[v+1] += tmpMulB;
        }
    }
}

template<unsigned int SIMDWidth, 		// number of SIMD lanes per PE
		unsigned int PECount,			// number of PEs
		unsigned int WeightsPrecision, 	// Number of bits in thresholds
		unsigned int ThresholdPrecision, // Number of bits in thresholds
		unsigned int MatrixW,			// width of matrix, multiple of SIMDWidth
		unsigned int MatrixH,			// height of matrix, multiple of PECount
		unsigned int WMemCount,			// entries in weight memory
		unsigned int TMemCount,			// entries in threshold memory
		unsigned int Precision,			// Input data bitwidth
		unsigned int ActivationPrecision, // Precisions for the activation (Output precision)
		unsigned int MacPrecision,		// Precision of MAC registers
		unsigned int NumVecs,			// Number of vectors in multi-vector
		unsigned int ActivationType = 0,	// Don't use activation
		template<int> class type_input = ap_uint		// For first layer use int value
>
void MatrixMultiVector_Precision_Batch_dsp_packed(stream<MultiChanData<NumVecs, SIMDWidth * Precision> > & in,
		stream<MultiChanData<NumVecs, PECount * ActivationPrecision> > & out,
		const ap_uint<SIMDWidth * WeightsPrecision> weightMem[PECount][WMemCount],
		const ap_uint<ThresholdPrecision> thresMem[PECount][TMemCount],
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
	ap_uint<Precision * SIMDWidth> inputBuf[NumVecs][synapseFold];
#pragma HLS ARRAY_PARTITION variable=inputBuf complete dim=1
	
	// PE accumulator registers, initialized to zero on first call to function
	// why not defined as static? then different calls to StreamingMatrixVector
	// with the same template parameters would share these accumulator registers
	ap_int<MacPrecision> macRegisters[NumVecs][PECount];
#pragma HLS ARRAY_PARTITION variable=macRegisters dim=0 complete 
	for (unsigned int v = 0; v < NumVecs; v++) {
		#pragma HLS UNROLL	
		for(unsigned int i = 0; i < PECount; i++) {
		  macRegisters[v][i] = 0;
	  	}
	}
	unsigned int nm = 0;
	unsigned int sf = 0;
	const unsigned int totalFold = neuronFold * synapseFold;

	for (unsigned int i = 0; i < totalFold * numReps; i++) 
	{
#pragma HLS PIPELINE II=1
		MultiChanData<NumVecs, SIMDWidth * Precision> inElem;
		if (nm == 0) {
			// read input from stream
			inElem = in.read();
			// buffer for reuse
			for(unsigned int v = 0; v < NumVecs; v++) {
#pragma HLS UNROLL
				inputBuf[v][sf] = inElem.data[v];
			}
		} else {
			// reuse buffered input
			for(unsigned int v = 0; v < NumVecs; v++) {
#pragma HLS UNROLL
				inElem.data[v] = inputBuf[v][sf];
			}
		}

        // Get weight for the channel
        type_input<Precision> dataUnsigned[NumVecs][SIMDWidth];
#pragma HLS ARRAY_RESHAPE variable=dataUnsigned complete dim=0
        for(unsigned int simd = 0; simd < SIMDWidth; simd++){
            // Low and high bit for each input channel
            unsigned int lowBit = simd * Precision;
            unsigned int highBit = (simd+1) * Precision - 1;
            for(unsigned int v = 0; v < NumVecs; v++) {
                dataUnsigned[v][simd] = inElem.data[v](highBit, lowBit);
            }
        }

		// compute matrix-vector product for each processing element
		for (unsigned int pe = 0; pe < PECount; pe++) {
#pragma HLS UNROLL
			ap_int<WeightsPrecision * SIMDWidth> memWeight =  weightMem[pe][nm * synapseFold + sf];
			ap_int<MacPrecision> tmpMac[NumVecs];
#pragma HLS ARRAY_RESHAPE variable=tmpMac complete dim=1
            PE_dsp_packed<MacPrecision, SIMDWidth, NumVecs>(dataUnsigned, tmpMac, memWeight);
			for(unsigned int v = 0; v < NumVecs; v++) {
#pragma HLS UNROLL			
                macRegisters[v][pe] += tmpMac[v];
			}
		}
		sf++;
		if(sf == synapseFold) {
			MultiChanData<NumVecs, PECount * ActivationPrecision> outElem;
			for (unsigned int pe = 0; pe < PECount; pe++) {
		#pragma HLS UNROLL
				ap_uint<ActivationPrecision> outputPe[NumVecs];
#pragma HLS ARRAY_PARTITION variable=outputPe complete dim=1
				if(ActivationType == FULL_THRESHOLDS){

					// TODO: Reducing precision check is used onl because the compiler tries to compile
					// this code even when ActivationType!=FULL_THRESHOLDS.
					// Need to find a way to remove this and set NumberOfThresholds = 1 << ActivationPrecision
					constexpr unsigned int reducingPrecision = Precision >= ActivationPrecision;
					constexpr unsigned int NumberOfThresholds = reducingPrecision ? (1 << ActivationPrecision) : 2;
					ap_int<ThresholdPrecision> thresholdPe;
					thresholdPe(ThresholdPrecision - 1, 0) = thresMem[pe][nm](ThresholdPrecision - 1, 0);
					for(unsigned int v = 0; v < NumVecs; v++) {
#pragma HLS UNROLL	
						outputPe[v] = ReducedPrecision_Threshold<ActivationPrecision, MacPrecision, ThresholdPrecision/NumberOfThresholds, 
							NumberOfThresholds-1>(macRegisters[v][pe], thresholdPe);
					}
				}

				// Assign to right bits of output buffers
				unsigned int lowBit = pe * ActivationPrecision;
				unsigned int highBit = (pe+1) * ActivationPrecision - 1;
				for(unsigned int v = 0; v < NumVecs; v++) {
#pragma HLS UNROLL	
					outElem.data[v](highBit, lowBit) = outputPe[v](ActivationPrecision-1, 0);
					macRegisters[v][pe] = 0;	// clear the accumulator
				}
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


template<unsigned int ConvKernelDim, 
		unsigned int IFMChannels,
		unsigned int Input_precision,		// Number of bits for each pixel
		unsigned int IFMDim, 
		unsigned int OFMDim,
		unsigned int Stride = 1, 
		unsigned int NumVecs=1 	>		// MMV value, related to output bandwidth 
void ConvolutionMMVInputGenerator_kernel_stride(  // This Input generator should be used when kernel%stride !=0 (e.g.m k=3 and stride=2)
    stream<ap_uint<IFMChannels*Input_precision> > &in,
    stream<MultiChanData<NumVecs, IFMChannels*Input_precision> > & out,
	const unsigned int numReps = 1
    ){
	constexpr unsigned int number_blocks = ConvKernelDim + Stride ;
	constexpr unsigned int cycles_write_block = (OFMDim * ConvKernelDim * ConvKernelDim)/NumVecs;
	constexpr unsigned int cycles_read_block = IFMDim*Stride;
	constexpr unsigned int max_cycles = MAX(cycles_write_block, cycles_read_block);
	constexpr unsigned int baseIter = IFMDim * ConvKernelDim + OFMDim * max_cycles;
	constexpr unsigned int initial_buffer_cycles = IFMDim * ConvKernelDim;

	unsigned int counter_internal_block = 0;
	unsigned int current_block_write = 0;
	unsigned int next_block_write = 0;
	unsigned int current_line = 0;
	unsigned int read_block = 0;


	unsigned int inp = 0, ofm_y = 0, ofm_x = 0, k_y = 0, k_x = 0, current_k_y = 0;

	ap_uint<IFMChannels*Input_precision> inputBuf[NumVecs][number_blocks][IFMDim];
#pragma HLS ARRAY_PARTITION variable=inputBuf complete dim=1
#pragma HLS ARRAY_PARTITION variable=inputBuf complete dim=2

#pragma HLS RESET variable=read_block
#pragma HLS DEPENDENCE variable=read_block intra false
#pragma HLS RESET variable=inp

#pragma HLS DEPENDENCE variable=current_block_write intra false
#pragma HLS DEPENDENCE variable=inputBuf inter false
#pragma HLS DEPENDENCE variable=inputBuf intra false

// #pragma HLS RESOURCE variable inputBuf core=RAM_2P_LUTRAM
for (unsigned int count_image = 0; count_image < numReps; count_image++) {
		for (unsigned int i = 0; i < baseIter; i++) {
	#pragma HLS PIPELINE II=1
			if (inp < initial_buffer_cycles) // Initial buffer of PoolDim lines
			{
				ap_uint<IFMChannels*Input_precision> inElem;
				inElem = in.read();
				for(unsigned int v = 0; v < NumVecs; v++)
					{
	#pragma HLS UNROLL
					inputBuf[v][current_block_write][current_line] = inElem;
					}
				current_line++;
				inp++;
				if (current_line == IFMDim)
				{
					current_line = 0;
					current_block_write++;
					if (current_block_write == number_blocks)
						current_block_write = 0;
					read_block++;
					counter_internal_block = 0;
				}
			}
			else
			{
				if (counter_internal_block < cycles_write_block-1) // We are writing output, MMV IFMChan per cycle
				{
					unsigned int current_block_read = (ofm_y*Stride + k_y)%number_blocks;
					unsigned int current_line_in_block = ofm_x * Stride + k_x;
					MultiChanData<NumVecs, IFMChannels*Input_precision> outElem;
					for(unsigned int v = 0; v < NumVecs; v++) {
	#pragma HLS UNROLL
						// each buffer's read addr is offset by its buffer index
						ap_uint<IFMChannels*Input_precision> temp_value = inputBuf[v][current_block_read][(current_line_in_block + v*Stride)];
						outElem.data[v] = temp_value;
					}
					out.write(outElem);
					k_x++;
					if (k_x == ConvKernelDim) {
						k_x = 0;
						k_y++;
						if (k_y == ConvKernelDim) {
							k_y = 0;
							ofm_x += NumVecs;
							if (ofm_x == OFMDim) {
								ofm_x = 0;
								ofm_y++;
								if (ofm_y == OFMDim) {
									ofm_y = 0;
									inp = 0;
								}
							}
						}
					}
				}
				if ((counter_internal_block < cycles_read_block - 1) && (read_block<IFMDim)) // In parallel we write in the buffer, in the current block write if we still need to
				{
					ap_uint<IFMChannels*Input_precision> inElem;
					inElem = in.read();

					for(unsigned int v = 0; v < NumVecs; v++)
						{
	#pragma HLS UNROLL
						inputBuf[v][current_block_write][current_line] = inElem;
						}
		//#pragma HLS DEPENDENCE variable=inputBuf intra false
		#pragma HLS DEPENDENCE variable=inputBuf inter false
					current_line++;
					if (current_line == IFMDim) // We read the whole block, we change the next block in which we want to we
					{ // We filled up a block, let's not read until
						current_line = 0;
						read_block++;
						current_block_write++;
						if (current_block_write == number_blocks)
							current_block_write = 0;
	#pragma HLS DEPENDENCE variable=current_block_write intra false
					}
				}
				counter_internal_block++; // = (counter_internal_block +1) % max_cycles;
				if (counter_internal_block == (max_cycles-1))
				{
					counter_internal_block = 0;
				}

			}
		} // End base_iter
		current_block_write=0;
		read_block=0;
    }
}



template<
		// convolution parameters
		unsigned int ConvKernelDim,		// e.g 3 for a 3x3 conv kernel (assumed square)
		unsigned int IFMChannels,		// number of input feature maps
		unsigned int IFMDim,			// width of input feature map (assumed square)
		unsigned int OFMChannels,		// number of output feature maps
		unsigned int Stride,

		// matrix-vector unit parameters
		unsigned int SIMDWidth,			// number of SIMD lanes
		unsigned int PECount,			// number of PEs
		unsigned int WMemCount,			// entries in each PEs weight memory
		unsigned int TMemCount,			// entries in each PEs threshold memory

		// precision parameters
		unsigned int WeightsPrecision,	// Number of bits in thresholds
		unsigned int ThresholdPrecision,// Number of bits in thresholds
		unsigned int MacPrecision,		// MAC bitwidth
		unsigned int Input_precision,			// Input data bitwidth
		unsigned int ActivationPrecision,	//Output data bitwidth
		unsigned int NumVecs = 1,
		unsigned int ActivationType=0,
		template<int> class type_input 	= ap_uint	// For first layer use int value	
>
void ConvolutionalLayerMMV_Same_Batch_kernel_stride_dsp_packed(stream<ap_uint<IFMChannels * Input_precision> > & in,
		stream<ap_uint<OFMChannels * ActivationPrecision> > & out,
		const ap_uint<SIMDWidth * WeightsPrecision> weightMem[PECount][WMemCount],
		const ap_uint<ThresholdPrecision> threshMem[PECount][TMemCount], const unsigned int numReps) {
#pragma HLS INLINE

	// Number of output windows
	constexpr unsigned int OFMDim = 1 + (IFMDim - Stride) / Stride + (((IFMDim - Stride) % Stride) > 0);

	// Output dimensions of the resize stage
	constexpr unsigned int intermediateDimension = ConvKernelDim + Stride * (OFMDim - 1);
	// compute weight matrix dimension from conv params
	constexpr unsigned int MatrixW = ConvKernelDim * ConvKernelDim * IFMChannels;
	constexpr unsigned int MatrixH = OFMChannels;
	const unsigned int mmvReps = (numReps * OFMDim * OFMDim) / NumVecs;
	
	stream<ap_uint<IFMChannels * Input_precision> > resizedInput("resizedInput");
#pragma HLS RESOURCE variable=resizedInput core=FIFO_LUTRAM
#pragma HLS STREAM variable=resizedInput depth=32
	stream<MultiChanData<NumVecs, IFMChannels * Input_precision> >swu2dwc("swu2dwc");
#pragma HLS RESOURCE variable=swu2dwc core=FIFO_LUTRAM
#pragma HLS STREAM variable=swu2dwc depth=32
	stream<MultiChanData<NumVecs, SIMDWidth * Input_precision> > dwc2mmv("dwc2mmv");
#pragma HLS RESOURCE variable=dwc2mmv core=FIFO_LUTRAM
#pragma HLS STREAM variable=dwc2mmv depth=32
	stream<MultiChanData<NumVecs, PECount * ActivationPrecision> > mmv2dwc("mmv2dwc");
#pragma HLS RESOURCE variable=mmv2dwc core=FIFO_LUTRAM
#pragma HLS STREAM variable=mmv2dwc depth=32
	stream<MultiChanData<NumVecs, OFMChannels * ActivationPrecision> > dwc2flatten("dwc2flatten");
#pragma HLS RESOURCE variable=dwc2flatten core=FIFO_LUTRAM
#pragma HLS STREAM variable=dwc2flatten depth=32
	stream<ap_uint<NumVecs * OFMChannels * ActivationPrecision> > flatten2serialize("flatten2serialize");
#pragma HLS RESOURCE variable=flatten2serialize core=FIFO_LUTRAM
#pragma HLS STREAM variable=flatten2serialize depth=32

	SameResize_Batch<IFMDim, ConvKernelDim, Stride, IFMChannels, Input_precision, 2>(in, resizedInput, numReps);	
	ConvolutionMMVInputGenerator_kernel_stride<ConvKernelDim, IFMChannels, Input_precision, intermediateDimension,
			OFMDim, Stride, NumVecs>(resizedInput, swu2dwc, numReps);
	MultiChanDataWidthConverter_Batch<IFMChannels * Input_precision, SIMDWidth * Input_precision,
		(OFMDim * OFMDim * ConvKernelDim * ConvKernelDim) / NumVecs,
		NumVecs>(swu2dwc, dwc2mmv, numReps);

	MatrixMultiVector_Precision_Batch_dsp_packed<
		SIMDWidth, PECount, WeightsPrecision, ThresholdPrecision, MatrixW, MatrixH, WMemCount,
		TMemCount, Input_precision, ActivationPrecision, MacPrecision, NumVecs, ActivationType, type_input >(dwc2mmv, mmv2dwc, weightMem, threshMem, mmvReps);

	MultiChanDataWidthConverter_Batch<
		PECount * ActivationPrecision, OFMChannels * ActivationPrecision, (OFMDim * OFMDim * OFMChannels) / (NumVecs * PECount)>(mmv2dwc, dwc2flatten, numReps);
	FlattenMultiChanData<NumVecs, OFMChannels * ActivationPrecision>(dwc2flatten, flatten2serialize, mmvReps);
	DataWidthConverter_Batch<OFMChannels * NumVecs * ActivationPrecision, OFMChannels * ActivationPrecision , 1>(flatten2serialize, out, mmvReps);
}
