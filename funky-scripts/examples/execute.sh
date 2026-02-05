#!/bin/bash
BUILD_DIR=build/
UKVM_BIN=${INCLUDEOS_PREFIX}/includeos/x86_64/lib/ukvm-bin
TAP_IF=tap100
SOCKET_IF=/tmp/solo5_socket
MIG_FILE=mig_file
# The monitor expects the bitstream file at this location.
# In the full system, it is saved there by the scheduler.
BITSTREAM=/tmp/bitstream_0.ukvm

function usage {
  cat <<EOF

Usage:
  $(basename ${0}) [<options>]

Options:
  -h                    print help

  -g                    run the unikernel in debug mode (gdb)

  -m                    enable VM migration

  -l                    load VM from migration file

  -o                    enable out-of-order execution for the OpenCL commmand queue

  -b <build_dir>        set path to dir including binary
                        (default: ${BUILD_DIR})

  -u <ukvm-bin>         set path to ukvm-bin
                        (default: ${UKVM_BIN})

  -t <fpga>             FPGA type (arria10, u50, or u280)

  -i <bitstream>        Bitstream for programming FPGA
                        (default: ${BITSTREAM})

  -n <network device>   set tap device
                        (default: ${TAP_IF})

  -s <socket name>      set socket file name
                        (default: ${SOCKET_IF})

  -f <migfile name>     set migration file name
                        (default: ${MIG_FILE})

  -a <arguments...>     set arguments for the app

EOF
}


########### get arguments #############
GDB_FLAG=false
MON_FLAG=false
LOAD_FLAG=false
OOO_FLAG=false

while getopts hgmlob:u:t:i:n:s:f:a: OPT
do
  case $OPT in
    "h" )
      usage
      exit -1 ;;
    "g" )
      GDB_FLAG=true ;;
    "m" )
      MON_FLAG=true ;;
    "l" )
      LOAD_FLAG=true ;;
    "o" )
      OOO_FLAG=true ;;
    "b" )
      USER_BUILD_DIR=${OPTARG} ;;
    "u" )
      USER_UKVM_BIN=${OPTARG} ;;
    "t" )
      USER_FPGA=${OPTARG} ;;
    "i" )
      USER_BITSTREAM=${OPTARG} ;;
    "n" )
      USER_TAP_IF=${OPTARG} ;;
    "s" )
      USER_SOCKET_IF=${OPTARG} ;;
    "f" )
      USER_MIG_FILE=${OPTARG} ;;
    "a" )
      USER_ARGS=${OPTARG} ;;
    * )
      echo "WARNING: ignoring unknown option" ;;
  esac
done

########### set arguments #############
if [ -n "${USER_BUILD_DIR}" ]; then
  echo "INFO: ${USER_BUILD_DIR} is specified as the build directory."
  BUILD_DIR=${USER_BUILD_DIR}
fi

if [ -n "${USER_UKVM_BIN}" ]; then
  echo "INFO: ${USER_UKVM_BIN} is used as the backend monitor."
  UKVM_BIN=${USER_UKVM_BIN}
fi

if [ -n "${USER_FPGA}" ]; then
  echo "INFO: ${USER_FPGA} is used as the FPGA type."
  FPGA=${USER_FPGA}
fi

if [ -n "${USER_BITSTREAM}" ]; then
  ln -sf "$(realpath "$USER_BITSTREAM")" ${BITSTREAM} || exit 1
  echo "INFO: ${USER_BITSTREAM} is used as the bitstream."
else
  echo "INFO: ${BITSTREAM} is used as the bitstream"
fi

if [ -n "${USER_SOCKET_IF}" ]; then
  echo "INFO: ${USER_SOCKET_IF} is used as a socket interface."
  SOCKET_IF=${USER_SOCKET_IF}
fi

if [ -n "${USER_MIG_FILE}" ]; then
  echo "INFO: ${USER_MIG_FILE} is used as a migration file."
  MIG_FILE=${USER_MIG_FILE}
fi

if [ -n "${USER_TAP_IF}" ]; then
  echo "INFO: ${USER_TAP_IF} is used as a tap interface."
  TAP_IF=${USER_TAP_IF}
fi

########### Error check #############
if [ ! -e ${BUILD_DIR} ]; then
  echo "Error: ${BUILD_DIR} doesn't exist. Please specify the correct build directory or compile an application first."
  exit -1
fi

# search build_dir for unikernel app binary
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

if [ -z ${FPGA} ]; then
  echo "Error: FPGA type is not set, use -t <fpga>."
  exit -1
fi

if [ -z ${USER_BITSTREAM} ] && [ ! -f ${BITSTREAM} ]; then
  echo "Error: no bitstream at ${BITSTREAM} or provided with -i <bitstream>."
  exit -1
fi

########### Execute app #############
# apply exec permission to ukvm bin
if [ -e ${UKVM_BIN} -a ! -x ${UKVM_BIN} ]; then
  chmod a+x ${UKVM_BIN}
fi

if "${MON_FLAG}" ; then
  MON_OPT="--mon=${SOCKET_IF}"
fi

if "${LOAD_FLAG}" ; then
  LOAD_OPT="--load=${MIG_FILE}"
fi

if "${OOO_FLAG}" ; then
  OOO_OPT="--ooo"
fi

if "${GDB_FLAG}" ; then
  echo "Usage: run --mem=4096 --disk=${APP_BIN} --net=${TAP_IF} --fpga=${FPGA} ${MON_OPT} ${LOAD_OPT} ${OOO_OPT} ${APP_BIN} ${USER_ARGS}"
  echo "Press Enter to start gdb..."
  read Wait
  gdb -tui ${UKVM_BIN}
else
  ${UKVM_BIN} --mem=4096 --disk=${APP_BIN} --net=${TAP_IF} --fpga=${FPGA} ${MON_OPT} ${LOAD_OPT} ${OOO_OPT} ${APP_BIN} ${USER_ARGS}
fi
