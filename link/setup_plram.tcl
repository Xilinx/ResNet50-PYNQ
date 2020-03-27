# Setup PLRAM 
sdx_memory_subsystem::update_plram_specification [get_bd_cells /memory_subsystem] PLRAM_MEM00 { SIZE 2M AXI_DATA_WIDTH 512 SLR_ASSIGNMENT SLR0 READ_LATENCY 6 MEMORY_PRIMITIVE URAM}
validate_bd_design -force
save_bd_design

