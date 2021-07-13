#ifndef __EVENT_H
#define __EVENT_H

#include <memory>
#include <vector>

#include "object.h"

namespace funkycl {

class event : public _cl_event
{
public:
  event(cmd_queue* cmd_queue, context* cntx, cl_command_type cmd);
  // event(cmd_queue* cq, context* ctx, cl_command_type cmd, cl_uint num_deps, const cl_event* deps);
  virtual ~event();

private:
  unsigned int m_id = 0;
  context* m_context;
  cmd_queue* m_command_queue;
  cl_command_type m_command_type = 0;
};


} // funkycl

#endif // __EVENT_H


