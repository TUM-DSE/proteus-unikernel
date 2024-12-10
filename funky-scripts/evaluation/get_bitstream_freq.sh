#!/usr/bin/env bash
set -eo pipefail
source ./common.sh

# Format of kernel frequency information that xclbinutil outputs:
# Name:           ulp_ucs_aclk_kernel_00
# Type:           SCALABLE
# Default Freq:   300 MHz
# Requested Freq: 650 MHz
# Achieved Freq:  345 MHz

vitis_bs_path=/share/felix/bitstreams/vitis-accel-examples
rosetta_bs_path=/share/felix/bitstreams/rosetta-funky
fpga_types="u50-slow u50-fast u280-slow u280-fast u280-ddr-slow u280-ddr-fast"
header="app_name,u50_slow_freq,u50_fast_freq,u280_slow_freq,u280_fast_freq,u280_ddr_slow_freq,u280_ddr_fast_freq"

get_bitstream_freq() {
  app_list=("$@")

  for app in "${app_list[@]}"; do
    echo -n "$app"
    for fpga_type in $fpga_types; do
      bs=$vitis_bs_path/$app/$fpga_type/bitstream
      if [ -f "$bs" ]; then
        output="$(xclbinutil --info --input "$vitis_bs_path"/"$app"/"$fpga_type"/bitstream \
          | grep -A 4 ulp_ucs_aclk_kernel_00 | grep "Achieved Freq" | cut -d ":" -f 2 | cut -d " " -f 3)"
      else
        output=""
      fi
      echo -n ,"$output"
    done
    echo
  done
}

echo $header

get_bitstream_freq "${VITIS_EXAMPLES_APPS[@]}"
get_bitstream_freq "${ROSETTA_APPS[@]}"
