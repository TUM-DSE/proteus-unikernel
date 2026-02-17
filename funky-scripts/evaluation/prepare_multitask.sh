#!/usr/bin/env bash

set -euo pipefail

script_dir=$(dirname "$(readlink -f "$0")")
mon_src_dir=$script_dir/../../../funky-monitor
sched_src_dir=$mon_src_dir/scheduler
sched_bin_dir=$script_dir/sched_bins
branches="1_fpga_u50 1_fpga_u280 2_fpga_50 2_fpga_280 4_fpga"

pushd "$sched_src_dir"

# Build scheduler binaries
for branch in $branches; do
  mkdir -p "$sched_bin_dir/$branch"
  git switch "$branch"
  make
  cp primary daemon deploy_script.sh "$sched_bin_dir/$branch"
done

cp daemon_u50 "$sched_bin_dir/4_fpga"
git switch proteus
make clean

# Build patched ukvm-bin
cd "$mon_src_dir"
git switch 4_fpga
cp scheduler/ukvm.patch /tmp
git switch proteus
git apply /tmp/ukvm.patch
export CC=gcc
export CXX=g++
make -j "$(nproc)"
cp ukvm/ukvm-bin "$sched_bin_dir"/ukvm-bin-patched
make clean
git restore ukvm/
rm /tmp/ukvm.patch

popd

# Make sure the bitstreams expected by the scheduler are in the right places in /tmp
if [ ! -f /tmp/rosetta_50 ]; then
  cp "$BITSTREAM_DIR/rosetta/spam-filter/u50-fast/SgdLR.hw.xclbin" /tmp/rosetta_50
fi
if [ ! -f /tmp/vitis_50 ]; then
  cp "$BITSTREAM_DIR/vitis-accel-examples/cl_helloworld/u50-fast/vector_addition.xclbin" /tmp/vitis_50
fi
if [ ! -f /tmp/rosetta_280 ]; then
  cp "$BITSTREAM_DIR/rosetta/spam-filter/u280-fast/SgdLR.hw.xclbin" /tmp/rosetta_280
fi
if [ ! -f /tmp/vitis_280 ]; then
  cp "$BITSTREAM_DIR/vitis-accel-examples/cl_helloworld/u280-fast/vector_addition.xclbin" /tmp/vitis_280
fi
