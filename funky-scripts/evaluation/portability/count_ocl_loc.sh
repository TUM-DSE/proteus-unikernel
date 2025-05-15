#!/usr/bin/env bash

# This script counts the number of OpenCL functions being called in
# Vitis Accel Examples and Rosetta including their common libraries.

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

rosetta_dir=$(realpath "$script_dir/../../../examples/Rosetta")

include_pattern='cl::|\.setArg|\.finish|\.getProfilingInfo|\.enqueueMigrateMemObjects'
include_pattern+='|\..enqueueTask|clFinish|clSetKernelArg|clCreateKernel|clReleaseKernel'
include_pattern+='|clCreateBuffer|clEnqueueWriteBuffer|clGetEventProfilingInfo|clEnqueueReadBuffer'
include_pattern+='clCreateProgramWithBinary|clEnqueueNDRangeKernel|clGetPlatformIDs|clGetDeviceIDs'
include_pattern+='clGetDeviceInfo|clCreateContext|clCreateCommandQueue|clReleaseMemObject'
include_pattern+='clReleaseProgram|clReleaseCommandQueue|clReleaseContext'
exclude_pattern='xcl::'

echo "app,ocl_loc"

# Individual vitis apps
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

# Rosetta
echo -n "3d-rendering,"
grep -E "$include_pattern" "$rosetta_dir"/3d-rendering/{*.h,*.cpp} | grep -Ev "$exclude_pattern" -c || true
echo -n "digit-recognition,"
grep -E "$include_pattern" "$rosetta_dir"/digit-recognition/{*.h,*.cpp} | grep -Ev "$exclude_pattern" -c || true
echo -n "optical-flow,"
grep -E "$include_pattern" "$rosetta_dir"/optical-flow/{*.h,*.cpp} | grep -Ev "$exclude_pattern" -c || true
echo -n "spam-filter,"
grep -E "$include_pattern" "$rosetta_dir"/spam-filter/{*.h,*.cpp} | grep -Ev "$exclude_pattern" -c || true
echo -n "rosetta_common_lib,"
grep -E "$include_pattern" "$rosetta_dir"/harness/{*.h,*.cpp} | grep -Ev "$exclude_pattern" -c || true
