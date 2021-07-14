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

} // funkycl


