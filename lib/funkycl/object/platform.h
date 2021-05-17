#ifndef __PLATFORM_H
#define __PLATFORM_H

#include <memory>
#include <vector>

#include "object.h"

namespace funkycl {
#define FUNKY_VFPGA_ID 1

class platform : public _cl_platform_id
{
public:
  platform();
  ~platform();

  static std::shared_ptr<platform>
  get_shared_platform();

};

platform*
get_global_platform();

} // namespace funkycl



#endif // __PLATFORM_H

