#!/bin/bash
BUILD_DIR=$1

function usage {
  cat <<EOF
Usage: 
  $(basename ${0}) <build dir>

Options: 
  -h                    print help
EOF
}

########### get arguments #############
while getopts h OPT
do
  case $OPT in
    "h" )
      usage
      exit -1 ;;
  esac
done

if [ -z ${BUILD_DIR} ]; then
  usage
  exit -1
fi

if [ -e ${BUILD_DIR} ]; then
  echo "Warning: the selected directory ${BUILD_DIR}/ already exists. This script will remove the old one and re-compile it. "
  read -p "Do you continue?: [y/N]: " yn
  case ${yn} in
    [yY]*) 
      rm -r ${BUILD_DIR}
      ;;
    *) 
      echo "Abort."
      exit -1 ;;
  esac
fi

mkdir ${BUILD_DIR}
pushd ${BUILD_DIR}
PLATFORM=x86_solo5 cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j 8
popd

