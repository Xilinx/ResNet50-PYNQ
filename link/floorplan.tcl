#  Copyright (c) 2019, Xilinx
#  All rights reserved.
#  
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#  
#  1. Redistributions of source code must retain the above copyright notice, this
#     list of conditions and the following disclaimer.
#  
#  2. Redistributions in binary form must reproduce the above copyright notice,
#     this list of conditions and the following disclaimer in the documentation
#     and/or other materials provided with the distribution.
#  
#  3. Neither the name of the copyright holder nor the names of its
#     contributors may be used to endorse or promote products derived from
#     this software without specific prior written permission.
#  
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
#  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
#  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
#  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
#  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
#  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#contents of this script are run after opt_design and before placement
add_cells_to_pblock pblock_dynamic_SLR0 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/inoutdma_0]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR0 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/fifo_inoutdma_dwc]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR0 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/dwc_inoutdma_preres]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR0 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/fifo_dwc_preres]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR0 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/preres_0]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR0 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/fifo_preres_dwc]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR0 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/dwc_preres_res2a]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR0 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/fifo_dwc_res2a]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR1 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/res2a_0]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR1 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/fifo_res2a_res2b]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR2 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/res2b_0]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR2 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/fifo_res2b_res2c]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR3 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/res2c_0]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR3 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/fifo_res2c_res3a]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR3 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/res3a_0]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR3 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/fifo_res3a_res3b]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR2 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/res3b_0]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR2 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/fifo_res3b_res3c]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR1 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/res3c_0]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR1 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/fifo_res3c_res3d]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR0 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/res3d_0]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR0 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/fifo_res3d_res4a]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR1 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/res4a_0]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR1 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/fifo_res4a_res4b]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR1 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/res4b_0]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR1 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/fifo_res4b_res4c]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR2 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/res4c_0]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR2 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/fifo_res4c_res4d]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR2 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/res4d_0]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR2 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/fifo_res4d_res4e]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR3 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/res4e_0]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR3 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/fifo_res4e_res4f]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR3 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/res4f_0]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR3 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/fifo_res4f_res5a]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR3 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/res5a_0]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR3 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/fifo_res5a_res5b]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR2 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/res5b_0]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR2 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/fifo_res5b_res5c]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR1 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/res5c_0]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR1 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/fifo_res5c_postres]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR0 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/postres_0]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR0 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/fifo_postres_inoutdma]] -clear_locs

add_cells_to_pblock pblock_dynamic_SLR0 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/rst0_pipe_slr0]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR0 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/rst0_buf_slr0]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR1 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/rst0_pipe_slr1]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR1 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/rst0_buf_slr1]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR2 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/rst0_pipe_slr2]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR2 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/rst0_buf_slr2]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR3 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/rst0_pipe_slr3]] -clear_locs
add_cells_to_pblock pblock_dynamic_SLR3 [get_cells [list pfm_top_i/dynamic_region/resnet50_1/resnet50_i/rst0_buf_slr3]] -clear_locs
