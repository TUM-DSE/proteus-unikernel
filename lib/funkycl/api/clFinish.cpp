#include "config.h"
#include <CL/opencl.h>
#include "object/object.h"
#include "object/device.h"
#include "object/cmd_queue.h"

#include <iostream>

namespace funkycl {

static cl_int 
clFinish(cl_command_queue command_queue)
{
  auto device = cl_to_funkycl(command_queue)->get_device();

  // TODO: send a SYNC request to backend and wait
  funky_msg::request dummy_sync_req(funky_msg::SYNC);
  device->vfpga_send_request(dummy_sync_req);

  std::cout << "TBD: do a hypercall for clFinish() !!" << std::endl; 

  return CL_SUCCESS;
}

} // funkycl


CL_API_ENTRY cl_int CL_API_CALL
clFinish(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0
{
  return funkycl::clFinish(command_queue);
}

