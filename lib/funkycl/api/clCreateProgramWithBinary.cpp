#include "config.h"
#include <CL/opencl.h>
#include "object/object.h"
#include "object/context.h"
#include "object/program.h"

namespace funkycl {

static cl_program
clCreateProgramWithBinary(cl_context                     context,
                          cl_uint                        num_devices,
                          const cl_device_id *           device_list,
                          const size_t *                 lengths,
                          const unsigned char **         binaries,
                          cl_int *                       binary_status,
                          cl_int *                       errcode_ret)
{
  auto program = std::make_unique<funkycl::program>(cl_to_funkycl(context), num_devices, device_list, binaries, lengths);

  if(errcode_ret)
    *errcode_ret = CL_SUCCESS;

  return program.release();
}

} // funkycl

CL_API_ENTRY cl_program CL_API_CALL
clCreateProgramWithBinary(cl_context                     context,
                          cl_uint                        num_devices,
                          const cl_device_id *           device_list,
                          const size_t *                 lengths,
                          const unsigned char **         binaries,
                          cl_int *                       binary_status,
                          cl_int *                       errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
  return funkycl::clCreateProgramWithBinary
    (context,num_devices,device_list,lengths, binaries, binary_status, errcode_ret);
}


