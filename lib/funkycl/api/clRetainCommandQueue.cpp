#include "config.h"
#include <CL/opencl.h>
#include "object/object.h"
#include "object/memory.h"

namespace funkycl {

static cl_int
clRetainCommandQueue(cl_command_queue command_queue)
{
  /* Nothing is done here because we haven't implemented any reference count. */
  // TODO: implement refcount
  return CL_SUCCESS;
}

} // funkycl

CL_API_ENTRY cl_int CL_API_CALL
clRetainCommandQueue(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0
{
  return funkycl::clRetainCommandQueue(command_queue);
}

