#!/bin/bash

source ./common.sh

build_benchmark() {
  arg_dir=$1
  arg_apps_array=$2[@]
  arg_apps=("${!arg_apps_array}")

  for i in ${!arg_apps[@]}; do
    echo "build ${arg_apps[$i]}..."
    cd ${arg_dir}/${arg_apps[$i]}

    make host &> /dev/null || echo "building ${arg_apps[$i]} failed" &
  done

  for job in $(jobs -p); do
    wait "$job"
  done
}

build_benchmark /home/felix/Projects/vitis-accel-examples/ocl_kernels VITIS_EXAMPLES_APPS
build_benchmark /home/felix/Projects/vitis-accel-examples/ocl_kernels BENCHMARK_APPS
#build_benchmark ${ROSETTA_DIR} ROSETTA_APPS
