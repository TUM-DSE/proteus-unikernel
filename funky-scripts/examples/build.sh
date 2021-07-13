#!/bin/bash
BUILD_DIR=$1

if [ -z ${BUILD_DIR} ]; then
  echo 'Usage: ./build.sh <build_dir>'
  exit -1
fi

if [ -e ${BUILD_DIR} ]; then
  echo "Warning: the selected directory ${BUILD_DIR}/ already exists. This script removes the old one and re-compile it."
  rm -r ${BUILD_DIR}
fi

mkdir ${BUILD_DIR}
pushd ${BUILD_DIR}
PLATFORM=x86_solo5 cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j 8
popd

