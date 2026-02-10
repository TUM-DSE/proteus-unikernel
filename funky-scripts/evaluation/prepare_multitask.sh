#!/usr/bin/env bash

set -euo pipefail

script_dir=$(dirname "$(readlink -f "$0")")
sched_src_dir=$script_dir/../../../funky-monitor/scheduler
sched_bin_dir=$script_dir/sched_bins
branches="single_u50 single_u280 2_u50 2_u280 2_u280_second_instance 4_u50 4_u280 4_u280_second_instance"

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
