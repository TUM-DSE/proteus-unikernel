# Vitis Accel Examples

This directory contains the host code for some simple FPGA OpenCL programs from the [Vitis_Accel_Examples repo](https://github.com/TUM-DSE/Vitis_Accel_Examples/tree/intel-port).

The [kernel code for cpp_kernels](https://github.com/TUM-DSE/Vitis_Accel_Examples/tree/intel-port/cpp_kernels) is written in Vitis HLS while the [kernel code for ocl_kernels](https://github.com/TUM-DSE/Vitis_Accel_Examples/tree/intel-port/ocl_kernels) is written in OpenCL and has been ported to Intel FPGAs.

For each example, use `./build.sh -h` and `./execute.sh -h` for instructions how to build and execute the code.
The bitstreams can be found in `xclbin/<fpga>/vitis-acc-ex-ocl/`.
