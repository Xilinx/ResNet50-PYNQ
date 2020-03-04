//RES4E
#pragma HLS ARRAY_PARTITION variable=thres_FPGAThresholdLayer_br20 complete dim=1

#pragma HLS ARRAY_PARTITION variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights complete dim=1
#pragma HLS RESOURCE variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights core=ROM_2P_BRAM
#pragma HLS ARRAY_PARTITION variable=thres_FPGABipolarConvThresholdLayer_br21 complete dim=1
#pragma HLS ARRAY_PARTITION variable=thres_FPGABipolarConvThresholdLayer_br21 complete dim=3

#pragma HLS ARRAY_PARTITION variable=weights_FPGABipolarConvThresholdLayer_br22.m_weights complete dim=1
#pragma HLS RESOURCE variable=weights_FPGABipolarConvThresholdLayer_br22.m_weights core=ROM_2P_BRAM
#pragma HLS ARRAY_PARTITION variable=thres_FPGABipolarConvThresholdLayer_br22 complete dim=1
#pragma HLS ARRAY_PARTITION variable=thres_FPGABipolarConvThresholdLayer_br22 complete dim=3

#pragma HLS ARRAY_PARTITION variable=weights_FPGABipolarConvThresholdLayer_br23.m_weights complete dim=1
#pragma HLS RESOURCE variable=weights_FPGABipolarConvThresholdLayer_br23.m_weights core=ROM_2P_BRAM
#pragma HLS ARRAY_PARTITION variable=thres_FPGABipolarConvThresholdLayer_br23 complete dim=1
#pragma HLS ARRAY_PARTITION variable=thres_FPGABipolarConvThresholdLayer_br23 complete dim=3

