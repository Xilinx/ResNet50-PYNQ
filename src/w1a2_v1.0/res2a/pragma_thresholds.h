#pragma once
//RES2A
#pragma HLS ARRAY_PARTITION variable = thres_FPGAThresholdLayer_top complete dim = 1
#pragma HLS RESOURCE variable=thres_FPGAThresholdLayer_top core=ROM_1P_LUTRAM

#pragma HLS ARRAY_PARTITION variable=thres_FPGABipolarConvThresholdLayer_br2a.m_thresholds complete dim=1
#pragma HLS ARRAY_PARTITION variable=thres_FPGABipolarConvThresholdLayer_br2a.m_thresholds complete dim=3

#pragma HLS ARRAY_PARTITION variable=thres_FPGABipolarConvThresholdLayer_br2b.m_thresholds complete dim=1
#pragma HLS ARRAY_PARTITION variable=thres_FPGABipolarConvThresholdLayer_br2b.m_thresholds complete dim=3


#pragma HLS ARRAY_PARTITION variable=thres_FPGABipolarConvThresholdLayer_br2c.m_thresholds complete dim=1
#pragma HLS ARRAY_PARTITION variable=thres_FPGABipolarConvThresholdLayer_br2c.m_thresholds complete dim=3

#pragma HLS ARRAY_PARTITION variable=thres_FPGABipolarConvThresholdLayer_br1.m_thresholds complete dim=1
#pragma HLS ARRAY_PARTITION variable=thres_FPGABipolarConvThresholdLayer_br1.m_thresholds complete dim=3