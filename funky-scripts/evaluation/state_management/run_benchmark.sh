#!/bin/bash
set -eo pipefail

GUEST_MEM_SIZE=4096 # increase the guest memory size
SOCKET_IF=/tmp/solo5_socket
MON_OPT="--mon=${SOCKET_IF}"
source ../common.sh

SCRIPT_PATH=$(readlink -f "$0")
SCRIPT_DIR=$(dirname "$SCRIPT_PATH")

function usage {
  cat <<EOF

Usage:
  $(basename ${0}) <bench_type> <repeat>

  <bench_type> sync_oh, fpga_state_oh, vm_state_oh

  Measure overheads of the selected functions using microbenchmarks.
EOF
}

run_benchmark() {
  arg_benchdir=$1
  arg_py_script=$2
  arg_repeat=$3
  arg_resultdir=$4
  arg_ofile=$5
  arg_fpga=$6

  ### execution
  echo "move into ${arg_benchdir}..."
  pushd ${arg_benchdir}
  python3 ${arg_py_script} ${arg_resultdir} ${arg_resultdir}/${arg_ofile} ${arg_repeat} ${arg_fpga} ${UKVM_EXEC_CMD}
  popd
  echo "back to $(pwd)."
}

BENCH_TYPE=$1
case "${BENCH_TYPE}" in
  "sync_oh")
  bench_name=mig_sync_oh
  py_script=measure_sync_oh.py
  ;;
  "fpga_state_oh")
  bench_name=mig_shift_reg
  py_script=measure_fstate_oh.py
  ;;
  "vm_state_oh")
  bench_name=mig_shift_reg
  py_script=measure_vmstate_oh.py
  UKVM_EXEC_CMD="${UKVM_BIN} --fpga=u50 --mem=${GUEST_MEM_SIZE} --disk=${APP_BIN} --net=${TAP_IF} ${MON_OPT} --checkpoint ${LOAD_OPT} ${APP_BIN} ${APP_BIN}"
  ;;
  "migration_oh")
  bench_name=mig_shift_reg
  py_script=measure_migration_oh.py
  ;;
  *)
  usage
  exit -1
  ;;
esac

REPEAT=$2
if [ -z ${REPEAT} ]; then
  usage
  exit -1
fi

### make ukvm-bin executable
set_ukvm_permission

### create dir where the results are saved
RESULTS_DIR="${SCRIPT_DIR}/${BENCH_TYPE}_$DATE"
mkdir -p ${RESULTS_DIR}

FPGAS=$PROTEUS_FPGAS

for fpga in $FPGAS; do
  fpga_model=${fpga%%-*}

  # UKVM expects the bitstream at /tmp/bitstream_0.ukvm
  ln -sf "/share/felix/bitstreams/vitis-accel-examples/cl_shift_register/$fpga/bitstream" /tmp/bitstream_0.ukvm
  UKVM_EXEC_CMD="${UKVM_BIN} --fpga=$fpga_model --mem=${GUEST_MEM_SIZE} --disk=${APP_BIN} --net=${TAP_IF} ${MON_OPT} ${LOAD_OPT} ${APP_BIN} ${APP_BIN}"

  if [ "$BENCH_TYPE" == "vm_state_oh" ]; then
    # Add --checkpoint to save VM snapshots to disk instead of RAM
    UKVM_EXEC_CMD="${UKVM_BIN} --fpga=$fpga_model --mem=${GUEST_MEM_SIZE} --disk=${APP_BIN} --net=${TAP_IF} ${MON_OPT} --checkpoint ${LOAD_OPT} ${APP_BIN} ${APP_BIN}"
  fi

  echo "[$fpga]"

  ### run benchmark
  run_benchmark ${MICROBENCHMARKS_DIR}/${bench_name} ${SCRIPT_DIR}/${py_script} ${REPEAT} ${RESULTS_DIR} "${BENCH_TYPE}.csv" ${fpga}
done
