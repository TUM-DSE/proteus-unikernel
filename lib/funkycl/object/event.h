#ifndef __EVENT_H
#define __EVENT_H

#include <memory>
#include <vector>

#include "object.h"
#include "refcount.h"

#include "context.h"
#include "cmd_queue.h"

namespace funkycl {

class event : public _cl_event, public refcount
{
public:
  event(cmd_queue* cmd_queue, context* cntx, cl_command_type cmd);
  // event(cmd_queue* cq, context* ctx, cl_command_type cmd, cl_uint num_deps, const cl_event* deps);
  virtual ~event();

private:
  unsigned int m_id = 0;
  // context* m_context;
  ptr<context> m_context;
  ptr<cmd_queue> m_cmd_queue;
  cl_command_type m_cmd_type = 0;
};


} // funkycl

#endif // __EVENT_H


