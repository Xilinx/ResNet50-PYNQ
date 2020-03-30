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

#include "bnn-library.h"
#include "weights.hpp"
#include "activations.hpp"

/* Set of residual layers for ResNet50, with special features:
 * 1) Branch2a has always Kernel=1 -> no SWG needed. When stride =1 nothing is needed, when stride=2 we throw away every other word
 * 2) Branch2b has always Kernel=3 and stride = 1 
 * 3) Branch2c has always Kernel=1 and stride = 1  -> no SWG needed at all
 * 4) When present, Branch1 has always Kernel=1 -> no SWG needed. When stride =1 nothing is needed, when stride=2 we throw away every other word
 * 5) In residual_1branch, dimensions of FM are constant though the layers
*/

// Simplified SWG when kernel=1 
// when stride=2 it throws away every other word and every other line
// when stride=1 it adds 1 cycle of latency and does nothing
template<unsigned int StreamWidth,		// stream width
		unsigned int FMDim, 	// number of pixels to pass through
		unsigned int Fold,  //number of input words per pixel
		unsigned int Stride // total number of words (NumTotal-NumAllowed swallowed)
>
void SWG_kernel1_Batch(stream<ap_uint<StreamWidth> > & in,
		stream<ap_uint<StreamWidth> > & out, const unsigned int numReps) {
	for (unsigned int im=0; im<numReps; im++) {
		for (unsigned int i = 0; i < FMDim; i++) {
			for (unsigned int j = 0; j < FMDim*Fold; j++) {
	#pragma HLS PIPELINE II=1
				ap_uint<StreamWidth> e = in.read();
				if (((j/Fold)%Stride == 0)&&(i%Stride == 0)) {
					out.write(e);
				}
			}
		}
	}		
}

// In ResNet networks, when when we have 2 branches, we have 3 convolutions on one side, and 1 convolution to be executed in parallel on the other side

template <
	unsigned int BYP_INBITS, unsigned int BYP_THBITS, unsigned int BYP_THPE, unsigned int BYP_THTMEM,

	unsigned int BR2A_IFMC, unsigned int BR2A_OFMC, unsigned int BR2A_IFMDIM, unsigned int BR2A_OFMDIM, unsigned int BR2A_STRIDE,	  // Residual layer parameters
	unsigned int BR2A_SIMD, unsigned int BR2A_PE, unsigned int BR2A_WMEM, unsigned int BR2A_TMEM,									   // Branch 2a parallelism
	typename BR2A_WINTERPRET, unsigned int BR2A_THBITS, unsigned int BR2A_MACBITS, unsigned int BR2A_INBITS, unsigned int BR2A_ACTBITS, // Branch 2a precision parameters

	unsigned int BR2B_IFMC, unsigned int BR2B_OFMC, unsigned int BR2B_IFMDIM, unsigned int BR2B_OFMDIM, unsigned int BR2B_STRIDE,
	unsigned int BR2B_SIMD, unsigned int BR2B_PE, unsigned int BR2B_WMEM, unsigned int BR2B_TMEM,									   // Branch 2b parallelism
	typename BR2B_WINTERPRET, unsigned int BR2B_THBITS, unsigned int BR2B_MACBITS, unsigned int BR2B_INBITS, unsigned int BR2B_ACTBITS, // Branch 2b precision parameters

	unsigned int BR2C_IFMC, unsigned int BR2C_OFMC, unsigned int BR2C_IFMDIM, unsigned int BR2C_OFMDIM, unsigned int BR2C_STRIDE,
	unsigned int BR2C_SIMD, unsigned int BR2C_PE, unsigned int BR2C_WMEM, unsigned int BR2C_TMEM,									   // Branch 2c parallelism
	typename BR2C_WINTERPRET, unsigned int BR2C_THBITS, unsigned int BR2C_MACBITS, unsigned int BR2C_INBITS, unsigned int BR2C_ACTBITS, // Branch 2c precision parameters

	unsigned int BR1_IFMC, unsigned int BR1_OFMC, unsigned int BR1_IFMDIM, unsigned int BR1_OFMDIM, unsigned int BR1_STRIDE,
	unsigned int BR1_SIMD, unsigned int BR1_PE, unsigned int BR1_WMEM, unsigned int BR1_TMEM,									 // Branch 1 parallelism
	typename BR1_WINTERPRET, unsigned int BR1_THBITS, unsigned int BR1_MACBITS, unsigned int BR1_INBITS, unsigned int BR1_ACTBITS, // Branch 1 precision parameters

	typename BR2A_WT, typename BR2B_WT, typename BR2C_WT, typename BR1_WT

	>
