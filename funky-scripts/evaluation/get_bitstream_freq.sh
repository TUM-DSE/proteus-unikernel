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
fpga_types="u50-300mhz u50-faster u280-300mhz u280-faster u280-ddr-300mhz u280-ddr-faster"
header="app_name,u50_slow_freq,u50_fast_freq,u280_slow_freq,u280_fast_freq,u280_ddr_slow_freq,u280_ddr_fast_freq"

echo $header

for app in "${VITIS_EXAMPLES_APPS[@]}"; do
  echo -n "$app"
  for fpga_type in $fpga_types; do
    output="$(xclbinutil --info --input "$vitis_bs_path"/"$app"/"$fpga_type"/bitstream \
      | grep -A 4 ulp_ucs_aclk_kernel_00 | grep "Achieved Freq" | cut -d ":" -f 2 | cut -d " " -f 3)"
    echo -n ,"$output"
  done
  echo
done

for app in "${ROSETTA_APPS[@]}"; do
  echo -n "$app"
  for fpga_type in $fpga_types; do
    output="$(xclbinutil --info --input "$rosetta_bs_path"/"$app"/"$fpga_type"/bitstream \
      | grep -A 4 ulp_ucs_aclk_kernel_00 | grep "Achieved Freq" | cut -d ":" -f 2 | cut -d " " -f 3)"
    echo -n ,"$output"
  done
  echo
done
