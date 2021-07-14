#include "event.h"

namespace funkycl {

event::
event(cmd_queue* cmdq, context* cntx, cl_command_type cmd)
  : m_context(cntx), m_cmd_queue(cmdq), m_cmd_type(cmd)
{
  static unsigned int id_count = 0;
  m_id = id_count++;

  DEBUG_STREAM("create event obj [" << m_id << "]");
}

event::~event()
{
  DEBUG_STREAM("destroy event obj [" << m_id << "]");
}

} // funkycl
