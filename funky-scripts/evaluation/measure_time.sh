#!/bin/bash

source ./common.sh

measure_time() {
  arg_dir=$1
  arg_apps_array=$2[@]
  arg_apps=("${!arg_apps_array}")
  arg_args_array=$3[@]
  arg_args=("${!arg_args_array}")
  

  DIR="time_$DATE"
  mkdir -p ${EVAL_SCRIPT_ROOT}/$DIR

  # evaluation for Vitis_Accel_Examples
  for i in ${!arg_apps[@]}; do
    echo "execute ${arg_apps[$i]}..."
    cd ${arg_dir}/${arg_apps[$i]}
    find_binary "build/"
  
    CSV="${EVAL_SCRIPT_ROOT}/${DIR}/${arg_apps[$i]}.csv"
    LOG="${EVAL_SCRIPT_ROOT}/${DIR}/${arg_apps[$i]}.log"
    USER_ARGS=${arg_args[$i]}
  
    # execute app
    measure_ukvm_exec_time ${CSV} ${LOG} ${USER_ARGS}
  done 
}

set_ukvm_permission

DIR="time_$DATE"
mkdir -p ${EVAL_SCRIPT_ROOT}/$DIR

measure_time ${VITIS_EXAMPLES_DIR} VITIS_EXAMPLES_APPS VITIS_EXAMPLES_ARGS
measure_time ${ROSETTA_DIR} ROSETTA_APPS ROSETTA_ARGS