void Residual_2branches(
	stream<ap_uint<BR2A_SIMD * BYP_INBITS>> &in, stream<ap_uint<BR1_PE * BR1_ACTBITS>> &out,
	ThresholdsActivation<BYP_THTMEM, BR2A_SIMD, (1 << BR2A_INBITS) - 1 , ap_uint<BYP_THBITS/(1 << BR2A_INBITS)>, ap_uint<BR2A_INBITS>, 0, std::less_equal<ap_uint<BYP_THBITS/(1 << BR2A_INBITS)>>> &threshMemBYP,
	BR2A_WT &weightMem2A, ThresholdsActivation<BR2A_TMEM, BR2A_PE, (1 << BR2A_ACTBITS) - 1, ap_int<BR2A_MACBITS>, ap_uint<BR2A_ACTBITS>, 0, std::less_equal<ap_int<BR2A_MACBITS>>> &threshMem2A,
	BR2B_WT &weightMem2B, ThresholdsActivation<BR2B_TMEM, BR2B_PE, (1 << BR2B_ACTBITS) - 1, ap_int<BR2B_MACBITS>, ap_uint<BR2B_ACTBITS>, 0, std::less_equal<ap_int<BR2B_MACBITS>>> &threshMem2B,
	BR2C_WT &weightMem2C, ThresholdsActivation<BR2C_TMEM, BR2C_PE, (1 << BR2C_ACTBITS) - 1, ap_int<BR2C_MACBITS>, ap_uint<BR2C_ACTBITS>, 0, std::less_equal<ap_int<BR2C_MACBITS>>> &threshMem2C,
	BR1_WT &weightMem1, ThresholdsActivation<BR1_TMEM, BR1_PE, (1 << BR1_ACTBITS) - 1, ap_int<BR1_MACBITS>, ap_uint<BR1_ACTBITS>, 0, std::less_equal<ap_int<BR1_MACBITS>>> &threshMem1,
	const unsigned int numReps)
{
#pragma HLS INLINE
	CASSERT_DATAFLOW(BR1_INBITS == BR2A_INBITS);
        CASSERT_DATAFLOW(BYP_INBITS >= BR2A_INBITS);
        CASSERT_DATAFLOW(BR1_ACTBITS == BR2C_ACTBITS);
        CASSERT_DATAFLOW(BR1_OFMC == BR2C_OFMC);
        CASSERT_DATAFLOW(BR1_OFMDIM == BR2C_OFMDIM);
        CASSERT_DATAFLOW(BR1_OFMC % (BR2A_IFMC/BYP_THPE) == 0);
        stream<ap_uint<BR2A_SIMD*BR2A_INBITS> > duplicate_in("duplicate_in");
#pragma HLS RESOURCE variable=duplicate_in core=FIFO_LUTRAM
#pragma HLS STREAM variable=duplicate_in depth=16
	stream<ap_uint<BR2A_SIMD*BR2A_INBITS> > conv_br2a_in("conv_br2a_in");
#pragma HLS RESOURCE variable=conv_br2a_in core=FIFO_LUTRAM
#pragma HLS STREAM variable=conv_br2a_in depth=16
	stream<ap_uint<BR2A_SIMD*BR2A_INBITS> > bypass_fifo("bypass_fifo");
#pragma HLS STREAM variable=bypass_fifo depth=BR2A_IFMDIM*4*BR2A_STRIDE*(BR2A_IFMC/BR2A_SIMD)
	stream<ap_uint<BR1_SIMD*BR1_INBITS> > conv_br1_in("conv_br1_inter");
#pragma HLS RESOURCE variable=conv_br1_in core=FIFO_LUTRAM
#pragma HLS STREAM variable=conv_br1_in depth=16
	stream<ap_uint<BR1_PE*BR1_ACTBITS> > conv_br1_out("conv_br1_out");
#pragma HLS RESOURCE variable=conv_br1_out core=FIFO_LUTRAM
#pragma HLS STREAM variable=conv_br1_out depth=16
	stream<ap_uint<BR2A_PE*BR2A_ACTBITS> > conv_br2a_out("conv_br2a_out");
#pragma HLS RESOURCE variable=conv_br2a_out core=FIFO_LUTRAM
#pragma HLS STREAM variable=conv_br2a_out depth=16
	stream<ap_uint<BR2B_IFMC*BR2B_INBITS> > conv_br2b_in("conv_br2b_in");
#pragma HLS RESOURCE variable=conv_br2b_in core=FIFO_LUTRAM
#pragma HLS STREAM variable=conv_br2b_in depth=16
	stream<ap_uint<BR2B_OFMC*BR2B_ACTBITS> > conv_br2b_out("conv_br2b_out");
#pragma HLS RESOURCE variable=conv_br2b_out core=FIFO_LUTRAM
#pragma HLS STREAM variable=conv_br2b_out depth=16
	stream<ap_uint<BR2C_SIMD*BR2C_INBITS> > conv_br2c_in("conv_br2c_in");
#pragma HLS RESOURCE variable=conv_br2c_in core=FIFO_LUTRAM
#pragma HLS STREAM variable=conv_br2c_in depth=16
	stream<ap_uint<BR2C_PE*BR2C_ACTBITS> > conv_br2c_out("conv_br2c_out");
#pragma HLS RESOURCE variable=conv_br2c_out core=FIFO_LUTRAM
#pragma HLS STREAM variable=conv_br2c_out depth=16
	stream<ap_uint<BR2C_PE*BR2C_ACTBITS> > conv_br1_out_resize("conv_br1_out_resize");
#pragma HLS RESOURCE variable=conv_br1_out_resize core=FIFO_LUTRAM
#pragma HLS STREAM variable=conv_br1_out_resize depth=16
        
        Thresholding_Batch <BR2A_IFMDIM, BR2A_IFMC, BR2A_SIMD, Slice<ap_uint<BYP_INBITS>>, Slice<ap_uint<BR2A_INBITS>>>(in, duplicate_in, threshMemBYP, numReps);
        
	DuplicateStreams_Batch<BR2A_SIMD*BR2A_INBITS, BR2A_IFMDIM*BR2A_IFMDIM*(BR2A_IFMC/BR2A_SIMD)>(duplicate_in, conv_br2a_in, bypass_fifo, numReps);

	// -- BR2A: 1 x 1 Convolution
	// Number of output windows
	constexpr unsigned int OFMDim_2a = 1 + (BR2A_IFMDIM - BR2A_STRIDE) / BR2A_STRIDE + (((BR2A_IFMDIM - BR2A_STRIDE) % BR2A_STRIDE) > 0);
	// Output dimensions of the resize stage
	constexpr unsigned int intermediateDimension_2a = BR2A_IFMDIM / BR2A_STRIDE;
	// Feed everything to the MVAU
	unsigned const MatrixW_2a = BR2A_IFMC;
	unsigned const MatrixH_2a = BR2A_OFMC;

	if (BR2A_STRIDE > 1) {
		stream<ap_uint<BR2A_SIMD * BR2A_INBITS>> convInp("res_2branch_BR2A.convInp");

		SWG_kernel1_Batch<BR2A_SIMD * BR2A_INBITS, BR2A_IFMDIM, BR2A_IFMC / BR2A_SIMD, BR2A_STRIDE>(conv_br2a_in, convInp, numReps);

		Matrix_Vector_Activate_Batch<MatrixW_2a, MatrixH_2a, BR2A_SIMD, BR2A_PE, 1, Slice<ap_uint<BR2A_INBITS>>, Slice<ap_uint<BR2A_ACTBITS>>, BR2A_WINTERPRET >
				(static_cast<hls::stream<ap_uint<BR2A_SIMD * BR2A_INBITS>>&>(convInp),
				static_cast<hls::stream<ap_uint<BR2A_PE * BR2A_ACTBITS>>&>  (conv_br2a_out),
				weightMem2A, threshMem2A, numReps * OFMDim_2a * OFMDim_2a, ap_resource_lut());
	}
	else {
		Matrix_Vector_Activate_Batch<MatrixW_2a, MatrixH_2a, BR2A_SIMD, BR2A_PE, 1, Slice<ap_uint<BR2A_INBITS>>, Slice<ap_uint<BR2A_ACTBITS>>, BR2A_WINTERPRET >
				(static_cast<hls::stream<ap_uint<BR2A_SIMD * BR2A_INBITS>>&>(conv_br2a_in),
				static_cast<hls::stream<ap_uint<BR2A_PE * BR2A_ACTBITS>>&>  (conv_br2a_out),
				weightMem2A, threshMem2A, numReps * OFMDim_2a * OFMDim_2a, ap_resource_lut());
	}
	StreamingDataWidthConverter_Batch<BR2A_PE * BR2A_ACTBITS, BR2B_IFMC * BR2B_INBITS, BR2A_OFMDIM * BR2A_OFMDIM *(BR2A_OFMC / BR2A_PE)>(conv_br2a_out, conv_br2b_in, numReps);

	// -- BR2B: 3 x 3 Convolution
	// Number of output windows
	constexpr unsigned int OFMDim = 1 + (BR2B_IFMDIM - BR2B_STRIDE) / BR2B_STRIDE + (((BR2B_IFMDIM - BR2B_STRIDE) % BR2B_STRIDE) > 0);
	// Output dimensions of the resize stage
	constexpr unsigned int intermediateDimension = 3 + BR2B_STRIDE * (OFMDim - 1);
	// Padding
	stream<ap_uint<BR2B_IFMC * BR2B_INBITS>> resizedInp("BR2B.resizedInput");
	SameResize_Batch<BR2B_IFMDIM, 3, BR2B_STRIDE, BR2B_IFMC, ap_uint<BR2B_INBITS>>(conv_br2b_in, resizedInp, numReps);

	// DWC from inp -> parallel in format
	unsigned const instreamW_RAW_2B = BR2B_IFMC * BR2B_INBITS;
	unsigned const instreamW_DWC_2B = BR2B_SIMD * BR2B_INBITS;
	unsigned const instreamWords_2B = intermediateDimension * intermediateDimension;
	WidthAdjustedInputStream<instreamW_RAW_2B, instreamW_DWC_2B, instreamWords_2B> dwc_2B_in(resizedInp, numReps);

	// DWC from parallel out format -> outp
	unsigned const outstreamW_RAW_2B = BR2B_PE * BR2B_ACTBITS;
	unsigned const outstreamW_DWC_2B = BR2B_OFMC * BR2B_ACTBITS;
	unsigned const outstreamWords_2B = OFMDim * OFMDim * (BR2B_OFMC / BR2B_PE);
	
	// Generate conv input stream (parallel in width)
	stream<ap_uint<BR2B_SIMD * BR2B_INBITS>> convInp("BR2B.convInp");

        if (BR2B_STRIDE > 1) {
	        ConvolutionInputGenerator_kernel_stride<3, BR2B_IFMC, BR2B_INBITS, intermediateDimension, OFMDim, BR2B_SIMD, BR2B_STRIDE>(dwc_2B_in, convInp, numReps);
        } else {
	        ConvolutionInputGenerator<3, BR2B_IFMC, BR2B_INBITS, intermediateDimension, OFMDim, BR2B_SIMD, BR2B_STRIDE>(dwc_2B_in, convInp, numReps);
        }

	// Feed everything to the MVAU
	unsigned const MatrixW = 3 * 3 * BR2B_IFMC;
	unsigned const MatrixH = BR2B_OFMC;
	stream<ap_uint<BR2B_PE * BR2B_ACTBITS>> dwc_2B_out("BR2B.convOut");
	Matrix_Vector_Activate_Batch<MatrixW, MatrixH, BR2B_SIMD, BR2B_PE, 1, Slice<ap_uint<BR2B_INBITS>>, Slice<ap_uint<BR2B_ACTBITS>>, BR2B_WINTERPRET >
			(static_cast<hls::stream<ap_uint<BR2B_SIMD * BR2B_INBITS>>&>(convInp),
			static_cast<hls::stream<ap_uint<BR2B_PE * BR2B_ACTBITS>>&>  (dwc_2B_out),
			weightMem2B, threshMem2B, numReps * OFMDim * OFMDim, ap_resource_lut());
	
	WidthAdjustedInputStream<outstreamW_RAW_2B, BR2C_SIMD * BR2C_INBITS, outstreamWords_2B> wa_in_2c(dwc_2B_out, numReps);

	// -- BR2C: 1 x 1 Convolution
	// Number of output windows
	constexpr unsigned int OFMDim_2c = 1 + (BR2C_IFMDIM - BR2C_STRIDE) / BR2C_STRIDE + (((BR2C_IFMDIM - BR2C_STRIDE) % BR2C_STRIDE) > 0);

	// Feed everything to the MVAU
	unsigned const MatrixW_2c = BR2C_IFMC;
	unsigned const MatrixH_2c = BR2C_OFMC;
	Matrix_Vector_Activate_Batch<MatrixW_2c, MatrixH_2c, BR2C_SIMD, BR2C_PE, 1, Slice<ap_uint<BR2C_INBITS>>, Slice<ap_uint<BR2C_ACTBITS>>, BR2C_WINTERPRET >
			(static_cast<hls::stream<ap_uint<BR2C_SIMD * BR2C_INBITS>>&>(wa_in_2c),
			static_cast<hls::stream<ap_uint<BR2C_PE * BR2C_ACTBITS>>&>  (conv_br2c_out),
			weightMem2C, threshMem2C, numReps * OFMDim_2c * OFMDim_2c, ap_resource_lut());


	// -- BR1: 1 x 1 Convolution
	StreamingDataWidthConverter_Batch<BR2A_SIMD * BR2A_INBITS, BR1_SIMD * BR1_INBITS, BR2A_IFMDIM * BR2A_IFMDIM *(BR2A_IFMC / BR2A_SIMD)>(bypass_fifo, conv_br1_in, numReps);
	// Number of output windows
	constexpr unsigned int OFMDim_1 = 1 + (BR2C_IFMDIM - BR2C_STRIDE) / BR2C_STRIDE + (((BR2C_IFMDIM - BR2C_STRIDE) % BR2C_STRIDE) > 0);
	// Output dimensions of the resize stage
	constexpr unsigned int intermediateDimension_1 = BR1_IFMDIM / BR1_STRIDE;
	// Feed everything to the MVAU
	unsigned const MatrixW_1 = BR1_IFMC;
	unsigned const MatrixH_1 = BR1_OFMC;

	if (BR1_STRIDE > 1) {
		stream<ap_uint<BR1_SIMD * BR1_INBITS>> convInp("res_2branch_BR1.convInp");

		SWG_kernel1_Batch<BR1_SIMD * BR1_INBITS, BR1_IFMDIM, BR1_IFMC / BR1_SIMD, BR1_STRIDE>(conv_br1_in, convInp, numReps);

		Matrix_Vector_Activate_Batch<MatrixW_1, MatrixH_1, BR1_SIMD, BR1_PE, 1, Slice<ap_uint<BR1_INBITS>>, Slice<ap_uint<BR1_ACTBITS>>, BR1_WINTERPRET >
				(static_cast<hls::stream<ap_uint<BR1_SIMD * BR1_INBITS>>&>(convInp),
				static_cast<hls::stream<ap_uint<BR1_PE * BR1_ACTBITS>>&>  (conv_br1_out),
				weightMem1, threshMem1, numReps * OFMDim_1 * OFMDim_1, ap_resource_lut());
	}
	else {
		Matrix_Vector_Activate_Batch<MatrixW_1, MatrixH_1, BR1_SIMD, BR1_PE, 1, Slice<ap_uint<BR1_INBITS>>, Slice<ap_uint<BR1_ACTBITS>>, BR1_WINTERPRET >
				(static_cast<hls::stream<ap_uint<BR1_SIMD * BR1_INBITS>>&>(conv_br1_in),
				static_cast<hls::stream<ap_uint<BR1_PE * BR1_ACTBITS>>&>  (conv_br1_out),
				weightMem1, threshMem1, numReps * OFMDim_1 * OFMDim_1, ap_resource_lut());
	}

	StreamingDataWidthConverter_Batch<BR1_PE*BR1_ACTBITS, BR2C_PE*BR2C_ACTBITS, BR1_OFMDIM*BR1_OFMDIM*(BR1_OFMC/BR1_PE)>(conv_br1_out, conv_br1_out_resize, numReps);	
	AddStreams_Batch<BR2C_PE, ap_uint<BR2C_ACTBITS>, ap_uint<BR2C_ACTBITS>, ap_ufixed<BR2C_ACTBITS, BR2C_ACTBITS, AP_RND, AP_SAT_SYM>, BR2C_OFMDIM*BR2C_OFMDIM*(BR2C_OFMC/BR2C_PE), -8>(conv_br2c_out, conv_br1_out_resize, out, numReps);
}

