/*===============================================================*/
/*                                                               */
/*                    digit_recognition.cpp                      */
/*                                                               */
/*   Main host function for the Digit Recognition application.   */
/*                                                               */
/*===============================================================*/

#include <os>

// standard C/C++ headers
#include <cstdio>
#include <cstdlib>
#include <getopt.h>
#include <string>
#include <time.h>
#include <sys/time.h>
#include <iostream>
#include <iomanip>
#include <chrono>

#ifdef OCL
  // opencl harness headers
  #include "CLWorld.h"
  #include "CLKernel.h"
  #include "CLMemObj.h"
  // harness namespace
  using namespace rosetta;
#endif

#ifdef SDSOC
  // sdsoc headers
  #include "sds_lib.h"
  // hardware function declaration
  #include "../sdsoc/digitrec.h"
#endif

#ifdef SW
  # include "../sw/digitrec_sw.h"
#endif

// other headers
#include "utils.h"
#include "typedefs.h"
#include "check_result.h"

// data
#include "training_data.h"
#include "testing_data.h"

int main(int argc, char ** argv) 
{
  printf("Digit Recognition Application\n");

  #ifdef OCL
    // parse command line arguments for opencl version
    std::string kernelFile("");
    parse_sdaccel_command_line_args(argc, argv, kernelFile);
  #endif
  // sw and sdsoc version have no additional command line arguments

  // for this benchmark, data is already included in arrays:
  //   training_data: contains 18000 training samples, with 1800 samples for each digit class
  //   testing_data:  contains 2000 test samples
  //   expected:      contains labels for the test samples

  // timers
  struct timeval start, end;

  // opencl version host code
  #ifdef OCL
    // times in ns
    uint64_t nstime_data_to_fpga = 0;
    uint64_t nstime_kernel = 0;
    uint64_t nstime_data_to_host = 0;

    // create space for the result
    LabelType* result = new LabelType[NUM_TEST];

    // create OpenCL world
    CLWorld digit_rec_world = CLWorld(TARGET_DEVICE, CL_DEVICE_TYPE_ACCELERATOR);

    // add the bitstream file
    digit_rec_world.addProgram(kernelFile);

    // create kernels
    CLKernel DigitRec(digit_rec_world.getContext(), digit_rec_world.getProgram(), "DigitRec", digit_rec_world.getDevice());

    // create mem objects
    CLMemObj training_mem ( (void*)training_data,  sizeof(DigitType), NUM_TRAINING * DIGIT_WIDTH, CL_MEM_READ_ONLY);
    CLMemObj testing_mem  ( (void*)testing_data ,  sizeof(DigitType), NUM_TEST     * DIGIT_WIDTH, CL_MEM_READ_ONLY);
    CLMemObj result_mem   ( (void*)result       ,  sizeof(LabelType), NUM_TEST,                   CL_MEM_WRITE_ONLY);

    auto q = digit_rec_world.getCmdQueue();
    clFinish(q);

    // start timer
    gettimeofday(&start, 0);

    auto start_time = std::chrono::high_resolution_clock::now();

    // add them to the world
    // added in sequence, each of them can be referenced by an index
    digit_rec_world.addMemObj(training_mem, nstime_data_to_fpga);
    digit_rec_world.addMemObj(testing_mem, nstime_data_to_fpga);
    digit_rec_world.addMemObj(result_mem, nstime_data_to_fpga);

    // set work size
    int global_size[3] = {1, 1, 1};
    int local_size[3] = {1, 1, 1};
    DigitRec.set_global(global_size);
    DigitRec.set_local(local_size);

    // add them to the world
    digit_rec_world.addKernel(DigitRec);

    // set kernel arguments
    digit_rec_world.setMemKernelArg(0, 0, 0);
    digit_rec_world.setMemKernelArg(0, 1, 1);
    digit_rec_world.setMemKernelArg(0, 2, 2);

    // run!
    digit_rec_world.runKernels(nstime_kernel);

    // read the data back
    digit_rec_world.readMemObj(2, nstime_data_to_host);

    clFinish(q);

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double>(end_time - start_time);
    auto nstime_cpu = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();

    // end timer
    gettimeofday(&end, 0);
  #endif

  // sdsoc version host code
  #ifdef SDSOC
    // allocate space for hardware function
    WholeDigitType* training_in0 = (WholeDigitType*)sds_alloc(sizeof(WholeDigitType) * NUM_TRAINING / 2);
    WholeDigitType* training_in1 = (WholeDigitType*)sds_alloc(sizeof(WholeDigitType) * NUM_TRAINING / 2);
    WholeDigitType* test_in      = (WholeDigitType*)sds_alloc(sizeof(WholeDigitType) * NUM_TEST);
  
    // pack the data into a wide datatype
    for (int i = 0; i < NUM_TRAINING / 2; i ++ )
    {
      training_in0[i].range(63 , 0  ) = training_data[i*DIGIT_WIDTH+0];
      training_in0[i].range(127, 64 ) = training_data[i*DIGIT_WIDTH+1];
      training_in0[i].range(191, 128) = training_data[i*DIGIT_WIDTH+2];
      training_in0[i].range(255, 192) = training_data[i*DIGIT_WIDTH+3];
    }
    for (int i = 0; i < NUM_TRAINING / 2; i ++ )
    {
      training_in1[i].range(63 , 0  ) = training_data[(NUM_TRAINING / 2 + i)*DIGIT_WIDTH+0];
      training_in1[i].range(127, 64 ) = training_data[(NUM_TRAINING / 2 + i)*DIGIT_WIDTH+1];
      training_in1[i].range(191, 128) = training_data[(NUM_TRAINING / 2 + i)*DIGIT_WIDTH+2];
      training_in1[i].range(255, 192) = training_data[(NUM_TRAINING / 2 + i)*DIGIT_WIDTH+3];
    }
  
    for (int i = 0; i < NUM_TEST; i ++ )
    {
      test_in[i].range(63 , 0  ) = testing_data[i*DIGIT_WIDTH+0];
      test_in[i].range(127, 64 ) = testing_data[i*DIGIT_WIDTH+1];
      test_in[i].range(191, 128) = testing_data[i*DIGIT_WIDTH+2];
      test_in[i].range(255, 192) = testing_data[i*DIGIT_WIDTH+3];
    }

    // create space for result
    LabelType* result = (LabelType*)sds_alloc(sizeof(LabelType) * NUM_TEST);

    // run the hardware function
    // first call: transfer data
    DigitRec(training_in0, test_in, result, 0);

    // second call: execute
    gettimeofday(&start, NULL);
    DigitRec(training_in1, test_in, result, 1);
    gettimeofday(&end, NULL);
  #endif

  // sw version host code
  #ifdef SW
    // create space for the result
    LabelType* result = new LabelType[NUM_TEST];

    // software version
    gettimeofday(&start, NULL);
    DigitRec_sw(training_data, testing_data, result);
    gettimeofday(&end, NULL);
  #endif

  // training_mem + testing_mem
  auto input_size = (NUM_TRAINING * DIGIT_WIDTH * sizeof(DigitType)) + (NUM_TEST * DIGIT_WIDTH * sizeof(DigitType));
  auto output_size = NUM_TEST * sizeof(LabelType);

  std::cout << "app_name,kernel_input_data_size,kernel_output_data_size,iterations,time_cpu,data_to_fpga_time_ocl,kernel_time_ocl,data_to_host_time_ocl\n"
            << "digit_recognition,"
            << std::dec
            << input_size << ","
            << output_size << ","
            << 1 << ","
            << std::setprecision(std::numeric_limits<double>::digits10)
            << nstime_cpu / (double)1'000'000'000 << ","
            << nstime_data_to_fpga / (double)1'000'000'000 << ","
            << nstime_kernel / (double)1'000'000'000 << ","
            << nstime_data_to_host / (double)1'000'000'000 << "\n";

  // check results
  printf("Checking results:\n");
  check_results( result, expected, NUM_TEST );
    
  // print time
  long long elapsed = (end.tv_sec - start.tv_sec) * 1000000LL + end.tv_usec - start.tv_usec;   
  printf("elapsed time: %lld us\n", elapsed);

  // cleanup
  #ifdef OCL
    digit_rec_world.releaseWorld();
    delete []result;
  #endif

  #ifdef SDSOC
    sds_free(training_in0);
    sds_free(training_in1);
    sds_free(test_in);
    sds_free(result);
  #endif

  #ifdef SW
    delete []result;
  #endif

  return EXIT_SUCCESS;

}
