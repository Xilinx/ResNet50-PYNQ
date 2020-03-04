//RES3A
#pragma HLS ARRAY_PARTITION variable=thres_FPGAThresholdLayer_top0 complete dim=1

#pragma HLS ARRAY_PARTITION variable=weights_FPGABipolarConvThresholdLayer_br20 complete dim=1
#pragma HLS ARRAY_PARTITION variable=thres_FPGABipolarConvThresholdLayer_br20 complete dim=1
#pragma HLS ARRAY_PARTITION variable=thres_FPGABipolarConvThresholdLayer_br20 complete dim=3

#pragma HLS ARRAY_PARTITION variable=weights_FPGABipolarConvThresholdLayer_br21 complete dim=1
#pragma HLS RESOURCE variable=weights_FPGABipolarConvThresholdLayer_br21 core=ROM_2P_BRAM
#pragma HLS ARRAY_PARTITION variable=thres_FPGABipolarConvThresholdLayer_br21 complete dim=1
#pragma HLS ARRAY_PARTITION variable=thres_FPGABipolarConvThresholdLayer_br21 complete dim=3

#pragma HLS ARRAY_PARTITION variable=weights_FPGABipolarConvThresholdLayer_br22 complete dim=1
#pragma HLS ARRAY_PARTITION variable=thres_FPGABipolarConvThresholdLayer_br22 complete dim=1
#pragma HLS ARRAY_PARTITION variable=thres_FPGABipolarConvThresholdLayer_br22 complete dim=3

#pragma HLS ARRAY_PARTITION variable=weights_FPGABipolarConvThresholdLayer_br10 complete dim=1
#pragma HLS ARRAY_PARTITION variable=thres_FPGABipolarConvThresholdLayer_br10 complete dim=1
#pragma HLS ARRAY_PARTITION variable=thres_FPGABipolarConvThresholdLayer_br10 complete dim=3

//Map together PE weight memories of RES3A 3x3 convolution (BRAM resource optimization)
//need to do this for each pair of consecutive PEs (16 in total); physical allocated RAM will be 72+72 deep, and 64 wide per PE (64 RAMB18)
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[0] instance=bram_shared_0 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[1] instance=bram_shared_0 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[2] instance=bram_shared_1 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[3] instance=bram_shared_1 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[4] instance=bram_shared_2 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[5] instance=bram_shared_2 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[6] instance=bram_shared_3 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[7] instance=bram_shared_3 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[8] instance=bram_shared_4 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[9] instance=bram_shared_4 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[10] instance=bram_shared_5 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[11] instance=bram_shared_5 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[12] instance=bram_shared_6 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[13] instance=bram_shared_6 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[14] instance=bram_shared_7 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[15] instance=bram_shared_7 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[16] instance=bram_shared_8 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[17] instance=bram_shared_8 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[18] instance=bram_shared_9 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[19] instance=bram_shared_9 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[20] instance=bram_shared_10 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[21] instance=bram_shared_10 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[22] instance=bram_shared_11 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[23] instance=bram_shared_11 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[24] instance=bram_shared_12 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[25] instance=bram_shared_12 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[26] instance=bram_shared_13 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[27] instance=bram_shared_13 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[28] instance=bram_shared_14 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[29] instance=bram_shared_14 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[30] instance=bram_shared_15 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[31] instance=bram_shared_15 horizontal
