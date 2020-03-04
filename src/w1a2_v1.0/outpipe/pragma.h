#pragma once
#pragma HLS ARRAY_PARTITION variable=bias_fc1000 complete dim=1
#pragma HLS RESOURCE variable=bias_fc1000 core=ROM_1P_BRAM