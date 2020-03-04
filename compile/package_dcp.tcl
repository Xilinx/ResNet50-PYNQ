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

if { $::argc != 4 } {
    puts "ERROR: Program \"$::argv0\" requires 3 arguments!\n"
    puts "Usage: $::argv0 <kernel_name> <in_path_to_hdl> <out_path_to_ip> <part>\n"
    exit
}

set krnl_name   [lindex $::argv 0]
set path_to_hdl [lindex $::argv 1]
set path_to_ip  [lindex $::argv 2]
set proj_part   [lindex $::argv 3]

set path_to_tmp $path_to_hdl/pkg_tmp

#copy DCP and XDC files to their proper places in the IP folder structure
set dcpdir [file join [pwd] $path_to_ip "dcp"]
if {[file exists $dcpdir]} {
    file delete -force $dcpdir
}
file mkdir $dcpdir
file copy -force $path_to_hdl/$krnl_name.dcp $dcpdir 

set impldir [file join [pwd] $path_to_ip "impl"]
if {[file exists $impldir]} {
    file delete -force $impldir
}
file mkdir $impldir
file copy -force $path_to_hdl/$krnl_name.xdc $impldir

#create packaging project
ipx::open_ipxact_file $path_to_ip/component.xml

set family [get_property FAMILY [get_parts $proj_part]]
set families "$family Production"
set_property supported_families $families [ipx::current_core]
ipx::save_core [ipx::current_core]

ipx::add_file_group xilinx_synthesischeckpoint [ipx::current_core]
ipx::add_file $dcpdir/$krnl_name.dcp [ipx::get_file_groups xilinx_synthesischeckpoint -of_objects [ipx::current_core]]
set_property type dcp [ipx::get_files dcp/$krnl_name.dcp -of_objects [ipx::get_file_groups xilinx_synthesischeckpoint -of_objects [ipx::current_core]]]

ipx::add_file_group xilinx_simulationcheckpoint [ipx::current_core]
ipx::add_file $dcpdir/$krnl_name.dcp [ipx::get_file_groups xilinx_simulationcheckpoint -of_objects [ipx::current_core]]
set_property type dcp [ipx::get_files dcp/$krnl_name.dcp -of_objects [ipx::get_file_groups xilinx_simulationcheckpoint -of_objects [ipx::current_core]]]
ipx::save_core [ipx::current_core]

ipx::add_file_group xilinx_implementation [ipx::current_core]
ipx::add_file $impldir/$krnl_name.xdc [ipx::get_file_groups xilinx_implementation -of_objects [ipx::current_core]]
set_property type xdc [ipx::get_files impl/$krnl_name.xdc -of_objects [ipx::get_file_groups xilinx_implementation -of_objects [ipx::current_core]]]
set_property used_in [list "implementation"] [ipx::get_files impl/$krnl_name.xdc -of_objects [ipx::get_file_groups xilinx_implementation -of_objects [ipx::current_core]]]
ipx::save_core [ipx::current_core]

ipx::remove_all_file [ipx::get_file_groups xilinx_anylanguagebehavioralsimulation]
ipx::remove_file_group xilinx_anylanguagebehavioralsimulation [ipx::current_core]

ipx::remove_all_file [ipx::get_file_groups xilinx_anylanguagesynthesis]
ipx::remove_file_group xilinx_anylanguagesynthesis [ipx::current_core]

set_property sdx_kernel true [ipx::current_core]
set_property sdx_kernel_type rtl [ipx::current_core]
ipx::save_core [ipx::current_core]

set corever [get_property core_revision [ipx::current_core]]
puts "core version = $corever"
set new_ver [expr $corever + 1]
puts "new version = $new_ver"
set_property core_revision $new_ver [ipx::current_core]
ipx::create_xgui_files [ipx::current_core]
ipx::update_checksums [ipx::current_core]
ipx::save_core [ipx::current_core]
ipx::check_integrity [ipx::current_core]

set hdldir [file join [pwd] $path_to_ip "src"]
file delete -force $hdldir

close_project


