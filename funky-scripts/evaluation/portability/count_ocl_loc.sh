#!/usr/bin/env bash

set -euo pipefail

script_dir=$(dirname "$(readlink -f "$0")")
vitis_dir=$(realpath "$script_dir/../../../examples/Vitis_Accel_Examples/ocl_kernels")
vitis_apps=(cl_array_partition cl_burst_rw cl_dataflow_func cl_dataflow_subfunc cl_helloworld \
  cl_lmem_2rw cl_loop_reorder cl_partition_cyclicblock cl_shift_register cl_systolic_array \
  cl_gmem_2banks cl_wide_mem_rw cl_wide_mem_rw_strm cl_wide_mem_rw_2x cl_wide_mem_rw_4x)
vitis_common_dir=$(realpath "$script_dir/../../../examples/funky_utils")
vitis_common_files=("bitmap/bitmap.h" "bitmap/bitmap.cpp" "cmdparser/cmdlineparser.h" \
  "cmdparser/cmdlineparser.cpp" "logger/logger.h" "logger/logger.cpp" "xcl2/xcl2.hpp" \
  "xcl2/xcl2.cpp")

include_pattern='cl::|\.setArg|\.finish|\.getProfilingInfo|\.enqueueMigrateMemObjects' \
include_pattern+='|\..enqueueTask'
exclude_pattern='xcl::'

echo "app,ocl_loc"

# Individual apps
for app in "${vitis_apps[@]}"; do
  echo -n "$app,"
  grep -E "$include_pattern" "$vitis_dir/$app/host.cpp" | grep -Ev "$exclude_pattern" -c || true
done

# Sum up vitis common lib
sum=0
for file in "${vitis_common_files[@]}"; do
  loc=$(grep -E "$include_pattern" "$vitis_common_dir/$file" | grep -Ev "$exclude_pattern" -c || true)
  sum=$((sum + loc))
done
echo "vitis_common_lib,$sum"
