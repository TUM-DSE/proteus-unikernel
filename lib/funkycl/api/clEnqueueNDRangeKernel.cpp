#include "config.h"
#include <CL/opencl.h>
#include "object/object.h"
#include "object/device.h"
#include "object/kernel.h"
#include "object/cmd_queue.h"

namespace funkycl {

/* Kernel Object APIs */
static cl_int
clEnqueueNDRangeKernel(cl_command_queue command_queue,
                       cl_kernel        kernel,
                       cl_uint          work_dim,
                       const size_t *   global_work_offset,
                       const size_t *   global_work_size,
                       const size_t *   local_work_size,
                       cl_uint          num_events_in_wait_list,
                       const cl_event * event_wait_list,
                       cl_event *       event)
{
  auto cmd_queue = cl_to_funkycl(command_queue);

  if(work_dim != 1)
  {
    std::cout << "Error: Funky does not support the work dimension greater than 1. Aborted. " << std::endl;
    return CL_FALSE;
  }

  if( (global_work_offset[0] != 0) || (global_work_size[0] != 1) || (local_work_size[0] != 1))
  {
    std::cout << "Error: Funky only supports {g_work_offset, g_work_size, l_work_size} = {0, 1, 1}. Aborted." << std::endl;
    return CL_FALSE;
  }

  cmd_queue->vfpga_send_exec_request(cmd_queue->get_id(), kernel);

  return CL_SUCCESS;
}

} // funkycl


CL_API_ENTRY cl_int CL_API_CALL
clEnqueueNDRangeKernel(cl_command_queue command_queue,
                       cl_kernel        kernel,
                       cl_uint          work_dim,
                       const size_t *   global_work_offset,
                       const size_t *   global_work_size,
                       const size_t *   local_work_size,
                       cl_uint          num_events_in_wait_list,
                       const cl_event * event_wait_list,
                       cl_event *       event) CL_API_SUFFIX__VERSION_1_0
{
  return funkycl::clEnqueueNDRangeKernel
    (command_queue, kernel, work_dim, global_work_offset, global_work_size, local_work_size, num_events_in_wait_list, event_wait_list, event);
}




