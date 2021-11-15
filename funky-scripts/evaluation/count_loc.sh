#!/bin/bash

source ./common.sh

VITIS_ORIG_DIR=$1
ROSETTA_ORIG_DIR=$2

count_loc() {
  arg_dir=$1
  arg_orig_dir=$2
  arg_apps_array=$3[@]
  arg_apps=("${!arg_apps_array}")

  for i in ${!arg_apps[@]}; do
    app=${arg_apps[$i]}
    echo "count ${app}..."
    cd ${arg_dir}/${app}

    CSV="${EVAL_SCRIPT_ROOT}/${DIR}/${arg_apps[$i]}.csv"

    HOSTCODE="host.cpp"
    HOSTCODE_DIR="src"
    if [ ${app} = "simple_vadd" ]; then
      HOSTCODE="vadd.cpp"
    elif [ ${arg_dir} = ${ROSETTA_DIR} ]; then
      HOSTCODE=${ROSETTA_CODE[$i]}
      HOSTCODE_DIR="src/host"
    fi

    COMMIT=`git rev-parse HEAD` # get commit hash
    echo "${apps}, ${COMMIT}" >> ${CSV}
    cloc --quiet ${arg_orig_dir}/${app}/${HOSTCODE_DIR}/${HOSTCODE} ${arg_orig_dir}/${app}/${HOSTCODE_DIR}/*.h
    cloc --quiet ${arg_orig_dir}/${app}/${HOSTCODE_DIR}/${HOSTCODE} ${arg_orig_dir}/${app}/${HOSTCODE_DIR}/*.h --csv >> ${CSV}

    echo "code changes for Funky" >> ${CSV}
    cloc --quiet --diff ${arg_orig_dir}/${app}/${HOSTCODE_DIR}/${HOSTCODE} ${arg_dir}/${app}/${HOSTCODE}
    cloc --diff --quiet ${arg_orig_dir}/${app}/${HOSTCODE_DIR}/${HOSTCODE} ${arg_dir}/${app}/${HOSTCODE} --csv >> ${CSV}
  done
}


DIR="loc_$DATE"
mkdir -p $DIR

count_loc ${VITIS_EXAMPLES_DIR} ${VITIS_ORIG_DIR} VITIS_EXAMPLES_APPS
count_loc ${ROSETTA_DIR} ${ROSETTA_ORIG_DIR} ROSETTA_APPS

