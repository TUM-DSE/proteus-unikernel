#!/bin/bash

source ./common.sh

build_benchmark() {
  arg_dir=$1
  arg_apps_array=$2[@]
  arg_apps=("${!arg_apps_array}")

  for i in "${!arg_apps[@]}"; do
    build_dir=${arg_dir}/${arg_apps[$i]}
    # Not all apps for Proteus exist in native codebase
    if [ ! -d $build_dir ]; then
      echo "Skipping $build_dir (no such directory)"
      continue
    fi

    echo "build ${arg_apps[$i]}..."
    cd "$build_dir"

    make host &> /dev/null || echo "building ${arg_apps[$i]} failed" &
  done

  for job in $(jobs -p); do
    wait "$job"
  done
}

extra_vitis_apps=(cl_gmem_2banks_2x_pipelined cl_gmem_2banks_4x_pipelined cl_wide_mem_rw_2x_pipelined cl_wide_mem_rw_4x_pipelined cl_wide_mem_rw_strm_oversub)

build_benchmark "$PROTEUS_DIR"/vitis-accel-examples/ocl_kernels VITIS_EXAMPLES_APPS
build_benchmark "$PROTEUS_DIR"/vitis-accel-examples/ocl_kernels BENCHMARK_APPS
build_benchmark "$PROTEUS_DIR"/vitis-accel-examples/ocl_kernels extra_vitis_apps
build_benchmark "$PROTEUS_DIR"/funky-rosetta ROSETTA_APPS
