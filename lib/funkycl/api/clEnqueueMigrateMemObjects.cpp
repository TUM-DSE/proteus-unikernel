#include "config.h"
#include <CL/opencl.h>
#include "object/object.h"
#include "object/device.h"
#include "object/kernel.h"
#include "object/cmd_queue.h"

#include <iostream>

namespace funkycl {

static cl_int
clEnqueueMigrateMemObjects(cl_command_queue       command_queue,
                           cl_uint                num_mem_objects,
                           const cl_mem *         mem_objects,
                           cl_mem_migration_flags flags,
                           cl_uint                num_events_in_wait_list,
                           const cl_event *       event_wait_list,
                           cl_event *             event)
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
  cmd_queue->vfpga_send_transfer_request(num_mem_objects, mem_objects, flags, cmd_queue->get_id());

  return CL_SUCCESS;
}

} // funkycl


CL_API_ENTRY cl_int CL_API_CALL
clEnqueueMigrateMemObjects(cl_command_queue       command_queue,
                           cl_uint                num_mem_objects,
                           const cl_mem *         mem_objects,
                           cl_mem_migration_flags flags,
                           cl_uint                num_events_in_wait_list,
                           const cl_event *       event_wait_list,
                           cl_event *             event) CL_API_SUFFIX__VERSION_1_2
{
  return funkycl::clEnqueueMigrateMemObjects
    (command_queue, num_mem_objects, mem_objects, flags, num_events_in_wait_list, event_wait_list, event);
}





