#!/bin/bash
set -eo pipefail

source ./common.sh

function usage {
  cat <<EOF

Usage: 
  $(basename ${0}) <repeat> <fpga> <speed>

  <fpga>  u50, u280, arria10
  <speed> 300mhz, faster

  Measure the end-to-end execution time of workloads in Vitis_Accel_Examples/ocl_kernels and Rosetta/.
  clock_gettime() with MONOTONIC timer is used for the measurement. 

EOF
}

measure_time() {
  arg_benchdir=$1
  # arg_apps_array=$2[@]
  # arg_apps=("${!arg_apps_array}")
  # arg_args_array=$3[@]
  # arg_args=("${!arg_args_array}")
  arg_repeat=$2
  arg_ifile=$3
  arg_ofile=$4
  bitstream_dir=$5
  fpga=$6
  speed=$7

  ### create dir where the results are saved
  RESULTS_DIR="${EVAL_SCRIPT_ROOT}/time_$DATE"
  mkdir -p ${RESULTS_DIR}
  APPLIST_CSV="${EVAL_SCRIPT_ROOT}/${arg_ifile}"
  RESULTS_CSV="${RESULTS_DIR}/${arg_ofile}"

  ### add label to csv 
  echo -n "app_name, " >> ${RESULTS_CSV}
  for loop in $(seq 1 ${arg_repeat}); do
    echo -n "${loop}, " >> ${RESULTS_CSV}
  done
  echo "average, stdev, " >> ${RESULTS_CSV}

  ### execution
  echo "move into ${arg_benchdir}..."
  pushd ${arg_benchdir}

  # python3 ${EVAL_SCRIPT_ROOT}/measure_time.py ${RESULTS_DIR} ${RESULTS_CSV} ${APPLIST_CSV} ${arg_repeat} ${UKVM_EXEC_CMD}
  measure_ukvm_exec_time_py ${RESULTS_DIR} ${RESULTS_CSV} ${APPLIST_CSV} ${arg_repeat} ${bitstream_dir} ${fpga} ${speed} \
    "${UKVM_BIN} --mem=1024 --disk=${APP_BIN} --net=${TAP_IF} --fpga=${fpga} ${MON_OPT} ${LOAD_OPT} ${APP_BIN} ${APP_BIN}"

  popd
  echo "back to $(pwd)."
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

measure_time ${VITIS_EXAMPLES_DIR} ${REPEAT} "vitis_applist.csv" "vitis.csv" /share/felix/bitstreams/vitis-accel-examples ${FPGA} ${SPEED}
#measure_time ${ROSETTA_DIR} ${REPEAT} "rosetta_applist.csv" "rosetta.csv" /share/felix/bitstreams/vitis-accel-examples ${FPGA} ${SPEED}