template <
	unsigned int BYP_INBITS, unsigned int BYP_THBITS, unsigned int BYP_THPE, unsigned int BYP_THTMEM,

	unsigned int BR2A_IFMC, unsigned int BR2A_OFMC, unsigned int BR2A_IFMDIM, unsigned int BR2A_OFMDIM, unsigned int BR2A_STRIDE,	  // Residual layer parameters
	unsigned int BR2A_SIMD, unsigned int BR2A_PE, unsigned int BR2A_WMEM, unsigned int BR2A_TMEM,									   // Branch 2a parallelism
	typename BR2A_WINTERPRET, unsigned int BR2A_THBITS, unsigned int BR2A_MACBITS, unsigned int BR2A_INBITS, unsigned int BR2A_ACTBITS, // Branch 2a precision parameters

	unsigned int BR2B_IFMC, unsigned int BR2B_OFMC, unsigned int BR2B_IFMDIM, unsigned int BR2B_OFMDIM, unsigned int BR2B_STRIDE,
	unsigned int BR2B_SIMD, unsigned int BR2B_PE, unsigned int BR2B_WMEM, unsigned int BR2B_TMEM,									   // Branch 2b parallelism
	typename BR2B_WINTERPRET, unsigned int BR2B_THBITS, unsigned int BR2B_MACBITS, unsigned int BR2B_INBITS, unsigned int BR2B_ACTBITS, // Branch 2b precision parameters

	unsigned int BR2C_IFMC, unsigned int BR2C_OFMC, unsigned int BR2C_IFMDIM, unsigned int BR2C_OFMDIM, unsigned int BR2C_STRIDE,
	unsigned int BR2C_SIMD, unsigned int BR2C_PE, unsigned int BR2C_WMEM, unsigned int BR2C_TMEM,									  // Branch 2c parallelism
	typename BR2C_WINTERPRET, unsigned int BR2C_THBITS, unsigned int BR2C_MACBITS, unsigned int BR2C_INBITS, unsigned int BR2C_ACTBITS, // Branch 2c precision parameters

	typename BR2A_WT, typename BR2B_WT, typename BR2C_WT

	>
