#include "config.h"
#include <CL/opencl.h>
#include "object/object.h"
#include "object/context.h"
#include "object/program.h"


namespace funkycl {

static cl_int
clBuildProgram(cl_program                     program,
                          cl_uint                        num_devices,
                          const cl_device_id *           device_list,
                          const char *                   options,
                          void                           (CL_CALLBACK * pfn_notify)(cl_program, void *),
                          void *                         user_data)
{
  return 0;
}

} // funkycl

CL_API_ENTRY cl_int CL_API_CALL
clBuildProgram(cl_program                     program,
                          cl_uint                        num_devices,
                          const cl_device_id *           device_list,
                          const char *                   options,
                          void                           (CL_CALLBACK * pfn_notify)(cl_program, void *),
                          void *                         user_data) CL_API_SUFFIX__VERSION_1_0
{
  return funkycl::clBuildProgram
    (program, num_devices, device_list, options, pfn_notify, user_data);
}
