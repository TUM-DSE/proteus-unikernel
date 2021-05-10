#!/bin/bash
BUILD_DIR=$1
USER_UKVM_BIN=$2

UKVM_BIN=${INCLUDEOS_PREFIX}/includeos/x86_64/lib/ukvm-bin

if [ -z ${BUILD_DIR} ]; then
  echo "Usage: ./execute.sh <build_dir> [<path to ukvm-bin>]"
  exit -1
fi

APP_BIN=$(find ${BUILD_DIR} -maxdepth 1 -executable -type f)

if [ ! -e ${APP_BIN} ]; then
  echo "Error: binary ${APP_BIN} is missing. Please specify the correct build directory."
  exit -1
fi

if [ -z ${XILINX_XRT} ]; then
  echo "Error: XILINX_XRT is not set. Please install XRT."
  exit -1
fi

if [ -z ${INCLUDEOS_PREFIX} ]; then
  echo "Error: INCLUDEOS_PREFIX is not set. Please install Funky OS."
  exit -1
fi


if [ ! -z ${USER_UKVM_BIN} ]; then
  echo "INFO: ${USER_UKVM_BIN} is used as the monitor."
  UKVM_BIN=${USER_UKVM_BIN}
fi

if [ -e ${UKVM_BIN} -a ! -x ${UKVM_BIN} ]; then
  chmod a+x ${UKVM_BIN}
fi

sudo -E ${UKVM_BIN} --disk=${APP_BIN} --net=tap100 ${APP_BIN}
