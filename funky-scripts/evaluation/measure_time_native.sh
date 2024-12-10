#!/bin/bash
set -eo pipefail

source ./common.sh

function usage {
  cat <<EOF

Usage:
  $(basename ${0}) <repeat> <fpga> <speed>

  <fpga>  u50, u280, arria10
  <speed> slow, fast, ddr-slow, ddr-fast

  Measure the end-to-end execution time of native Vitis_Accel_Examples/ocl_kernels and Rosetta/.
  clock_gettime() with MONOTONIC timer is used for the measurement.

EOF
}

measure_time() {
  arg_benchdir=$1
  arg_repeat=$2
  arg_ifile=$3
  arg_ofile=$4
  bitstream_dir=$5
  fpga=$6
  speed=$7

  ### create dir where the results are saved
  RESULTS_DIR="${EVAL_SCRIPT_ROOT}/time_native_$DATE"
  mkdir -p ${RESULTS_DIR}
  APPLIST_CSV="${EVAL_SCRIPT_ROOT}/${arg_ifile}"
  RESULTS_CSV="$RESULTS_DIR/$fpga-$speed-$arg_ofile.csv"
  KERNEL_RESULTS_CSV="$RESULTS_DIR/$fpga-$speed-$arg_ofile-kernel.csv"
  KERNEL_RESULTS_HEADER="app_name,kernel_input_data_size,iterations,data_to_fpga_avg_time,kernel_avg_time,data_to_host_avg_time"

  ### add label to csvd
  echo -n "app_name," >> ${RESULTS_CSV}
  for loop in $(seq 1 ${arg_repeat}); do
    echo -n "${loop}," >> ${RESULTS_CSV}
  done
  echo "average,stdev" >> ${RESULTS_CSV}

  echo "$KERNEL_RESULTS_HEADER" >> "$KERNEL_RESULTS_CSV"

  ### execution
  echo "move into ${arg_benchdir}..."
  pushd ${arg_benchdir}

  python3 ${EVAL_SCRIPT_ROOT}/measure_time_native.py ${RESULTS_DIR} ${RESULTS_CSV} ${APPLIST_CSV} ${arg_repeat} ${bitstream_dir} ${fpga} ${speed}

  popd
  echo "back to $(pwd)."

  #TODO: Rosetta

  for log_file in "$RESULTS_DIR"/cl_*; do
    # Each application prints the header followed by the data in the next line
    grep -A 1 "$KERNEL_RESULTS_HEADER" "$log_file" \
      | head -n 2 | tail -n 1 >> "$KERNEL_RESULTS_CSV" \
      || echo "Failed to find kernel performance data in $log_file"
  done
}

set_ukvm_permission # make ukvm-bin executable

REPEAT=$1
FPGA=$2
SPEED=$3

if [ -z "$REPEAT" ] || [ -z "$FPGA" ] || [ -z "$SPEED" ]; then
  usage
  exit 1
fi

DIR="time_$DATE"
mkdir -p ${EVAL_SCRIPT_ROOT}/$DIR

measure_time /home/felix/Projects/vitis-accel-examples/ocl_kernels ${REPEAT} "vitis_applist.csv" "vitis" /share/felix/bitstreams/vitis-accel-examples ${FPGA} ${SPEED}
#measure_time ${ROSETTA_DIR} ${REPEAT} "rosetta_applist.csv" "rosetta" /share/felix/bitstreams/rosetta-funky ${FPGA} ${SPEED}

