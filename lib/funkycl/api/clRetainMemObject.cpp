#include "config.h"
#include <CL/opencl.h>
#include "object/object.h"
#include "object/memory.h"

namespace funkycl {

static cl_int
clRetainMemObject(cl_mem memobj)
{
  /* Nothing is done here because we haven't implemented any reference count. */
  // TODO: implement refcount
  DEBUG_PRINT("debug");
  return CL_SUCCESS;
}

} // funkycl

CL_API_ENTRY cl_int CL_API_CALL
clRetainMemObject(cl_mem memobj) CL_API_SUFFIX__VERSION_1_0
{
  return funkycl::clRetainMemObject(memobj);
}
