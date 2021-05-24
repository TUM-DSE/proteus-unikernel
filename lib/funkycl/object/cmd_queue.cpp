#include "cmd_queue.h"

namespace funkycl
{


cmd_queue::cmd_queue(context* context, device* device, cl_command_queue_properties props) 
  : m_context(context), m_device(device), m_props(props)
{}

cmd_queue::~cmd_queue()
{}


} // funkycl


