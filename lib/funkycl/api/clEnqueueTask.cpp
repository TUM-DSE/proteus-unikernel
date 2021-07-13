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
  // auto context = cl_to_funkycl(kernel)->get_context();
  // auto device = context->get_device();

  auto device = cl_to_funkycl(command_queue)->get_device();
  
  // TODO: read arguments from arg
  int size = 4096;
  funky_msg::arg_info arg0(0, 1);
  funky_msg::arg_info arg1(1, 2);
  funky_msg::arg_info arg2(2, 3);
  funky_msg::arg_info arg3(3, -1, &size, sizeof(size));

  // TODO: send an "EXECUTE" request to backend
  funky_msg::arg_info* args[] = {&arg0, &arg1, &arg2, &arg3};

  std::string dummy_kernel("vadd");
  funky_msg::request dummy_exec_req(funky_msg::EXECUTE, dummy_kernel.c_str(), dummy_kernel.length(), 4, (void **)args);
  device->vfpga_send_request(dummy_exec_req);

  // TODO: call clEnqueueNDRangeKernel() here. clEnqueueTask() is just a wrapper

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




