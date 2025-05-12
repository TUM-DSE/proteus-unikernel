#!/bin/bash

source ../common.sh

VITIS_ORIG_DIR=$1
ROSETTA_ORIG_DIR=$2

count_loc() {
  arg_dir=$1
  arg_orig_dir=$2
  arg_apps_array=$3[@]
  arg_output_dir=$4
  arg_apps=("${!arg_apps_array}")

  for i in ${!arg_apps[@]}; do
    app=${arg_apps[$i]}
    echo "count ${app}..."
    cd ${arg_dir}/${app}

    LOG="${arg_output_dir}/${arg_apps[$i]}.log"
    CSV="${arg_output_dir}/${arg_apps[$i]}.csv"

    HOSTCODE="host.cpp"
    HOSTCODE_DIR="src"
    if [ ${app} = "simple_vadd" ]; then
      HOSTCODE="vadd.cpp"
    elif [ ${arg_dir} = ${ROSETTA_DIR} ]; then
      HOSTCODE=""
      HOSTCODE_DIR="src/host"
    fi

    COMMIT=`git rev-parse HEAD` # get commit hash
    # LOC of orig code
    # cloc --quiet ${arg_orig_dir}/${app}/${HOSTCODE_DIR}/${HOSTCODE} ${arg_orig_dir}/${app}/${HOSTCODE_DIR}/*.h --include-lang=C/C++\ Header,C,C++ --exclude-dir=build --by-file | tee ${LOG}
    # cloc --quiet ${arg_orig_dir}/${app}/${HOSTCODE_DIR}/${HOSTCODE} ${arg_orig_dir}/${app}/${HOSTCODE_DIR}/*.h --include-lang=C/C++\ Header,C,C++ --exclude-dir=build --by-file --csv >> ${CSV}

    if [ ${app} = "optical-flow" ]; then
      cp -ar ${arg_orig_dir}/${app}/imageLib/ ${arg_orig_dir}/${app}/${HOSTCODE_DIR}/
    fi

    # cloc --quiet ${arg_dir}/${app}/${HOSTCODE} ${arg_dir}/${app}/*.h --include-lang=C/C++\ Header,C,C++ --exclude-dir=build --by-file | tee ${LOG}
    # cloc --quiet ${arg_dir}/${app}/${HOSTCODE} ${arg_dir}/${app}/*.h --include-lang=C/C++\ Header,C,C++ --exclude-dir=build --by-file --csv >> ${CSV}

    # echo "code changes for Funky" >> ${CSV}
    cloc --quiet --diff ${arg_orig_dir}/${app}/${HOSTCODE_DIR}/${HOSTCODE} ${arg_dir}/${app}/${HOSTCODE} --include-lang=C/C++\ Header,C,C++ --exclude-dir=build --by-file | tee ${LOG}
    # First line is empty, skip it with tail -n +2
    cloc --quiet --diff ${arg_orig_dir}/${app}/${HOSTCODE_DIR}/${HOSTCODE} ${arg_dir}/${app}/${HOSTCODE} --include-lang=C/C++\ Header,C,C++ --exclude-dir=build --by-file --csv | tail -n +2 >> ${CSV}

    if [ ${app} = "optical-flow" ]; then
      rm -r ${arg_orig_dir}/${app}/${HOSTCODE_DIR}/imageLib
    fi
  done

  # additional count for common lib
  if [ ${arg_dir} = ${VITIS_EXAMPLES_DIR} ]; then
    LOG="${arg_output_dir}/funky_utils.log"
    CSV="${arg_output_dir}/funky_utils.csv"

    # LOC of orig code
    # cloc --quiet ${arg_orig_dir}/../common/includes --exclude-dir=oclHelper,opencl,simplebmp --by-file | tee ${LOG}
    # cloc --quiet ${arg_orig_dir}/../common/includes --exclude-dir=oclHelper,opencl,simplebmp --by-file --csv >> ${CSV}

    # cloc --quiet ${arg_dir}/../../funky_utils --exclude-dir=oclHelper,opencl,simplebmp --by-file | tee ${LOG}
    # cloc --quiet ${arg_dir}/../../funky_utils --exclude-dir=oclHelper,opencl,simplebmp --by-file --csv >> ${CSV}

    # echo "code changes for Funky" >> ${CSV}
    cloc --quiet --diff ${arg_orig_dir}/../common/includes ${arg_dir}/../../funky_utils --exclude-dir=oclHelper,opencl,simplebmp,memdisk,acl2,timer --by-file | tee ${LOG}
    cloc --quiet --diff ${arg_orig_dir}/../common/includes ${arg_dir}/../../funky_utils --exclude-dir=oclHelper,opencl,simplebmp,memdisk,acl2,timer --by-file --csv | tail -n +2 >> ${CSV}
  else 
    LOG="${arg_output_dir}/harness.log"
    CSV="${arg_output_dir}/harness.csv"

    # cloc --quiet ${arg_orig_dir}/harness/ocl_src --by-file | tee ${LOG}
    # cloc --quiet ${arg_orig_dir}/harness/ocl_src --by-file --csv >> ${CSV}

    cloc --quiet ${arg_dir}/harness --by-file | tee ${LOG}
    cloc --quiet ${arg_dir}/harness --by-file --csv >> ${CSV}

    echo "code changes for Funky" >> ${CSV}
    cloc --quiet --diff ${arg_orig_dir}/harness/ocl_src ${arg_dir}/harness --by-file | tee ${LOG}
    cloc --quiet --diff ${arg_orig_dir}/harness/ocl_src ${arg_dir}/harness --by-file --csv | tail -n +2 >> ${CSV}
  fi
}

DIR="${EVAL_SCRIPT_ROOT}/portability/loc_$DATE"
mkdir -p $DIR

count_loc ${VITIS_EXAMPLES_DIR} ${VITIS_ORIG_DIR} VITIS_EXAMPLES_APPS ${DIR}
count_loc ${ROSETTA_DIR} ${ROSETTA_ORIG_DIR} ROSETTA_APPS ${DIR}

# Summarize all csv files in one file
loc_file="$DIR"/loc.csv
echo "Saving summary in $loc_file"
head -n 1 "$DIR"/"${VITIS_EXAMPLES_APPS[0]}".csv > "$loc_file"

for f in "$DIR"/*.csv; do
  if [ "$f" == "$loc_file" ]; then
    continue
  fi
  tail -n +2 "$f" >> "$loc_file"
done
