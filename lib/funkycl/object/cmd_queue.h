#ifndef __CMD_QUEUE_H
#define __CMD_QUEUE_H

#include <memory>
#include <vector>

#include "object.h"
#include "refcount.h"
#include "device.h"
#include "context.h"
#include "memory.h"
#include "kernel.h"

namespace funkycl {
#define FUNKY_VFPGA_ID 1

class cmd_queue : public _cl_command_queue, public refcount
{
public:
  cmd_queue(context* context, device* device, cl_command_queue_properties props);
  ~cmd_queue();

  unsigned int
  get_id() const
  {
    return m_id;
  }

  device* get_device();

  /* for managing vfpga request/response queues */
  bool vfpga_send_memory_request();
  bool vfpga_send_transfer_request(cl_uint, const cl_mem*, cl_mem_migration_flags, cl_uint);
  bool vfpga_send_exec_request(cl_kernel, cl_uint);

private:
  unsigned int m_id=0;
  // device* m_device;
  ptr<device> m_device;
  // context* m_context;
  ptr<context> m_context;
  cl_command_queue_properties m_props;

  /* for vfpga TRANSFER requests */
  std::vector<std::unique_ptr<std::vector<int>>> trans_memids_list; 
  std::vector<std::unique_ptr<funky_msg::transfer_info>> trans_info_list; 

  /* for vfpga EXECUTE requests */
  using exec_args_info_type = std::vector<funky_msg::arg_info>;
  std::vector<std::unique_ptr<exec_args_info_type>> exec_args_list; 

};

} // namespace funkycl


#endif // __CMD_QUEUE_H

