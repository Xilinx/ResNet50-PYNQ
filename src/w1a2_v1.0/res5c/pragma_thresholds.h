//RES5C
#pragma HLS ARRAY_PARTITION variable=thres_FPGAThresholdLayer_top complete dim=1



#pragma HLS ARRAY_PARTITION variable=thres_FPGABipolarConvThresholdLayer_br2a complete dim=1
#pragma HLS ARRAY_PARTITION variable=thres_FPGABipolarConvThresholdLayer_br2a complete dim=3



#pragma HLS ARRAY_PARTITION variable=thres_FPGABipolarConvThresholdLayer_br2b complete dim=1
#pragma HLS ARRAY_PARTITION variable=thres_FPGABipolarConvThresholdLayer_br2b complete dim=3



#pragma HLS ARRAY_PARTITION variable=thres_FPGABipolarConvThresholdLayer_br2c complete dim=1
#pragma HLS ARRAY_PARTITION variable=thres_FPGABipolarConvThresholdLayer_br2c complete dim=3


//need to do this for each pair of consecutive PEs (16 in total); physical allocated RAM will be 1152+1152 deep, and 64 wide per PE (192 RAMB18)































