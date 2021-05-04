#!/bin/bash
BUILD_DIR=$1
APP_BIN=funkycl-app

UKVM_BIN=${INCLUDEOS_PREFIX}/includeos/x86_64/lib/ukvm-bin

if [ -z ${BUILD_DIR} ]; then
  echo "Usage: ./execute.sh <build_dir>"
  exit -1
fi

if [ ! -e ${BUILD_DIR}/funkycl-app ]; then
  echo "Error: binary ${APP_BIN} is missing. Please do ./build.sh ${BUILD_DIR} first."
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

if [ -e ${UKVM_BIN} -a ! -x ${UKVM_BIN} ]; then
  chmod a+x ${UKVM_BIN}
fi

sudo -E ${UKVM_BIN} --disk=${BUILD_DIR}/funkycl-app --net=tap100 ${BUILD_DIR}/funkycl-app
