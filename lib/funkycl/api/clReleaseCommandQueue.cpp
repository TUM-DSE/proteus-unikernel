#include "config.h"
#include <CL/opencl.h>
#include "object/object.h"
#include "object/cmd_queue.h"

namespace funkycl {

static cl_int
clReleaseCommandQueue(cl_command_queue cmd_queue)
{
  // if (cl_to_funkycl(cmd_queue)->release()) // release() is not implemented
  delete cl_to_funkycl(cmd_queue);

  return CL_SUCCESS;
}

} // funkycl

cl_int
clReleaseCommandQueue(cl_command_queue cmd_queue)
{
  return funkycl::clReleaseCommandQueue(cmd_queue);
}


