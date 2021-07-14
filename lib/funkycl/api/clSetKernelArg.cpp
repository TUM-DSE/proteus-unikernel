#include "config.h"
#include <CL/opencl.h>
#include "object/object.h"
#include "object/kernel.h"

namespace funkycl {

static cl_int
clSetKernelArg(cl_kernel    kernel,
               cl_uint      arg_index,
               size_t       arg_size,
               const void * arg_value)
{
  // TODO: how to confirm if argument is cl_mem or not?
  // There would be no way to detect the type of argument other than reading meta data in xclbin...
  DEBUG_STREAM("set an argument ...");
  auto f_kernel = cl_to_funkycl(kernel);

  f_kernel->create_argument(arg_index);
  f_kernel->set_argument(arg_index, arg_size, arg_value);

  DEBUG_STREAM("finish.");

  return CL_SUCCESS;
}

} // xocl

CL_API_ENTRY cl_int CL_API_CALL
clSetKernelArg(cl_kernel    kernel,
               cl_uint      arg_index,
               size_t       arg_size,
               const void * arg_value) CL_API_SUFFIX__VERSION_1_0
{
  return funkycl::clSetKernelArg(kernel, arg_index, arg_size, arg_value);
}

