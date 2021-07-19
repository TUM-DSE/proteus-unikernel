#include "config.h"
#include <CL/opencl.h>
#include "object/object.h"
#include "object/device.h"
#include "object/kernel.h"
#include "object/cmd_queue.h"

namespace funkycl {

/* Kernel Object APIs */
static cl_int
clEnqueueTask(cl_command_queue  command_queue,
              cl_kernel         kernel,
              cl_uint           num_events_in_wait_list,
              const cl_event *  event_wait_list,
              cl_event *        event)
{
  auto cmd_queue = cl_to_funkycl(command_queue);
  cmd_queue->vfpga_send_exec_request(kernel);

  return CL_SUCCESS;
}

} // funkycl


cl_int
clEnqueueTask(cl_command_queue  command_queue,
              cl_kernel         kernel,
              cl_uint           num_events_in_wait_list,
              const cl_event *  event_wait_list,
              cl_event *        event)
{
  return funkycl::clEnqueueTask
    (command_queue, kernel, num_events_in_wait_list, event_wait_list, event);
}




