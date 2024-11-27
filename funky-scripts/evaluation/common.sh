DATE=`date "+%m%d%Y_%H%M%S"`

##### directory paths
EVAL_SCRIPT_ROOT=$(readlink -f $(dirname ${BASH_SOURCE:-$0}))
VITIS_EXAMPLES_DIR=${EVAL_SCRIPT_ROOT}/../../examples/Vitis_Accel_Examples/ocl_kernels
ROSETTA_DIR=${EVAL_SCRIPT_ROOT}/../../examples/Rosetta

##### params for unikernel execution
UKVM_BIN=${INCLUDEOS_PREFIX}/includeos/x86_64/lib/ukvm-bin
TAP_IF=tap100
SOCKET_IF=/tmp/solo5_socket
MIG_FILE=mig_file
APP_BIN=build/funkycl-app
UKVM_EXEC_CMD="${UKVM_BIN} --mem=1024 --disk=${APP_BIN} --net=${TAP_IF} ${MON_OPT} ${LOAD_OPT} ${APP_BIN} ${APP_BIN}"

##### Vitis_Accel_Examples
VITIS_EXAMPLES_APPS=(cl_array_partition cl_burst_rw cl_dataflow_func cl_dataflow_subfunc cl_gmem_2banks cl_helloworld cl_lmem_2rw cl_loop_reorder cl_partition_cyclicblock cl_shift_register cl_systolic_array cl_wide_mem_rw)
# We pass dummy args instead of the bitstream because the monitor handles bitstreams
VITIS_EXAMPLES_ARGS=(a a a a a a a a a a a a)

##### Rosetta
ROSETTA_APPS=(3d-rendering digit-recognition optical-flow spam-filter)
ROSETTA_BINS=(rendering_host.exe DigitRec_host.exe optical_flow_host.exe SgdLR_host.exe)
ROSETTA_ARGS=("-f rendering.xclbin" "-f DigitRec.hw.xclbin" "-f optical_flow.hw.xclbin -p sintel_alley -o outputs.flo" "-f SgdLR.hw.xclbin -p data/")
ROSETTA_CODE=("3d_rendering_host.cpp" "digit_recognition.cpp" "optical_flow_host.cpp" "spam_filter.cpp")

find_binary() {
  build_dir=$1

  if [ ! -e ${build_dir} ]; then
    echo "Error: ${build_dir} doesn't exist. Please specify the correct build directory or compile an application first."
    exit -1
  fi

  # search build_dir for unikernel app binary
  APP_BIN=$(find ${build_dir} -maxdepth 1 -executable -type f)

  if [ ! -e ${APP_BIN} ]; then
    echo "Error: binary ${APP_BIN} is missing. Please specify the correct build directory."
    exit -1
  fi

  # echo "${APP_BIN} is found."
}

set_ukvm_permission() {
  # apply exec permission to ukvm bin
  if [ -e ${UKVM_BIN} -a ! -x ${UKVM_BIN} ]; then
    chmod a+x ${UKVM_BIN}
  fi
}

measure_ukvm_exec_time() {
  arg_csv=$1
  arg_log=$2
  arg_uargs=${@:3}

  T_START=`date +%s%N`
  ${UKVM_BIN} --mem=1024 --disk=${APP_BIN} --net=${TAP_IF} ${MON_OPT} ${LOAD_OPT} ${APP_BIN} ${APP_BIN} ${arg_uargs} >& ${arg_log}
  # echo "${UKVM_BIN} --mem=1024 --disk=${APP_BIN} --net=${TAP_IF} ${MON_OPT} ${LOAD_OPT} ${APP_BIN} ${APP_BIN} ${arg_uargs} >& ${arg_log}"
  T_END=`date +%s%N`

  # calculate execution time in nanoseconds
  T_ELAPSED=`echo "scale=9; (${T_END} - ${T_START})/1000000000" | bc`
  echo "${T_ELAPSED}," > ${arg_csv}
  echo "${T_ELAPSED},"

  # perf stat -x , ${UKVM_BIN} --mem=1024 --disk=${APP_BIN} --net=${TAP_IF} ${MON_OPT} ${LOAD_OPT} ${APP_BIN} ${APP_BIN} ${USER_ARGS} 2> /dev/null 1> ${CSV}
  # /usr/bin/time -o out.csv -f '%e, %S, %U, %P' ls
}

measure_ukvm_exec_time_py() {
  arg_dir=$1
  arg_out_csv=$2
  arg_app_list=$3
  arg_repeat=$4
  arg_cmd=${@:5}

  python3 ${EVAL_SCRIPT_ROOT}/measure_time.py ${arg_dir} ${arg_out_csv} ${arg_app_list} ${arg_repeat} ${arg_cmd}
}


