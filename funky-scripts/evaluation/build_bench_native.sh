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
    cd $build_dir

    make host &> /dev/null || echo "building ${arg_apps[$i]} failed" &
  done

  for job in $(jobs -p); do
    wait "$job"
  done
}

build_benchmark /home/felix/Projects/vitis-accel-examples/ocl_kernels VITIS_EXAMPLES_APPS
build_benchmark /home/felix/Projects/vitis-accel-examples/ocl_kernels BENCHMARK_APPS
build_benchmark /home/felix/Projects/funky/funky-rosetta ROSETTA_APPS
