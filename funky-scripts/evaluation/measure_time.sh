#!/bin/bash
set -eo pipefail

source ./common.sh

function usage {
  cat <<EOF

Usage:
  $(basename ${0}) <repeat> <fpga>-<speed>...

  <fpga>  u50, u280, arria10
  <speed> slow, fast, ddr-slow, ddr-fast

  Measure the end-to-end execution time of workloads in Vitis_Accel_Examples/ocl_kernels and Rosetta/.
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
  RESULTS_DIR="${EVAL_SCRIPT_ROOT}/time_$DATE"
  mkdir -p ${RESULTS_DIR}
  APPLIST_CSV="${EVAL_SCRIPT_ROOT}/${arg_ifile}"
  RESULTS_CSV="$RESULTS_DIR/$fpga-$speed-$arg_ofile.csv"

  ### add label to csv
  echo -n "app_name," >> ${RESULTS_CSV}
  for loop in $(seq 1 ${arg_repeat}); do
    echo -n "${loop}," >> ${RESULTS_CSV}
  done
  echo "average,stddev,kernel_input_data_size,kernel_output_data_size,kernel_iterations,time_cpu,time_cpu_stddev,data_to_fpga_ocl,data_to_fpga_ocl_stddev,kernel_ocl,kernel_ocl_stddev,data_to_host_ocl,data_to_host_ocl_stddev" >> ${RESULTS_CSV}

  ### execution
  pushd ${arg_benchdir} > /dev/null

  python3 ${EVAL_SCRIPT_ROOT}/measure_time.py ${RESULTS_DIR} ${RESULTS_CSV} ${APPLIST_CSV} ${arg_repeat} ${bitstream_dir} ${fpga} ${speed} \
    ${UKVM_BIN} --mem=4096 --disk=${APP_BIN} --net=${TAP_IF} --fpga=${fpga} ${MON_OPT} ${LOAD_OPT} ${APP_BIN} ${APP_BIN}

  popd > /dev/null
}

# Increase memory memory limits for Intel FPGA emulation
export CL_CONFIG_CPU_FORCE_LOCAL_MEM_SIZE=8GB
export CL_CONFIG_CPU_FORCE_PRIVATE_MEM_SIZE=8GB

set_ukvm_permission # make ukvm-bin executable

repeat=$1
if [ -z "$repeat" ]; then
  usage
  exit 1
fi

for fpga in "${@:2}"; do
  model=${fpga%%-*}
  speed=${fpga#*-}
  if [ -z "$model" ] || [ -z "$speed" ]; then
    usage
    exit 1
  fi

  echo ["$(date +%T)"] "$fpga":
  measure_time "$VITIS_EXAMPLES_DIR" "$repeat" "vitis_applist_proteus.csv" "vitis" /share/felix/bitstreams/vitis-accel-examples "$model" "$speed"
  measure_time "$ROSETTA_DIR" "$repeat" "rosetta_applist_proteus.csv" "rosetta" /share/felix/bitstreams/rosetta "$model" "$speed"
done
