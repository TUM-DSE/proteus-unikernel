#include "config.h"
#include <CL/opencl.h>
#include "object/object.h"
#include "object/device.h"
#include "object/kernel.h"
#include "object/cmd_queue.h"

#include <iostream>

namespace funkycl {

static cl_int
clEnqueueReadBuffer(cl_command_queue    command_queue,
                    cl_mem              buffer,
                    cl_bool             blocking_read,
                    size_t              offset,
                    size_t              size,
                    void *              ptr,
                    cl_uint             num_events_in_wait_list,
                    const cl_event *    event_wait_list,
                    cl_event *          event)
{
  auto cmd_queue = cl_to_funkycl(command_queue);

  /* send a MEMORY request every time when this function is called
   * TODO: send the request only if any memobj has been newly created 
   *       since the last time this function called 
   * */
  auto ret = cmd_queue->vfpga_send_memory_request();
  if(ret)
    DEBUG_STREAM("Memory creation request has been issued.");
  else
    DEBUG_STREAM("No memory request is issued. all memobjs are already initialized.");

  /* send a "TRANSFER" request to backend */
  bool is_write = false;
  cmd_queue->vfpga_send_transfer_request(cmd_queue->get_id(), 1, &buffer, blocking_read, offset, size, ptr, is_write);

  /* if blocking_read is true, wait until the buffer data has been read and copied into host memory (ptr) */
  if(blocking_read == CL_TRUE)
    clFinish(command_queue);

  return CL_SUCCESS;
}

} // funkycl


extern CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReadBuffer(cl_command_queue    command_queue,
                    cl_mem              buffer,
                    cl_bool             blocking_read,
                    size_t              offset,
                    size_t              size,
                    void *              ptr,
                    cl_uint             num_events_in_wait_list,
                    const cl_event *    event_wait_list,
                    cl_event *          event) CL_API_SUFFIX__VERSION_1_0
{
  return funkycl::clEnqueueReadBuffer
    (command_queue, buffer, blocking_read, offset, size, ptr, num_events_in_wait_list, event_wait_list, event);
}