void Residual_1branch(
	stream<ap_uint<BR2A_SIMD * BYP_INBITS>> &in, stream<ap_uint<BR2C_PE * BR2C_ACTBITS>> &out,
	ThresholdsActivation<BYP_THTMEM, BR2A_SIMD, (1 << BR2A_INBITS) - 1 , ap_uint<BYP_THBITS/(1 << BR2A_INBITS)>, ap_uint<BR2A_INBITS>, 0, std::less_equal<ap_uint<BYP_THBITS/(1 << BR2A_INBITS)>>> &threshMemBYP,
	BR2A_WT &weightMem2A, ThresholdsActivation<BR2A_TMEM, BR2A_PE, (1 << BR2A_ACTBITS) - 1, ap_int<BR2A_MACBITS>, ap_uint<BR2A_ACTBITS>, 0, std::less_equal<ap_int<BR2A_MACBITS>>> &threshMem2A,
	BR2B_WT &weightMem2B, ThresholdsActivation<BR2B_TMEM, BR2B_PE, (1 << BR2B_ACTBITS) - 1, ap_int<BR2B_MACBITS>, ap_uint<BR2B_ACTBITS>, 0, std::less_equal<ap_int<BR2B_MACBITS>>> &threshMem2B,
	BR2C_WT &weightMem2C, ThresholdsActivation<BR2C_TMEM, BR2C_PE, (1 << BR2C_ACTBITS) - 1, ap_int<BR2C_MACBITS>, ap_uint<BR2C_ACTBITS>, 0, std::less_equal<ap_int<BR2C_MACBITS>>> &threshMem2C,
	const unsigned int numReps)
{
#pragma HLS INLINE
        CASSERT_DATAFLOW(BYP_INBITS > BR2A_INBITS);
        CASSERT_DATAFLOW(BYP_INBITS == BR2C_ACTBITS);
        CASSERT_DATAFLOW(BR2A_IFMC == BR2C_OFMC);
        CASSERT_DATAFLOW(BR2A_IFMDIM == BR2C_OFMDIM);
	stream<ap_uint<BR2A_SIMD*BYP_INBITS> > thres_br2a_in("thres_br2a_in");
#pragma HLS STREAM variable=thres_br2a_in depth=16
#pragma HLS RESOURCE variable=thres_br2a_in core=FIFO_LUTRAM
	stream<ap_uint<BR2A_SIMD*BR2A_INBITS> > conv_br2a_in("conv_br2a_in");
#pragma HLS STREAM variable=conv_br2a_in depth=16
#pragma HLS RESOURCE variable=conv_br2a_in core=FIFO_LUTRAM
	stream<ap_uint<BR2A_IFMC*BYP_INBITS> > conv_br1_in("conv_br1_in");
#pragma HLS STREAM variable=conv_br1_in depth=16
#pragma HLS RESOURCE variable=conv_br1_in core=FIFO_LUTRAM
	stream<ap_uint<BR2A_SIMD*BYP_INBITS> > bypass_fifo("bypass_fifo");
#pragma HLS STREAM variable=bypass_fifo depth=BR2A_IFMDIM*4*(BR2A_IFMC/BR2A_SIMD)
	stream<ap_uint<BR2C_PE*BR2C_ACTBITS> > conv_br1_out("conv_br1_out");
#pragma HLS STREAM variable=conv_br1_out depth=16
#pragma HLS RESOURCE variable=conv_br1_out core=FIFO_LUTRAM
	stream<ap_uint<BR2A_PE*BR2A_ACTBITS> > conv_br2a_out("conv_br2a_out");
#pragma HLS STREAM variable=conv_br2a_out depth=16
#pragma HLS RESOURCE variable=conv_br2a_out core=FIFO_LUTRAM
	stream<ap_uint<BR2B_IFMC*BR2B_INBITS> > conv_br2b_in("conv_br2b_in");
#pragma HLS STREAM variable=conv_br2b_in depth=16
#pragma HLS RESOURCE variable=conv_br2b_in core=FIFO_LUTRAM
	stream<ap_uint<BR2B_OFMC*BR2B_ACTBITS> > conv_br2b_out("conv_br2b_out");
#pragma HLS STREAM variable=conv_br2b_out depth=16
#pragma HLS RESOURCE variable=conv_br2b_out core=FIFO_LUTRAM
	stream<ap_uint<BR2C_SIMD*BR2C_INBITS> > conv_br2c_in("conv_br2c_in");
#pragma HLS STREAM variable=conv_br2c_in depth=16
#pragma HLS RESOURCE variable=conv_br2c_in core=FIFO_LUTRAM
	stream<ap_uint<BR2C_PE*BR2C_ACTBITS> > conv_br2c_out("conv_br2c_out");
#pragma HLS STREAM variable=conv_br2c_out depth=16
#pragma HLS RESOURCE variable=conv_br2c_out core=FIFO_LUTRAM
		
	DuplicateStreams_Batch<BR2A_SIMD*BYP_INBITS, BR2A_IFMDIM*BR2A_IFMDIM*(BR2A_IFMC/BR2A_SIMD)>(in, thres_br2a_in, bypass_fifo, numReps);
	//split then quantize down if necessary       
        Thresholding_Batch <BR2A_IFMDIM, BR2A_IFMC, BR2A_SIMD, Slice<ap_uint<BYP_INBITS>>, Slice<ap_uint<BR2A_INBITS>>>(thres_br2a_in, conv_br2a_in, threshMemBYP, numReps);

	// -- BR2A: 1 x 1 Convolution
	// Number of output windows
	constexpr unsigned int OFMDim_2a = 1 + (BR2A_IFMDIM - BR2A_STRIDE) / BR2A_STRIDE + (((BR2A_IFMDIM - BR2A_STRIDE) % BR2A_STRIDE) > 0);

	// Feed everything to the MVAU
	unsigned const MatrixW_2a = BR2A_IFMC;
	unsigned const MatrixH_2a = BR2A_OFMC;
	Matrix_Vector_Activate_Batch<MatrixW_2a, MatrixH_2a, BR2A_SIMD, BR2A_PE, 1, Slice<ap_uint<BR2A_INBITS>>, Slice<ap_uint<BR2A_ACTBITS>>, BR2A_WINTERPRET >
			(static_cast<hls::stream<ap_uint<BR2A_SIMD * BR2A_INBITS>>&>(conv_br2a_in),
			static_cast<hls::stream<ap_uint<BR2A_PE * BR2A_ACTBITS>>&>  (conv_br2a_out),
			weightMem2A, threshMem2A, numReps * OFMDim_2a * OFMDim_2a, ap_resource_lut());

	StreamingDataWidthConverter_Batch<BR2A_PE * BR2A_ACTBITS, BR2B_IFMC * BR2B_INBITS, BR2A_OFMDIM*BR2A_OFMDIM*(BR2A_OFMC/BR2A_PE)>(conv_br2a_out, conv_br2b_in, numReps);


	// -- BR2B: 3 x 3 Convolution
	// Number of output windows
	constexpr unsigned int OFMDim = 1 + (BR2B_IFMDIM - BR2B_STRIDE) / BR2B_STRIDE + (((BR2B_IFMDIM - BR2B_STRIDE) % BR2B_STRIDE) > 0);
	// Output dimensions of the resize stage
	constexpr unsigned int intermediateDimension = 3 + BR2B_STRIDE * (OFMDim - 1);
	// Padding
	stream<ap_uint<BR2B_IFMC * BR2B_INBITS>> resizedInp("BR2B.resizedInput");
	SameResize_Batch<BR2B_IFMDIM, 3, BR2B_STRIDE, BR2B_IFMC, ap_uint<BR2B_INBITS>>(conv_br2b_in, resizedInp, numReps);

	// DWC from inp -> parallel in format
	unsigned const instreamW_RAW_2B = BR2B_IFMC * BR2B_INBITS;
	unsigned const instreamW_DWC_2B = BR2B_SIMD * BR2B_INBITS;
	unsigned const instreamWords_2B = intermediateDimension * intermediateDimension;
	WidthAdjustedInputStream<instreamW_RAW_2B, instreamW_DWC_2B, instreamWords_2B> dwc_2B_in(resizedInp, numReps);

	// DWC from parallel out format -> outp
	unsigned const outstreamW_RAW_2B = BR2B_PE * BR2B_ACTBITS;
	unsigned const outstreamW_DWC_2B = BR2B_OFMC * BR2B_ACTBITS;
	unsigned const outstreamWords_2B = OFMDim * OFMDim * (BR2B_OFMC / BR2B_PE);
	
	// Generate conv input stream (parallel in width)
	stream<ap_uint<BR2B_SIMD * BR2B_INBITS>> convInp("BR2B.convInp");
	ConvolutionInputGenerator<3, BR2B_IFMC, BR2B_INBITS, intermediateDimension, OFMDim, BR2B_SIMD, BR2B_STRIDE>(dwc_2B_in, convInp, numReps);

	// Feed everything to the MVAU
	unsigned const MatrixW = 3 * 3 * BR2B_IFMC;
	unsigned const MatrixH = BR2B_OFMC;
	stream<ap_uint<BR2B_PE * BR2B_ACTBITS>> dwc_2B_out("BR2B.convOut");
	Matrix_Vector_Activate_Batch<MatrixW, MatrixH, BR2B_SIMD, BR2B_PE, 1, Slice<ap_uint<BR2B_INBITS>>, Slice<ap_uint<BR2B_ACTBITS>>, BR2B_WINTERPRET >
			(static_cast<hls::stream<ap_uint<BR2B_SIMD * BR2B_INBITS>>&>(convInp),
			static_cast<hls::stream<ap_uint<BR2B_PE * BR2B_ACTBITS>>&>  (dwc_2B_out),
			weightMem2B, threshMem2B, numReps * OFMDim * OFMDim, ap_resource_lut());
	
	// TODO: Convert at once?
	StreamingDataWidthConverter_Batch<outstreamW_RAW_2B, outstreamW_DWC_2B, outstreamWords_2B>(dwc_2B_out, conv_br2b_out, numReps);
	StreamingDataWidthConverter_Batch<BR2B_OFMC * BR2B_ACTBITS, BR2C_SIMD * BR2C_INBITS , OFMDim*OFMDim>(conv_br2b_out, conv_br2c_in, numReps);


	// -- BR2C: 1 x 1 Convolution
	// Number of output windows
	constexpr unsigned int OFMDim_2c = 1 + (BR2C_IFMDIM - BR2C_STRIDE) / BR2C_STRIDE + (((BR2C_IFMDIM - BR2C_STRIDE) % BR2C_STRIDE) > 0);

	// Feed everything to the MVAU
	unsigned const MatrixW_2c = BR2C_IFMC;
	unsigned const MatrixH_2c = BR2C_OFMC;
	Matrix_Vector_Activate_Batch<MatrixW_2c, MatrixH_2c, BR2C_SIMD, BR2C_PE, 1, Slice<ap_uint<BR2C_INBITS>>, Slice<ap_uint<BR2C_ACTBITS>>, BR2C_WINTERPRET >
			(static_cast<hls::stream<ap_uint<BR2C_SIMD * BR2A_INBITS>>&>(conv_br2c_in),
			static_cast<hls::stream<ap_uint<BR2C_PE * BR2C_ACTBITS>>&>  (conv_br2c_out),
			weightMem2C, threshMem2C, numReps * OFMDim_2c * OFMDim_2c, ap_resource_lut());

	StreamingDataWidthConverter_Batch<BR2A_SIMD * BYP_INBITS, BR2C_PE * BR2C_ACTBITS, BR2A_IFMDIM*BR2A_IFMDIM*(BR2A_IFMC/BR2A_SIMD)>(bypass_fifo, conv_br1_out, numReps);
	AddStreams_Batch<BR2C_PE, ap_uint<BR2C_ACTBITS>, ap_uint<BR2C_ACTBITS>, ap_ufixed<BR2C_ACTBITS, BR2C_ACTBITS, AP_RND, AP_SAT_SYM>, BR2C_OFMDIM*BR2C_OFMDIM*(BR2C_OFMC/BR2C_PE), -8>(conv_br2c_out, conv_br1_out, out, numReps);
}



