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
  auto device = cl_to_funkycl(command_queue)->get_device();

  static bool is_first=true;

  // TODO: send a "TRANSFER" request to backend
  if(is_first) {
    /* send a TRANSFER request */
    int in_mem_ids[] = {1, 2};
    funky_msg::transfer_info trans_input(in_mem_ids, 2, 0);
    funky_msg::request transfer_input_req(funky_msg::TRANSFER, (void *)&trans_input);
    device->vfpga_send_request(transfer_input_req);
    is_first=false;
  }
  else {
    /* send a TRANSFER request */
    int out_mem_ids[] = {3};
    funky_msg::transfer_info trans_output(out_mem_ids, 1, CL_MIGRATE_MEM_OBJECT_HOST);
    funky_msg::request transfer_output_req(funky_msg::TRANSFER, (void *)&trans_output);
    device->vfpga_send_request(transfer_output_req);
  }

  DEBUG_PRINT("");

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





