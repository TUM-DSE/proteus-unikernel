cmake_minimum_required(VERSION 2.8.9)

# IncludeOS install location
if (NOT DEFINED ENV{INCLUDEOS_PREFIX})
  set(ENV{INCLUDEOS_PREFIX} /usr/local)
endif()

include($ENV{INCLUDEOS_PREFIX}/includeos/pre.service.cmake)
project (hello_xilinx)

# set(SERVICE_NAME "Service with no Service::start, only main")
set(SERVICE_NAME "hello world in Vitis Accel Examples")
set(BINARY       "funkycl-app")
# set(MAX_MEM 128)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/xcl2)

set(SOURCES 
  xcl2/xcl2.hpp
  xcl2/xcl2.cpp
  host.cpp)

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