/*
* Residual layers with external weight memory provided as an axi-stream
*/


template <
	unsigned int BYP_INBITS, unsigned int BYP_THBITS, unsigned int BYP_THPE, unsigned int BYP_THTMEM,

	unsigned int BR1_IFMC, unsigned int BR1_OFMC, unsigned int BR1_IFMDIM, unsigned int BR1_OFMDIM, unsigned int BR1_STRIDE,
	unsigned int BR1_SIMD, unsigned int BR1_PE, unsigned int BR1_WMEM, unsigned int BR1_TMEM,									 // Branch 1 parallelism
	unsigned int BR1_WBITS, typename BR1_WINTERPRET, unsigned int BR1_THBITS, unsigned int BR1_MACBITS, unsigned int BR1_INBITS, unsigned int BR1_ACTBITS, // Branch 1 precision parameters

	unsigned int BR2A_IFMC, unsigned int BR2A_OFMC, unsigned int BR2A_IFMDIM, unsigned int BR2A_OFMDIM, unsigned int BR2A_STRIDE,	  // Residual layer parameters
	unsigned int BR2A_SIMD, unsigned int BR2A_PE, unsigned int BR2A_WMEM, unsigned int BR2A_TMEM,									   // Branch 2a parallelism
	unsigned int BR2A_WBITS, typename BR2A_WINTERPRET, unsigned int BR2A_THBITS, unsigned int BR2A_MACBITS, unsigned int BR2A_INBITS, unsigned int BR2A_ACTBITS, // Branch 2a precision parameters

	unsigned int BR2B_IFMC, unsigned int BR2B_OFMC, unsigned int BR2B_IFMDIM, unsigned int BR2B_OFMDIM, unsigned int BR2B_STRIDE,
	unsigned int BR2B_SIMD, unsigned int BR2B_PE, unsigned int BR2B_WMEM, unsigned int BR2B_TMEM,									   // Branch 2b parallelism
	unsigned int BR2B_WBITS, typename BR2B_WINTERPRET, unsigned int BR2B_THBITS, unsigned int BR2B_MACBITS, unsigned int BR2B_INBITS, unsigned int BR2B_ACTBITS, // Branch 2b precision parameters

	unsigned int BR2C_IFMC, unsigned int BR2C_OFMC, unsigned int BR2C_IFMDIM, unsigned int BR2C_OFMDIM, unsigned int BR2C_STRIDE,
	unsigned int BR2C_SIMD, unsigned int BR2C_PE, unsigned int BR2C_WMEM, unsigned int BR2C_TMEM,									   // Branch 2c parallelism
	unsigned int BR2C_WBITS, typename BR2C_WINTERPRET, unsigned int BR2C_THBITS, unsigned int BR2C_MACBITS, unsigned int BR2C_INBITS, unsigned int BR2C_ACTBITS, // Branch 2c precision parameters

	typename BR2A_WT, typename BR2B_WT, typename BR2C_WT, typename BR1_WT

	>
