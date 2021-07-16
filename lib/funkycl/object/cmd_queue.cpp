#include "cmd_queue.h"

namespace funkycl
{

cmd_queue::cmd_queue(context* context, device* device, cl_command_queue_properties props) 
  : m_context(context), m_device(device), m_props(props)
{
  static unsigned int id_count = 0;
  m_id = id_count++;

  DEBUG_STREAM("create cmd_queue obj [" << m_id << "]");
}

cmd_queue::~cmd_queue()
{
  DEBUG_STREAM("destroy cmd_queue obj [" << m_id << "]");
}

device*
cmd_queue::get_device()
{
  return m_device.get();
}

bool 
cmd_queue::vfpga_send_memory_request()
{
  auto context = m_context.get();
  auto device  = m_device.get();

  if(!context->is_meminfo_list_updated())
    return false;

  funky_msg::request memory_req(funky_msg::MEMORY, context->get_all_meminfo_size(), context->load_all_meminfo_addr());
  device->vfpga_send_request(memory_req);

  return true;
}

bool 
cmd_queue::vfpga_send_transfer_request(cl_uint num_mem_objects, const cl_mem* mem_objects, cl_mem_migration_flags flags)
{
  /* identify memory objects to be transferred */
  std::vector<int> memids;
  for (cl_uint i=0; i<num_mem_objects; i++)
  {
    auto mem = cl_to_funkycl(mem_objects[i]);
    memids.emplace_back(mem->get_id());
    DEBUG_STREAM("memid[" << i << "]:" << memids.back());
  }
  trans_memids_list.emplace_back(std::make_unique<std::vector<int>>(memids));

  /* create request info */
  auto trans_memids = trans_memids_list.back().get();
  DEBUG_STREAM("trans_memids addr: " << &(trans_memids->back()) );

  trans_info_list.emplace_back(std::make_unique<funky_msg::transfer_info>(&(trans_memids->front()), trans_memids->size(), flags));
  auto trans_info = trans_info_list.back().get();
  DEBUG_STREAM("Create a new transfer request info. addr: " << trans_info->ids << ", num: " << trans_info->num << ", flags: " << trans_info->flags);

  /* send a TRANSFER request */
  funky_msg::request transfer_req(funky_msg::TRANSFER, (void *)(trans_info));
  auto device  = m_device.get();
  device->vfpga_send_request(transfer_req);

  return true;
}

} // funkycl


