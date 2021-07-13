#include "event.h"
#include <iostream>

namespace funkycl {

event::
event(cmd_queue* cmdq, context* cntx, cl_command_type cmd)
  : m_context(cntx), m_command_queue(cmdq), m_command_type(cmd)
{
  static unsigned int id_count = 0;
  m_id = id_count++;

  std::cout << "xocl::event::event(" << m_id << std::endl;
}

event::~event()
{}

} // funkycl