void Residual_2branches_WStream(
	stream<ap_uint<BR2A_SIMD * BYP_INBITS>> &in, stream<ap_uint<BR1_PE * BR1_ACTBITS>> &out,
	ThresholdsActivation<BYP_THTMEM, BR2A_SIMD, (1 << BR2A_INBITS) - 1 , ap_uint<BYP_THBITS/(1 << BR2A_INBITS)>, ap_uint<BR2A_INBITS>, 0, std::less_equal<ap_uint<BYP_THBITS/(1 << BR2A_INBITS)>>> &threshMemBYP,
	BR1_WT &weightMem1, ThresholdsActivation<BR1_TMEM, BR1_PE, (1 << BR1_ACTBITS) - 1, ap_int<BR1_MACBITS>, ap_uint<BR1_ACTBITS>, 0, std::less_equal<ap_int<BR1_MACBITS>>> &threshMem1,
	BR2A_WT &weightMem2A, ThresholdsActivation<BR2A_TMEM, BR2A_PE, (1 << BR2A_ACTBITS) - 1, ap_int<BR2A_MACBITS>, ap_uint<BR2A_ACTBITS>, 0, std::less_equal<ap_int<BR2A_MACBITS>>> &threshMem2A,
	BR2B_WT &weightMem2B, ThresholdsActivation<BR2B_TMEM, BR2B_PE, (1 << BR2B_ACTBITS) - 1, ap_int<BR2B_MACBITS>, ap_uint<BR2B_ACTBITS>, 0, std::less_equal<ap_int<BR2B_MACBITS>>> &threshMem2B,
	BR2C_WT &weightMem2C, ThresholdsActivation<BR2C_TMEM, BR2C_PE, (1 << BR2C_ACTBITS) - 1, ap_int<BR2C_MACBITS>, ap_uint<BR2C_ACTBITS>, 0, std::less_equal<ap_int<BR2C_MACBITS>>> &threshMem2C,
	const unsigned int numReps)
{
#pragma HLS INLINE
	CASSERT_DATAFLOW(BR1_INBITS == BR2A_INBITS);
        CASSERT_DATAFLOW(BYP_INBITS >= BR2A_INBITS);
        CASSERT_DATAFLOW(BR1_ACTBITS == BR2C_ACTBITS);
        CASSERT_DATAFLOW(BR1_OFMC == BR2C_OFMC);
        CASSERT_DATAFLOW(BR1_OFMDIM == BR2C_OFMDIM);
        CASSERT_DATAFLOW(BR1_OFMC % (BR2A_IFMC/BYP_THPE) == 0);
        stream<ap_uint<BR2A_SIMD*BR2A_INBITS> > duplicate_in("duplicate_in");
#pragma HLS RESOURCE variable=duplicate_in core=FIFO_LUTRAM
#pragma HLS STREAM variable=duplicate_in depth=16
	stream<ap_uint<BR2A_SIMD*BR2A_INBITS> > conv_br2a_in("conv_br2a_in");
#pragma HLS RESOURCE variable=conv_br2a_in core=FIFO_LUTRAM
#pragma HLS STREAM variable=conv_br2a_in depth=16
	stream<ap_uint<BR2A_SIMD*BR2A_INBITS> > bypass_fifo("bypass_fifo");
#pragma HLS STREAM variable=bypass_fifo depth=BR2A_IFMDIM*4*BR2A_STRIDE*(BR2A_IFMC/BR2A_SIMD)
	stream<ap_uint<BR1_SIMD*BR1_INBITS> > conv_br1_in("conv_br1_inter");
#pragma HLS RESOURCE variable=conv_br1_in core=FIFO_LUTRAM
#pragma HLS STREAM variable=conv_br1_in depth=16
	stream<ap_uint<BR1_PE*BR1_ACTBITS> > conv_br1_out("conv_br1_out");
#pragma HLS RESOURCE variable=conv_br1_out core=FIFO_LUTRAM
#pragma HLS STREAM variable=conv_br1_out depth=16
	stream<ap_uint<BR2A_PE*BR2A_ACTBITS> > conv_br2a_out("conv_br2a_out");
#pragma HLS RESOURCE variable=conv_br2a_out core=FIFO_LUTRAM
#pragma HLS STREAM variable=conv_br2a_out depth=16
	stream<ap_uint<BR2B_IFMC*BR2B_INBITS> > conv_br2b_in("conv_br2b_in");
#pragma HLS RESOURCE variable=conv_br2b_in core=FIFO_LUTRAM
#pragma HLS STREAM variable=conv_br2b_in depth=16
	stream<ap_uint<BR2B_OFMC*BR2B_ACTBITS> > conv_br2b_out("conv_br2b_out");
#pragma HLS RESOURCE variable=conv_br2b_out core=FIFO_LUTRAM
#pragma HLS STREAM variable=conv_br2b_out depth=16
	stream<ap_uint<BR2C_SIMD*BR2C_INBITS> > conv_br2c_in("conv_br2c_in");
#pragma HLS RESOURCE variable=conv_br2c_in core=FIFO_LUTRAM
#pragma HLS STREAM variable=conv_br2c_in depth=16
	stream<ap_uint<BR2C_PE*BR2C_ACTBITS> > conv_br2c_out("conv_br2c_out");
#pragma HLS RESOURCE variable=conv_br2c_out core=FIFO_LUTRAM
#pragma HLS STREAM variable=conv_br2c_out depth=16
	stream<ap_uint<BR2C_PE*BR2C_ACTBITS> > conv_br1_out_resize("conv_br1_out_resize");
#pragma HLS RESOURCE variable=conv_br1_out_resize core=FIFO_LUTRAM
#pragma HLS STREAM variable=conv_br1_out_resize depth=16
        
        Thresholding_Batch <BR2A_IFMDIM, BR2A_IFMC, BR2A_SIMD, Slice<ap_uint<BYP_INBITS>>, Slice<ap_uint<BR2A_INBITS>>>(in, duplicate_in, threshMemBYP, numReps);
        
	DuplicateStreams_Batch<BR2A_SIMD*BR2A_INBITS, BR2A_IFMDIM*BR2A_IFMDIM*(BR2A_IFMC/BR2A_SIMD)>(duplicate_in, conv_br2a_in, bypass_fifo, numReps);

	// -- BR2A: 1 x 1 Convolution
	// Number of output windows
	constexpr unsigned int OFMDim_2a = 1 + (BR2A_IFMDIM - BR2A_STRIDE) / BR2A_STRIDE + (((BR2A_IFMDIM - BR2A_STRIDE) % BR2A_STRIDE) > 0);
	// Output dimensions of the resize stage
	constexpr unsigned int intermediateDimension_2a = BR2A_IFMDIM / BR2A_STRIDE;
	// Feed everything to the MVAU
	unsigned const MatrixW_2a = BR2A_IFMC;
	unsigned const MatrixH_2a = BR2A_OFMC;

	if (BR2A_STRIDE > 1) {
		stream<ap_uint<BR2A_SIMD * BR2A_INBITS>> convInp("res_2branch_BR2A.convInp");

		SWG_kernel1_Batch<BR2A_SIMD * BR2A_INBITS, BR2A_IFMDIM, BR2A_IFMC / BR2A_SIMD, BR2A_STRIDE>(conv_br2a_in, convInp, numReps);

		Matrix_Vector_Activate_Stream_Batch<MatrixW_2a, MatrixH_2a, BR2A_SIMD, BR2A_PE, Slice<ap_uint<BR2A_INBITS>>, Slice<ap_uint<BR2A_ACTBITS>>, BR2A_WINTERPRET, ap_uint<BR2A_WBITS> >
				(static_cast<hls::stream<ap_uint<BR2A_SIMD * BR2A_INBITS>>&>(convInp),
				static_cast<hls::stream<ap_uint<BR2A_PE * BR2A_ACTBITS>>&>  (conv_br2a_out),
				weightMem2A, threshMem2A, numReps * OFMDim_2a * OFMDim_2a, ap_resource_lut());
	}
	else {
		Matrix_Vector_Activate_Stream_Batch<MatrixW_2a, MatrixH_2a, BR2A_SIMD, BR2A_PE, Slice<ap_uint<BR2A_INBITS>>, Slice<ap_uint<BR2A_ACTBITS>>, BR2A_WINTERPRET, ap_uint<BR2A_WBITS> >
				(static_cast<hls::stream<ap_uint<BR2A_SIMD * BR2A_INBITS>>&>(conv_br2a_in),
				static_cast<hls::stream<ap_uint<BR2A_PE * BR2A_ACTBITS>>&>  (conv_br2a_out),
				weightMem2A, threshMem2A, numReps * OFMDim_2a * OFMDim_2a, ap_resource_lut());
	}
	StreamingDataWidthConverter_Batch<BR2A_PE * BR2A_ACTBITS, BR2B_IFMC * BR2B_INBITS, BR2A_OFMDIM * BR2A_OFMDIM *(BR2A_OFMC / BR2A_PE)>(conv_br2a_out, conv_br2b_in, numReps);

	// -- BR2B: 3 x 3 Convolution
	// Number of output windows
	constexpr unsigned int OFMDim = 1 + (BR2B_IFMDIM - BR2B_STRIDE) / BR2B_STRIDE + (((BR2B_IFMDIM - BR2B_STRIDE) % BR2B_STRIDE) > 0);
	// Output dimensions of the resize stage
	constexpr unsigned int intermediateDimension = 3 + BR2B_STRIDE * (OFMDim - 1);
	// Padding
	stream<ap_uint<BR2B_IFMC * BR2B_INBITS>> resizedInp("BR2B.resizedInput");
	SameResize_Batch<BR2B_IFMDIM, 3, BR2B_STRIDE, BR2B_IFMC, ap_uint<BR2B_INBITS>>(conv_br2b_in, resizedInp, numReps);

	// DWC from inp -> parallel in format
	unsigned const instreamW_RAW_2B = BR2B_IFMC * BR2B_INBITS;
	unsigned const instreamW_DWC_2B = BR2B_SIMD * BR2B_INBITS;
	unsigned const instreamWords_2B = intermediateDimension * intermediateDimension;
	WidthAdjustedInputStream<instreamW_RAW_2B, instreamW_DWC_2B, instreamWords_2B> dwc_2B_in(resizedInp, numReps);

	// DWC from parallel out format -> outp
	unsigned const outstreamW_RAW_2B = BR2B_PE * BR2B_ACTBITS;
	unsigned const outstreamW_DWC_2B = BR2B_OFMC * BR2B_ACTBITS;
	unsigned const outstreamWords_2B = OFMDim * OFMDim * (BR2B_OFMC / BR2B_PE);
	
	// Generate conv input stream (parallel in width)
	stream<ap_uint<BR2B_SIMD * BR2B_INBITS>> convInp("BR2B.convInp");

        if (BR2B_STRIDE > 1) {
	        ConvolutionInputGenerator_kernel_stride<3, BR2B_IFMC, BR2B_INBITS, intermediateDimension, OFMDim, BR2B_SIMD, BR2B_STRIDE>(dwc_2B_in, convInp, numReps);
        } else {
	        ConvolutionInputGenerator<3, BR2B_IFMC, BR2B_INBITS, intermediateDimension, OFMDim, BR2B_SIMD, BR2B_STRIDE>(dwc_2B_in, convInp, numReps);
        }

	// Feed everything to the MVAU
	unsigned const MatrixW = 3 * 3 * BR2B_IFMC;
	unsigned const MatrixH = BR2B_OFMC;
	stream<ap_uint<BR2B_PE * BR2B_ACTBITS>> dwc_2B_out("BR2B.convOut");
	Matrix_Vector_Activate_Stream_Batch<MatrixW, MatrixH, BR2B_SIMD, BR2B_PE, Slice<ap_uint<BR2B_INBITS>>, Slice<ap_uint<BR2B_ACTBITS>>, BR2B_WINTERPRET,ap_int<BR2B_WBITS> >
			(static_cast<hls::stream<ap_uint<BR2B_SIMD * BR2B_INBITS>>&>(convInp),
			static_cast<hls::stream<ap_uint<BR2B_PE * BR2B_ACTBITS>>&>  (dwc_2B_out),
			weightMem2B, threshMem2B, numReps * OFMDim * OFMDim, ap_resource_lut());
	
	WidthAdjustedInputStream<outstreamW_RAW_2B, BR2C_SIMD * BR2C_INBITS, outstreamWords_2B> wa_in_2c(dwc_2B_out, numReps);

	// -- BR2C: 1 x 1 Convolution
	// Number of output windows
	constexpr unsigned int OFMDim_2c = 1 + (BR2C_IFMDIM - BR2C_STRIDE) / BR2C_STRIDE + (((BR2C_IFMDIM - BR2C_STRIDE) % BR2C_STRIDE) > 0);

	// Feed everything to the MVAU
	unsigned const MatrixW_2c = BR2C_IFMC;
	unsigned const MatrixH_2c = BR2C_OFMC;
	Matrix_Vector_Activate_Stream_Batch<MatrixW_2c, MatrixH_2c, BR2C_SIMD, BR2C_PE, Slice<ap_uint<BR2C_INBITS>>, Slice<ap_uint<BR2C_ACTBITS>>, BR2C_WINTERPRET,ap_int<BR2C_WBITS> >
			(static_cast<hls::stream<ap_uint<BR2C_SIMD * BR2C_INBITS>>&>(wa_in_2c),
			static_cast<hls::stream<ap_uint<BR2C_PE * BR2C_ACTBITS>>&>  (conv_br2c_out),
			weightMem2C, threshMem2C, numReps * OFMDim_2c * OFMDim_2c, ap_resource_lut());


	// -- BR1: 1 x 1 Convolution
	StreamingDataWidthConverter_Batch<BR2A_SIMD * BR2A_INBITS, BR1_SIMD * BR1_INBITS, BR2A_IFMDIM * BR2A_IFMDIM *(BR2A_IFMC / BR2A_SIMD)>(bypass_fifo, conv_br1_in, numReps);
	// Number of output windows
	constexpr unsigned int OFMDim_1 = 1 + (BR2C_IFMDIM - BR2C_STRIDE) / BR2C_STRIDE + (((BR2C_IFMDIM - BR2C_STRIDE) % BR2C_STRIDE) > 0);
	// Output dimensions of the resize stage
	constexpr unsigned int intermediateDimension_1 = BR1_IFMDIM / BR1_STRIDE;
	// Feed everything to the MVAU
	unsigned const MatrixW_1 = BR1_IFMC;
	unsigned const MatrixH_1 = BR1_OFMC;

	if (BR1_STRIDE > 1) {
		stream<ap_uint<BR1_SIMD * BR1_INBITS>> convInp("res_2branch_BR1.convInp");

		SWG_kernel1_Batch<BR1_SIMD * BR1_INBITS, BR1_IFMDIM, BR1_IFMC / BR1_SIMD, BR1_STRIDE>(conv_br1_in, convInp, numReps);

		Matrix_Vector_Activate_Stream_Batch<MatrixW_1, MatrixH_1, BR1_SIMD, BR1_PE, Slice<ap_uint<BR1_INBITS>>, Slice<ap_uint<BR1_ACTBITS>>, BR1_WINTERPRET ,ap_int<BR1_WBITS> >
				(static_cast<hls::stream<ap_uint<BR1_SIMD * BR1_INBITS>>&>(convInp),
				static_cast<hls::stream<ap_uint<BR1_PE * BR1_ACTBITS>>&>  (conv_br1_out),
				weightMem1, threshMem1, numReps * OFMDim_1 * OFMDim_1, ap_resource_lut());
	}
	else {
		Matrix_Vector_Activate_Stream_Batch<MatrixW_1, MatrixH_1, BR1_SIMD, BR1_PE, Slice<ap_uint<BR1_INBITS>>, Slice<ap_uint<BR1_ACTBITS>>, BR1_WINTERPRET, ap_int<BR1_WBITS> >
				(static_cast<hls::stream<ap_uint<BR1_SIMD * BR1_INBITS>>&>(conv_br1_in),
				static_cast<hls::stream<ap_uint<BR1_PE * BR1_ACTBITS>>&>  (conv_br1_out),
				weightMem1, threshMem1, numReps * OFMDim_1 * OFMDim_1, ap_resource_lut());
	}

	StreamingDataWidthConverter_Batch<BR1_PE*BR1_ACTBITS, BR2C_PE*BR2C_ACTBITS, BR1_OFMDIM*BR1_OFMDIM*(BR1_OFMC/BR1_PE)>(conv_br1_out, conv_br1_out_resize, numReps);	
	AddStreams_Batch<BR2C_PE, ap_uint<BR2C_ACTBITS>, ap_uint<BR2C_ACTBITS>, ap_ufixed<BR2C_ACTBITS, BR2C_ACTBITS, AP_RND, AP_SAT_SYM>, BR2C_OFMDIM*BR2C_OFMDIM*(BR2C_OFMC/BR2C_PE), -8>(conv_br2c_out, conv_br1_out_resize, out, numReps);
}

