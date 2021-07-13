#!/bin/bash
BUILD_DIR=build/
UKVM_BIN=${INCLUDEOS_PREFIX}/includeos/x86_64/lib/ukvm-bin
TAP_IF=tap100

function usage {
  cat <<EOF

Usage: 
  $(basename ${0}) [<options>]

Options: 
  -h                    print help

  -u <build_dir>        set path to dir including binary 
                        (default: ${BUILD_DIR})

  -u <ukvm-bin>         set path to ukvm-bin 
                        (default: ${UKVM_BIN})

  -t <network device>   set tap device 
                        (default: ${TAP_IF})

  -a <arguments...>     set arguments for the app 

EOF
}

if [ ! -e ${BUILD_DIR} ]; then
  echo "Error: ${BUILD_DIR} doesn't exist. Please specify the correct build directory or compile an application first."
  exit -1
fi

# search build_dir for unikernel app binary
APP_BIN=$(find ${BUILD_DIR} -maxdepth 1 -executable -type f)

########### Error check #############
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

########### get arguments #############
while getopts u:n:a:h OPT
do
  case $OPT in
    "h" )
      usage
      exit -1 ;;
    "u" )
      USER_UKVM_BIN=${OPTARG} ;;
    "n" )
      USER_TAP_IF=${OPTARG} ;;
    "a" )
      USER_ARGS=${OPTARG} ;;
  esac
done

########### set arguments #############
if [ ! -z ${USER_UKVM_BIN} ]; then
  echo "INFO: ${USER_UKVM_BIN} is used as the monitor."
  UKVM_BIN=${USER_UKVM_BIN}
fi

if [ ! -z ${USER_TAP_IF} ]; then
  echo "INFO: ${USER_TAP_IF} is used as a tap interface."
  TAP_IF=${USER_TAP_IF}
fi



########### Execute app #############
# apply exec permission to ukvm bin
if [ -e ${UKVM_BIN} -a ! -x ${UKVM_BIN} ]; then
  chmod a+x ${UKVM_BIN}
fi

# sudo -E ${UKVM_BIN} --disk=${APP_BIN} --net=tap100 ${APP_BIN}
${UKVM_BIN} --disk=${APP_BIN} --net=tap100 ${APP_BIN} ${USER_ARGS}