//Map together PE weight memories of RES4E_2B 1x1 convolution (BRAM resource optimization)
//need to do this for each pair of consecutive PEs (8 in total); physical allocated RAM will be 256+256 deep, and 32 wide per PE (16 RAMB18)
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[0] instance=bram_shared_2a_0 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[1] instance=bram_shared_2a_0 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[2] instance=bram_shared_2a_1 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[3] instance=bram_shared_2a_1 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[4] instance=bram_shared_2a_2 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[5] instance=bram_shared_2a_2 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[6] instance=bram_shared_2a_3 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[7] instance=bram_shared_2a_3 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[8] instance=bram_shared_2a_4 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[9] instance=bram_shared_2a_4 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[10] instance=bram_shared_2a_5 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[11] instance=bram_shared_2a_5 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[12] instance=bram_shared_2a_6 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[13] instance=bram_shared_2a_6 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[14] instance=bram_shared_2a_7 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[15] instance=bram_shared_2a_7 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[16] instance=bram_shared_2a_8 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[17] instance=bram_shared_2a_8 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[18] instance=bram_shared_2a_9 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[19] instance=bram_shared_2a_9 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[20] instance=bram_shared_2a_10 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[21] instance=bram_shared_2a_10 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[22] instance=bram_shared_2a_11 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[23] instance=bram_shared_2a_11 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[24] instance=bram_shared_2a_12 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[25] instance=bram_shared_2a_12 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[26] instance=bram_shared_2a_13 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[27] instance=bram_shared_2a_13 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[28] instance=bram_shared_2a_14 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[29] instance=bram_shared_2a_14 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[30] instance=bram_shared_2a_15 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br21.m_weights[31] instance=bram_shared_2a_15 horizontal
//Map together PE weight memories of RES4E 3x3 convolution (BRAM resource optimization)
//need to do this for each pair of consecutive PEs (16 in total); physical allocated RAM will be 288+288 deep, and 64 wide per PE (64 RAMB18)
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br22.m_weights[0] instance=bram_shared_2b_0 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br22.m_weights[1] instance=bram_shared_2b_0 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br22.m_weights[2] instance=bram_shared_2b_1 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br22.m_weights[3] instance=bram_shared_2b_1 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br22.m_weights[4] instance=bram_shared_2b_2 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br22.m_weights[5] instance=bram_shared_2b_2 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br22.m_weights[6] instance=bram_shared_2b_3 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br22.m_weights[7] instance=bram_shared_2b_3 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br22.m_weights[8] instance=bram_shared_2b_4 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br22.m_weights[9] instance=bram_shared_2b_4 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br22.m_weights[10] instance=bram_shared_2b_5 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br22.m_weights[11] instance=bram_shared_2b_5 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br22.m_weights[12] instance=bram_shared_2b_6 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br22.m_weights[13] instance=bram_shared_2b_6 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br22.m_weights[14] instance=bram_shared_2b_7 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br22.m_weights[15] instance=bram_shared_2b_7 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br22.m_weights[16] instance=bram_shared_2b_8 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br22.m_weights[17] instance=bram_shared_2b_8 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br22.m_weights[18] instance=bram_shared_2b_9 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br22.m_weights[19] instance=bram_shared_2b_9 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br22.m_weights[20] instance=bram_shared_2b_10 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br22.m_weights[21] instance=bram_shared_2b_10 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br22.m_weights[22] instance=bram_shared_2b_11 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br22.m_weights[23] instance=bram_shared_2b_11 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br22.m_weights[24] instance=bram_shared_2b_12 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br22.m_weights[25] instance=bram_shared_2b_12 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br22.m_weights[26] instance=bram_shared_2b_13 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br22.m_weights[27] instance=bram_shared_2b_13 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br22.m_weights[28] instance=bram_shared_2b_14 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br22.m_weights[29] instance=bram_shared_2b_14 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br22.m_weights[30] instance=bram_shared_2b_15 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br22.m_weights[31] instance=bram_shared_2b_15 horizontal
//Map together PE weight memories of RES4E_2C 1x1 convolution (BRAM resource optimization)
//need to do this for each pair of consecutive PEs (16 in total); physical allocated RAM will be 256+256 deep, and 32 wide per PE (64 RAMB18)
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br23.m_weights[0] instance=bram_shared_2c_0 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br23.m_weights[1] instance=bram_shared_2c_0 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br23.m_weights[2] instance=bram_shared_2c_1 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br23.m_weights[3] instance=bram_shared_2c_1 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br23.m_weights[4] instance=bram_shared_2c_2 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br23.m_weights[5] instance=bram_shared_2c_2 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br23.m_weights[6] instance=bram_shared_2c_3 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br23.m_weights[7] instance=bram_shared_2c_3 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br23.m_weights[8] instance=bram_shared_2c_4 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br23.m_weights[9] instance=bram_shared_2c_4 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br23.m_weights[10] instance=bram_shared_2c_5 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br23.m_weights[11] instance=bram_shared_2c_5 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br23.m_weights[12] instance=bram_shared_2c_6 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br23.m_weights[13] instance=bram_shared_2c_6 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br23.m_weights[14] instance=bram_shared_2c_7 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br23.m_weights[15] instance=bram_shared_2c_7 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br23.m_weights[16] instance=bram_shared_2c_8 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br23.m_weights[17] instance=bram_shared_2c_8 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br23.m_weights[18] instance=bram_shared_2c_9 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br23.m_weights[19] instance=bram_shared_2c_9 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br23.m_weights[20] instance=bram_shared_2c_10 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br23.m_weights[21] instance=bram_shared_2c_10 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br23.m_weights[22] instance=bram_shared_2c_11 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br23.m_weights[23] instance=bram_shared_2c_11 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br23.m_weights[24] instance=bram_shared_2c_12 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br23.m_weights[25] instance=bram_shared_2c_12 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br23.m_weights[26] instance=bram_shared_2c_13 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br23.m_weights[27] instance=bram_shared_2c_13 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br23.m_weights[28] instance=bram_shared_2c_14 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br23.m_weights[29] instance=bram_shared_2c_14 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br23.m_weights[30] instance=bram_shared_2c_15 horizontal
#pragma HLS ARRAY_MAP variable=weights_FPGABipolarConvThresholdLayer_br23.m_weights[31] instance=bram_shared_2c_15 horizontal
