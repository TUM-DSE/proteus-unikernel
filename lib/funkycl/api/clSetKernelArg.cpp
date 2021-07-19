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
  auto f_kernel = cl_to_funkycl(kernel);
  DEBUG_STREAM("arg id: " << arg_index <<  ", size: " << arg_size << ", addr: " << arg_value);

  // TODO: how to find out type of the argument: cl_mem or not?
  // There would be no way to detect the type other than reading meta data in xclbin...
  // As a temporal solution, check arg_size and treat the argument as cl_mem if it is the same as sizeof(cl_mem). 
  // However, this goes wrong if the argument is uint64_t, size_t or other 64-bit variables (not pointer). 
  // Another way is to modify original OpenCL API definitions (OpenCL headers), but not preferrable. 

  // size of a pointer to _cl_mem (8 Bytes)
  if(arg_size == sizeof(cl_mem)) 
    f_kernel->create_clmem_argument(arg_index);
  else
    f_kernel->create_scalar_argument(arg_index);

  f_kernel->set_argument(arg_index, arg_size, arg_value);

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

