#!/usr/bin/env bash

set -euo pipefail

script_dir=$(dirname "$(readlink -f "$0")")
sched_src_dir=$script_dir/../../../funky-monitor/scheduler
sched_bin_dir=$script_dir/sched_bins
branches="1_fpga_u50 1_fpga_u280 2_fpga_50 2_fpga_280 4_fpga"

pushd "$sched_src_dir"

for branch in $branches; do
  mkdir -p "$sched_bin_dir/$branch"
  git switch "$branch"
  make
  cp primary daemon "$sched_bin_dir/$branch"
done

git switch proteus
make clean
popd
