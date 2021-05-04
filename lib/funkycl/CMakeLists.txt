cmake_minimum_required(VERSION 2.8.9)

add_definitions(-DARCH_${ARCH})
add_definitions(-DARCH="${ARCH}")

include_directories(${INCLUDEOS_ROOT}/api/posix)
include_directories(${LIBCXX_INCLUDE_DIR})
include_directories(${MUSL_INCLUDE_DIR})
include_directories(${INCLUDEOS_ROOT}/src/include)
include_directories(${INCLUDEOS_ROOT}/api)
include_directories(${INCLUDEOS_ROOT}/lib/funkycl/include/funkycl_lib)

set(LIBRARY_NAME "funkycl_lib")

set(SOURCES
  funkycl.cpp
  )

# LiveUpdate static library
add_library(${LIBRARY_NAME} STATIC ${SOURCES})
add_dependencies(${LIBRARY_NAME} PrecompiledLibraries)

install(TARGETS ${LIBRARY_NAME} DESTINATION includeos/${ARCH}/lib)
install(DIRECTORY include/funkycl_lib DESTINATION includeos/include)
