#pragma once
//RES2A
#pragma HLS ARRAY_PARTITION variable = thres_FPGAThresholdLayer_top0 complete dim = 1
#pragma HLS RESOURCE variable=thres_FPGAThresholdLayer_top0 core=ROM_1P_LUTRAM
#pragma HLS ARRAY_PARTITION variable=weights_FPGABipolarConvThresholdLayer_br20.m_weights complete dim=1
#pragma HLS ARRAY_PARTITION variable=thres_FPGABipolarConvThresholdLayer_br20.m_thresholds complete dim=1
#pragma HLS ARRAY_PARTITION variable=thres_FPGABipolarConvThresholdLayer_br20.m_thresholds complete dim=3
#pragma HLS ARRAY_PARTITION variable = weights_FPGABipolarConvThresholdLayer_br21.m_weights complete dim = 1
#pragma HLS ARRAY_PARTITION variable=thres_FPGABipolarConvThresholdLayer_br21.m_thresholds complete dim=1
#pragma HLS ARRAY_PARTITION variable=thres_FPGABipolarConvThresholdLayer_br21.m_thresholds complete dim=3
#pragma HLS ARRAY_PARTITION variable=weights_FPGABipolarConvThresholdLayer_22 complete dim=1
#pragma HLS RESOURCE variable=weights_FPGABipolarConvThresholdLayer_br22.m_weights core=ROM_1P_LUTRAM
#pragma HLS ARRAY_PARTITION variable=thres_FPGABipolarConvThresholdLayer_br22.m_thresholds complete dim=1
#pragma HLS ARRAY_PARTITION variable=thres_FPGABipolarConvThresholdLayer_br22.m_thresholds complete dim=3
#pragma HLS ARRAY_PARTITION variable=weights_FPGABipolarConvThresholdLayer_br10.m_weights complete dim=1
#pragma HLS ARRAY_PARTITION variable=thres_FPGABipolarConvThresholdLayer_br10.m_thresholds complete dim=1
#pragma HLS ARRAY_PARTITION variable=thres_FPGABipolarConvThresholdLayer_br10.m_thresholds complete dim=3