template <
	unsigned int BYP_INBITS, unsigned int BYP_THBITS, unsigned int BYP_THPE, unsigned int BYP_THTMEM,

	unsigned int BR2A_IFMC, unsigned int BR2A_OFMC, unsigned int BR2A_IFMDIM, unsigned int BR2A_OFMDIM, unsigned int BR2A_STRIDE,	  // Residual layer parameters
	unsigned int BR2A_SIMD, unsigned int BR2A_PE, unsigned int BR2A_WMEM, unsigned int BR2A_TMEM,									   // Branch 2a parallelism
	unsigned int BR2A_WBITS, typename BR2A_WINTERPRET, unsigned int BR2A_THBITS, unsigned int BR2A_MACBITS, unsigned int BR2A_INBITS, unsigned int BR2A_ACTBITS, // Branch 2a precision parameters

	unsigned int BR2B_IFMC, unsigned int BR2B_OFMC, unsigned int BR2B_IFMDIM, unsigned int BR2B_OFMDIM, unsigned int BR2B_STRIDE,
	unsigned int BR2B_SIMD, unsigned int BR2B_PE, unsigned int BR2B_WMEM, unsigned int BR2B_TMEM,									   // Branch 2b parallelism
	unsigned int BR2B_WBITS, typename BR2B_WINTERPRET, unsigned int BR2B_THBITS, unsigned int BR2B_MACBITS, unsigned int BR2B_INBITS, unsigned int BR2B_ACTBITS, // Branch 2b precision parameters

	unsigned int BR2C_IFMC, unsigned int BR2C_OFMC, unsigned int BR2C_IFMDIM, unsigned int BR2C_OFMDIM, unsigned int BR2C_STRIDE,
	unsigned int BR2C_SIMD, unsigned int BR2C_PE, unsigned int BR2C_WMEM, unsigned int BR2C_TMEM,									  // Branch 2c parallelism
	unsigned int BR2C_WBITS, typename BR2C_WINTERPRET, unsigned int BR2C_THBITS, unsigned int BR2C_MACBITS, unsigned int BR2C_INBITS, unsigned int BR2C_ACTBITS, // Branch 2c precision parameters

	typename BR2A_WT, typename BR2B_WT, typename BR2C_WT

	>
