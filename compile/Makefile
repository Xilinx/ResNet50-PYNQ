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

SHELL := /bin/bash
DMAS=inoutdma
DMA_IPS=$(addsuffix .zip,$(DMAS))
RESBLOCKS_BAKEDIN=res2a res2b res2c res3a res3b res3c res3d
RESBLOCKS_STREAMED=res4a res4b res4c res4d res4e res4f res5a res5b res5c
RESBLOCK_BAKEDIN_IPS=$(addsuffix .zip,$(RESBLOCKS_BAKEDIN))
RESBLOCK_STREAMED_IPS=$(addsuffix .zip,$(RESBLOCKS_STREAMED))
RESBLOCK_STREAMER_IPS=$(addsuffix _streamer.zip,$(RESBLOCKS_STREAMED))
MISC_LAYERS=preres postres
MISC_LAYER_IPS=$(addsuffix .zip,$(MISC_LAYERS))
DEFAULT_FREQ_MHZ=250
DEFAULT_PERIOD_NS=$(shell expr 1000 / $(DEFAULT_FREQ_MHZ))
DEVICE=xcu250-figd2104-2L-e
COMMAND=ip
NET=w1a2_v1.0

all: resnet50.xo

resnet50.xo: $(RESBLOCK_BAKEDIN_IPS) $(RESBLOCK_STREAMED_IPS) $(RESBLOCK_STREAMER_IPS) $(DMA_IPS) $(MISC_LAYER_IPS)
	vivado -mode batch -source ipi.tcl -tclargs $(DEFAULT_FREQ_MHZ) $(DEVICE)
	vivado -mode batch -source package_ip.tcl -tclargs resnet50 ip/ ip_repo/
	vivado -mode batch -source package_dcp.tcl -tclargs resnet50 ip/ ip_repo/ $(DEVICE)
	vivado -mode batch -source gen_xo.tcl -tclargs resnet50.xo resnet50 ip_repo/ kernel.xml

.PHONY=resblocks_bakedin
resblocks_bakedin: $(RESBLOCK_BAKEDIN_IPS)

.PHONY=resblocks_streamed
resblocks_streamed: $(RESBLOCK_STREAMED_IPS)

.PHONY=resblocks
resblocks: resblock_bakedin resblocks_streamed

$(RESBLOCK_BAKEDIN_IPS): %.zip: resblock.tcl ../src/resblock.cpp ../src/testbench/tb_resblock.cpp
	if [[ $* = res*a ]]; then echo "Recipe for 2-branch block $*"; else echo "Recipe for 1-branch block $*"; fi
	echo "Using bakedin weights for $*"
	rm -rf build_$*
	vivado_hls resblock.tcl -tclargs `if [[ $* = res*a ]]; then echo 2; else echo 1; fi` $* $(NET) $(COMMAND) $(DEVICE) RES_WMEM > build_$*.log
	cp build_$*/sol1/impl/ip/*.zip $@

$(RESBLOCK_STREAMED_IPS): %.zip: resblock.tcl ../src/resblock.cpp ../src/testbench/tb_resblock.cpp
	if [[ $* = res*a ]]; then echo "Recipe for 2-branch block $*"; else echo "Recipe for 1-branch block $*"; fi
	echo "Using streaming weights for $*"
	rm -rf build_$*
	vivado_hls resblock.tcl -tclargs `if [[ $* = res*a ]]; then echo 2; else echo 1; fi` $* $(NET) $(COMMAND) $(DEVICE) RES_WSTREAM > build_$*.log
	cp build_$*/sol1/impl/ip/*.zip $@

.PHONY=streamers
streamers: $(RESBLOCK_STREAMER_IPS)

$(RESBLOCK_STREAMER_IPS): %_streamer.zip: streamer.tcl ../src/streamer.cpp
	if [[ $* = res*a ]]; then echo "Recipe for 2-branch block streamer $*"; else echo "Recipe for 1-branch block streamer $*"; fi
	rm -rf build_$*_streamer
	vivado_hls streamer.tcl -tclargs `if [[ $* = res*a ]]; then echo 2; else echo 1; fi` $* $(NET) $(COMMAND) $(DEVICE) > build_$*_streamer.log
	cp build_$*_streamer/sol1/impl/ip/*.zip $@

inoutdma.zip: inoutdma.tcl ../src/inoutdma.cpp
	echo "Recipe for bidirectional DMA"
	rm -rf build_inoutdma
	vivado_hls inoutdma.tcl $(DEVICE) $(DEFAULT_PERIOD_NS) $(NET) $(COMMAND) > build_inoutdma.log
	cp build_inoutdma/sol1/impl/ip/*.zip $@

preres.zip: preres.tcl ../src/preres.cpp
	echo "Recipe for top layers"
	vivado_hls preres.tcl -tclargs $(DEVICE) $(NET) $(COMMAND) > build_preres.log
	cp build_preres/sol1/impl/ip/*.zip $@

postres.zip: postres.tcl ../src/postres.cpp
	echo "Recipe for bottom layers"
	vivado_hls postres.tcl -tclargs $(DEVICE) $(NET) $(COMMAND) > build_postres.log
	cp build_postres/sol1/impl/ip/*.zip $@

.PHONY=clean
clean:
	-rm -rf syn_* synth-res* *.log

.PHONY=distclean
distclean:
	git clean -xfd
