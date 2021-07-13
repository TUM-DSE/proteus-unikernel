#include "config.h"
#include <CL/opencl.h>
#include "object/object.h"
#include "object/memory.h"

namespace funkycl {

static cl_int
clReleaseMemObject(cl_mem memobj)
{
  // if (cl_to_funkycl(memobj)->release()) // release() cannot be called?
  delete cl_to_funkycl(memobj);

  return CL_SUCCESS;
}

} // funkycl

CL_API_ENTRY cl_int CL_API_CALL
clReleaseMemObject(cl_mem memobj) CL_API_SUFFIX__VERSION_1_0
{
  return funkycl::clReleaseMemObject(memobj);
}



