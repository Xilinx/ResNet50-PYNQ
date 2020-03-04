#pragma once
#pragma HLS ARRAY_PARTITION variable=weights_conv0 complete dim=1
#pragma HLS RESOURCE variable=weights_conv0 core=ROM_1P_LUTRAM
#pragma HLS ARRAY_PARTITION variable=thres_conv0 complete dim=1
#pragma HLS RESOURCE variable=thres_conv0 core=ROM_1P_LUTRAM