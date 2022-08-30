cmake_minimum_required(VERSION 2.8.9)

# IncludeOS install location
if (NOT DEFINED ENV{INCLUDEOS_PREFIX})
  set(ENV{INCLUDEOS_PREFIX} /usr/local)
endif()

include($ENV{INCLUDEOS_PREFIX}/includeos/pre.service.cmake)
project (pipecnn_intel)

# set(SERVICE_NAME "Service with no Service::start, only main")
set(SERVICE_NAME "pipecnn in Intel FPGA")
set(BINARY       "funkycl-app")
set(CMAKE_BUILD_TYPE Debug)
set(FUNKY_UTILS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../funky_utils)
set(PIPECNN_COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../common)

# set(MAX_MEM 128)

include_directories(${FUNKY_UTILS_DIR})
include_directories(${PIPECNN_COMMON_DIR})

set(SOURCES 
  ${PIPECNN_COMMON_DIR}/timer.cpp 
  ${PIPECNN_COMMON_DIR}/ocl_util.cpp 
  ${FUNKY_UTILS_DIR}/memdisk/diskio.cpp 
  main.cpp)

if ("$ENV{PLATFORM}" STREQUAL "x86_solo5")
  set(DRIVERS 
    solo5fpga
    )
endif()

set(LIBRARIES
  libfunkycl.a
  )

# include service build script
include($ENV{INCLUDEOS_PREFIX}/includeos/post.service.cmake)

# mount a bitstream file in VFS by using memdisk
diskbuilder(../../../xclbin/aria10/pipecnn_emulation)