void Residual_1branch_WStream(
	stream<ap_uint<BR2A_SIMD * BYP_INBITS>> &in, stream<ap_uint<BR2C_PE * BR2C_ACTBITS>> &out,
	ThresholdsActivation<BYP_THTMEM, BR2A_SIMD, (1 << BR2A_INBITS) - 1 , ap_uint<BYP_THBITS/(1 << BR2A_INBITS)>, ap_uint<BR2A_INBITS>, 0, std::less_equal<ap_uint<BYP_THBITS/(1 << BR2A_INBITS)>>> &threshMemBYP,
	BR2A_WT &weightMem2A, ThresholdsActivation<BR2A_TMEM, BR2A_PE, (1 << BR2A_ACTBITS) - 1, ap_int<BR2A_MACBITS>, ap_uint<BR2A_ACTBITS>, 0, std::less_equal<ap_int<BR2A_MACBITS>>> &threshMem2A,
	BR2B_WT &weightMem2B, ThresholdsActivation<BR2B_TMEM, BR2B_PE, (1 << BR2B_ACTBITS) - 1, ap_int<BR2B_MACBITS>, ap_uint<BR2B_ACTBITS>, 0, std::less_equal<ap_int<BR2B_MACBITS>>> &threshMem2B,
	BR2C_WT &weightMem2C, ThresholdsActivation<BR2C_TMEM, BR2C_PE, (1 << BR2C_ACTBITS) - 1, ap_int<BR2C_MACBITS>, ap_uint<BR2C_ACTBITS>, 0, std::less_equal<ap_int<BR2C_MACBITS>>> &threshMem2C,
	const unsigned int numReps)
{
#pragma HLS INLINE
        CASSERT_DATAFLOW(BYP_INBITS > BR2A_INBITS);
        CASSERT_DATAFLOW(BYP_INBITS == BR2C_ACTBITS);
        CASSERT_DATAFLOW(BR2A_IFMC == BR2C_OFMC);
        CASSERT_DATAFLOW(BR2A_IFMDIM == BR2C_OFMDIM);
	stream<ap_uint<BR2A_SIMD*BYP_INBITS> > thres_br2a_in("thres_br2a_in");
#pragma HLS STREAM variable=thres_br2a_in depth=16
#pragma HLS RESOURCE variable=thres_br2a_in core=FIFO_LUTRAM
	stream<ap_uint<BR2A_SIMD*BR2A_INBITS> > conv_br2a_in("conv_br2a_in");
#pragma HLS STREAM variable=conv_br2a_in depth=16
#pragma HLS RESOURCE variable=conv_br2a_in core=FIFO_LUTRAM
	stream<ap_uint<BR2A_IFMC*BYP_INBITS> > conv_br1_in("conv_br1_in");
#pragma HLS STREAM variable=conv_br1_in depth=16
#pragma HLS RESOURCE variable=conv_br1_in core=FIFO_LUTRAM
	stream<ap_uint<BR2A_SIMD*BYP_INBITS> > bypass_fifo("bypass_fifo");
#pragma HLS STREAM variable=bypass_fifo depth=BR2A_IFMDIM*4*(BR2A_IFMC/BR2A_SIMD)
	stream<ap_uint<BR2C_PE*BR2C_ACTBITS> > conv_br1_out("conv_br1_out");
#pragma HLS STREAM variable=conv_br1_out depth=16
#pragma HLS RESOURCE variable=conv_br1_out core=FIFO_LUTRAM
	stream<ap_uint<BR2A_PE*BR2A_ACTBITS> > conv_br2a_out("conv_br2a_out");
#pragma HLS STREAM variable=conv_br2a_out depth=16
#pragma HLS RESOURCE variable=conv_br2a_out core=FIFO_LUTRAM
	stream<ap_uint<BR2B_IFMC*BR2B_INBITS> > conv_br2b_in("conv_br2b_in");
#pragma HLS STREAM variable=conv_br2b_in depth=16
#pragma HLS RESOURCE variable=conv_br2b_in core=FIFO_LUTRAM
	stream<ap_uint<BR2B_OFMC*BR2B_ACTBITS> > conv_br2b_out("conv_br2b_out");
#pragma HLS STREAM variable=conv_br2b_out depth=16
#pragma HLS RESOURCE variable=conv_br2b_out core=FIFO_LUTRAM
	stream<ap_uint<BR2C_SIMD*BR2C_INBITS> > conv_br2c_in("conv_br2c_in");
#pragma HLS STREAM variable=conv_br2c_in depth=16
#pragma HLS RESOURCE variable=conv_br2c_in core=FIFO_LUTRAM
	stream<ap_uint<BR2C_PE*BR2C_ACTBITS> > conv_br2c_out("conv_br2c_out");
#pragma HLS STREAM variable=conv_br2c_out depth=16
#pragma HLS RESOURCE variable=conv_br2c_out core=FIFO_LUTRAM
		
	DuplicateStreams_Batch<BR2A_SIMD*BYP_INBITS, BR2A_IFMDIM*BR2A_IFMDIM*(BR2A_IFMC/BR2A_SIMD)>(in, thres_br2a_in, bypass_fifo, numReps);
	//split then quantize down if necessary       
        Thresholding_Batch <BR2A_IFMDIM, BR2A_IFMC, BR2A_SIMD, Slice<ap_uint<BYP_INBITS>>, Slice<ap_uint<BR2A_INBITS>>>(thres_br2a_in, conv_br2a_in, threshMemBYP, numReps);

	// -- BR2A: 1 x 1 Convolution
	// Number of output windows
	constexpr unsigned int OFMDim_2a = 1 + (BR2A_IFMDIM - BR2A_STRIDE) / BR2A_STRIDE + (((BR2A_IFMDIM - BR2A_STRIDE) % BR2A_STRIDE) > 0);

	// Feed everything to the MVAU
	unsigned const MatrixW_2a = BR2A_IFMC;
	unsigned const MatrixH_2a = BR2A_OFMC;
	Matrix_Vector_Activate_Stream_Batch<MatrixW_2a, MatrixH_2a, BR2A_SIMD, BR2A_PE, Slice<ap_uint<BR2A_INBITS>>, Slice<ap_uint<BR2A_ACTBITS>>, BR2A_WINTERPRET, ap_uint<BR2A_WBITS> >
			(static_cast<hls::stream<ap_uint<BR2A_SIMD * BR2A_INBITS>>&>(conv_br2a_in),
			static_cast<hls::stream<ap_uint<BR2A_PE * BR2A_ACTBITS>>&>  (conv_br2a_out),
			weightMem2A, threshMem2A, numReps * OFMDim_2a * OFMDim_2a, ap_resource_lut());

	StreamingDataWidthConverter_Batch<BR2A_PE * BR2A_ACTBITS, BR2B_IFMC * BR2B_INBITS, BR2A_OFMDIM*BR2A_OFMDIM*(BR2A_OFMC/BR2A_PE)>(conv_br2a_out, conv_br2b_in, numReps);


	// -- BR2B: 3 x 3 Convolution
	// Number of output windows
	constexpr unsigned int OFMDim = 1 + (BR2B_IFMDIM - BR2B_STRIDE) / BR2B_STRIDE + (((BR2B_IFMDIM - BR2B_STRIDE) % BR2B_STRIDE) > 0);
	// Output dimensions of the resize stage
	constexpr unsigned int intermediateDimension = 3 + BR2B_STRIDE * (OFMDim - 1);
	// Padding
	stream<ap_uint<BR2B_IFMC * BR2B_INBITS>> resizedInp("BR2B.resizedInput");
	SameResize_Batch<BR2B_IFMDIM, 3, BR2B_STRIDE, BR2B_IFMC, ap_uint<BR2B_INBITS>>(conv_br2b_in, resizedInp, numReps);

	// DWC from inp -> parallel in format
	unsigned const instreamW_RAW_2B = BR2B_IFMC * BR2B_INBITS;
	unsigned const instreamW_DWC_2B = BR2B_SIMD * BR2B_INBITS;
	unsigned const instreamWords_2B = intermediateDimension * intermediateDimension;
	WidthAdjustedInputStream<instreamW_RAW_2B, instreamW_DWC_2B, instreamWords_2B> dwc_2B_in(resizedInp, numReps);

	// DWC from parallel out format -> outp
	unsigned const outstreamW_RAW_2B = BR2B_PE * BR2B_ACTBITS;
	unsigned const outstreamW_DWC_2B = BR2B_OFMC * BR2B_ACTBITS;
	unsigned const outstreamWords_2B = OFMDim * OFMDim * (BR2B_OFMC / BR2B_PE);
	
	// Generate conv input stream (parallel in width)
	stream<ap_uint<BR2B_SIMD * BR2B_INBITS>> convInp("BR2B.convInp");
	ConvolutionInputGenerator<3, BR2B_IFMC, BR2B_INBITS, intermediateDimension, OFMDim, BR2B_SIMD, BR2B_STRIDE>(dwc_2B_in, convInp, numReps);

	// Feed everything to the MVAU
	unsigned const MatrixW = 3 * 3 * BR2B_IFMC;
	unsigned const MatrixH = BR2B_OFMC;
	stream<ap_uint<BR2B_PE * BR2B_ACTBITS>> dwc_2B_out("BR2B.convOut");
	Matrix_Vector_Activate_Stream_Batch<MatrixW, MatrixH, BR2B_SIMD, BR2B_PE, Slice<ap_uint<BR2B_INBITS>>, Slice<ap_uint<BR2B_ACTBITS>>, BR2B_WINTERPRET, ap_uint<BR2B_WBITS> >
			(static_cast<hls::stream<ap_uint<BR2B_SIMD * BR2B_INBITS>>&>(convInp),
			static_cast<hls::stream<ap_uint<BR2B_PE * BR2B_ACTBITS>>&>  (dwc_2B_out),
			weightMem2B, threshMem2B, numReps * OFMDim * OFMDim, ap_resource_lut());
	
	// TODO: Convert at once?
	StreamingDataWidthConverter_Batch<outstreamW_RAW_2B, outstreamW_DWC_2B, outstreamWords_2B>(dwc_2B_out, conv_br2b_out, numReps);
	StreamingDataWidthConverter_Batch<BR2B_OFMC * BR2B_ACTBITS, BR2C_SIMD * BR2C_INBITS , OFMDim*OFMDim>(conv_br2b_out, conv_br2c_in, numReps);


	// -- BR2C: 1 x 1 Convolution
	// Number of output windows
	constexpr unsigned int OFMDim_2c = 1 + (BR2C_IFMDIM - BR2C_STRIDE) / BR2C_STRIDE + (((BR2C_IFMDIM - BR2C_STRIDE) % BR2C_STRIDE) > 0);

	// Feed everything to the MVAU
	unsigned const MatrixW_2c = BR2C_IFMC;
	unsigned const MatrixH_2c = BR2C_OFMC;
	Matrix_Vector_Activate_Stream_Batch<MatrixW_2c, MatrixH_2c, BR2C_SIMD, BR2C_PE, Slice<ap_uint<BR2C_INBITS>>, Slice<ap_uint<BR2C_ACTBITS>>, BR2C_WINTERPRET, ap_uint<BR2C_WBITS> >
			(static_cast<hls::stream<ap_uint<BR2C_SIMD * BR2A_INBITS>>&>(conv_br2c_in),
			static_cast<hls::stream<ap_uint<BR2C_PE * BR2C_ACTBITS>>&>  (conv_br2c_out),
			weightMem2C, threshMem2C, numReps * OFMDim_2c * OFMDim_2c, ap_resource_lut());

	StreamingDataWidthConverter_Batch<BR2A_SIMD * BYP_INBITS, BR2C_PE * BR2C_ACTBITS, BR2A_IFMDIM*BR2A_IFMDIM*(BR2A_IFMC/BR2A_SIMD)>(bypass_fifo, conv_br1_out, numReps);
	AddStreams_Batch<BR2C_PE, ap_uint<BR2C_ACTBITS>, ap_uint<BR2C_ACTBITS>, ap_ufixed<BR2C_ACTBITS, BR2C_ACTBITS, AP_RND, AP_SAT_SYM>, BR2C_OFMDIM*BR2C_OFMDIM*(BR2C_OFMC/BR2C_PE), -8>(conv_br2c_out, conv_br1_out, out, numReps);
}
