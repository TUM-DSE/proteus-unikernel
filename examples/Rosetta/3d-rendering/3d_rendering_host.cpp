/*===============================================================*/
/*                                                               */
/*                       3d_rendering.cpp                        */
/*                                                               */
/*      Main host function for the 3D Rendering application.     */
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
#include <chrono>

#define OCL

#ifdef OCL
  // harness headers
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
  #include "../sdsoc/rendering.h"
#endif

#ifdef SW
  # include "../sw/rendering_sw.h"
#endif

// other headers
#include "utils.h"
#include "typedefs.h"
#include "check_result.h"

// data
#include "input_data.h"


int main(int argc, char ** argv) 
{
  printf("3D Rendering Application\n");

  #ifdef OCL
    // parse command line arguments for opencl version
    std::string kernelFile("");
    parse_sdaccel_command_line_args(argc, argv, kernelFile);
  #endif
  // sdsoc and sw versions have no additional command line arguments

  // for this benchmark, data is included in array triangle_3ds

  // timers
  struct timeval start, end;

  // opencl version host code
  #ifdef OCL

    // create space for input and output
    bit32* input  = new bit32[3 * NUM_3D_TRI];
    bit32* output = new bit32[NUM_FB];

    // times in ns
    uint64_t nstime_data_to_fpga = 0;
    uint64_t nstime_kernel = 0;
    uint64_t nstime_data_to_host = 0;
  
    // pack input data for better performance
    for ( int i = 0; i < NUM_3D_TRI; i ++)
    {
      input[3*i](7,0)     = triangle_3ds[i].x0;
      input[3*i](15,8)    = triangle_3ds[i].y0;
      input[3*i](23,16)   = triangle_3ds[i].z0;
      input[3*i](31,24)   = triangle_3ds[i].x1;
      input[3*i+1](7,0)   = triangle_3ds[i].y1;
      input[3*i+1](15,8)  = triangle_3ds[i].z1;
      input[3*i+1](23,16) = triangle_3ds[i].x2;
      input[3*i+1](31,24) = triangle_3ds[i].y2;
      input[3*i+2](7,0)   = triangle_3ds[i].z2;
      input[3*i+2](31,8)  = 0;
    }
  
    // create OpenCL world
    CLWorld rendering_world = CLWorld(TARGET_DEVICE, CL_DEVICE_TYPE_ACCELERATOR);
  
    // add the bitstream file
    rendering_world.addProgram(kernelFile);
  
    // create kernels
    // this kernel is written in C++
    CLKernel Rendering(rendering_world.getContext(), rendering_world.getProgram(), "rendering", rendering_world.getDevice());
  
    // create mem objects
    CLMemObj input_mem ( (void*)input,  sizeof(bit32), 3 * NUM_3D_TRI, CL_MEM_READ_ONLY);
    CLMemObj output_mem( (void*)output, sizeof(bit32), NUM_FB,         CL_MEM_WRITE_ONLY);
  
    auto q = rendering_world.getCmdQueue();
    clFinish(q);

    // start timer
    gettimeofday(&start, 0);

    auto start_time = std::chrono::high_resolution_clock::now();
  
    // add them to the world
    // added in sequence, each of them can be referenced by an index
    rendering_world.addMemObj(input_mem, nstime_data_to_fpga);
    rendering_world.addMemObj(output_mem, nstime_data_to_fpga);
  
    // set work size
    int global_size[3] = {1, 1, 1};
    int local_size[3] = {1, 1, 1};
    Rendering.set_global(global_size);
    Rendering.set_local(local_size);
  
    // add them to the world
    rendering_world.addKernel(Rendering);
  
    // set kernel arguments
    rendering_world.setMemKernelArg(0, 0, 0);
    rendering_world.setMemKernelArg(0, 1, 1);
  
    // run!
    rendering_world.runKernels(nstime_kernel);
  
    // read the data back
    rendering_world.readMemObj(1, nstime_data_to_host);

    clFinish(q);

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double>(end_time - start_time);
    auto nstime_cpu = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
    
    // end timer
    gettimeofday(&end, 0);
  #endif

  #ifdef SDSOC
    // create space for input and output
    bit32 *input = (bit32*)sds_alloc(3 * NUM_3D_TRI * sizeof(bit32));
    bit32* output = (bit32*)sds_alloc(NUM_FB * sizeof(bit32));

    // pack input data for better performance
    for ( int i = 0; i < NUM_3D_TRI; i++)
    {
      input[3*i](7,0)     = triangle_3ds[i].x0;
      input[3*i](15,8)    = triangle_3ds[i].y0;
      input[3*i](23,16)   = triangle_3ds[i].z0;
      input[3*i](31,24)   = triangle_3ds[i].x1;
      input[3*i+1](7,0)   = triangle_3ds[i].y1;
      input[3*i+1](15,8)  = triangle_3ds[i].z1;
      input[3*i+1](23,16) = triangle_3ds[i].x2;
      input[3*i+1](31,24) = triangle_3ds[i].y2;
      input[3*i+2](7,0)   = triangle_3ds[i].z2;
      input[3*i+2](31,8)  = 0;
    }

    // run hardware function and time it

    gettimeofday(&start, 0);
    rendering(input, output);
    gettimeofday(&end, 0);

  #endif

  #ifdef SW
    // output
    bit8 output[MAX_X][MAX_Y];
    // run and time sw function
    gettimeofday(&start, 0);
    rendering_sw(triangle_3ds, output);
    gettimeofday(&end, 0);
  #endif 

  auto input_size = 3 * NUM_3D_TRI * sizeof(bit32);
  auto output_size = NUM_FB * sizeof(bit32);

  std::cout << "app_name,kernel_input_data_size,kernel_output_data_size,iterations,time_cpu,data_to_fpga_time_ocl,kernel_time_ocl,data_to_host_time_ocl\n"
            << "3d_rendering,"
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
  check_results(output);
 
  // print time
  long long elapsed = (end.tv_sec - start.tv_sec) * 1000000LL + end.tv_usec - start.tv_usec;   
  printf("elapsed time: %lld us\n", elapsed);

  // cleanup
  #ifdef OCL
    rendering_world.releaseWorld();
    delete []input;
    delete []output;
  #endif

  #ifdef SDSOC
    sds_free(input);
    sds_free(output);
  #endif

  return EXIT_SUCCESS;

}
