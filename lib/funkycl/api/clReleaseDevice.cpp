#include "config.h"
#include <CL/opencl.h>
#include "object/object.h"
#include "object/device.h"

namespace funkycl {

static cl_int
clReleaseDevice(cl_device_id device)
{
  // TODO: delete device?
  return CL_SUCCESS;
}

} // funkycl

cl_int
clReleaseDevice(cl_device_id device)
{
  return funkycl::clReleaseDevice(device);
}


