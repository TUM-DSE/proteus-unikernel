#!/usr/bin/env bash

set -u
 
vitis_bs_path=/share/felix/bitstreams/vitis-accel-examples
#rosetta_bs_path=/share/felix/bitstreams/rosetta-funky
fpga_types="u50-slow u50-fast u280-slow u280-fast u280-ddr-slow u280-ddr-fast"

for base_dir in "$vitis_bs_path"/*; do
  for fpga_type in $fpga_types; do
    bs_dir=$base_dir/$fpga_type
    if [ -d "$bs_dir" ]; then
      bs=$(find "$bs_dir" -name \*.xclbin -print | grep -v link)
      if [ -f "$bs" ]; then
        ln -sf "$(basename "$bs")" "$bs_dir"/bitstream
      fi
    fi
  done
done
