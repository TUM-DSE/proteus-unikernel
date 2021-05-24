#ifndef __CMD_QUEUE_H
#define __CMD_QUEUE_H

#include <memory>
#include <vector>

#include "object.h"
#include "device.h"
#include "context.h"

namespace funkycl {
#define FUNKY_VFPGA_ID 1

class cmd_queue : public _cl_command_queue
{
public:
  cmd_queue(context* context, device* device, cl_command_queue_properties props);
  ~cmd_queue();


private:
  device* m_device;
  context* m_context;
  cl_command_queue_properties m_props;
  

};

} // namespace funkycl


#endif // __CMD_QUEUE_H

