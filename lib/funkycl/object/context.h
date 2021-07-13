#ifndef __CONTEXT_H
#define __CONTEXT_H

#include <memory>
#include <vector>

#include "config.h"
#include <CL/opencl.h>

#include "object.h"
#include "device.h"

namespace funkycl {
#define FUNKY_VFPGA_ID 1

class context : public _cl_context
{
public:
  context(const cl_context_properties* properties
      ,size_t num_devices 
      ,const cl_device_id* devices
      );
      // ,const notify_action& notify=notify_action());

  context(platform* pltf);
  ~context();

  device*
  get_first_device() const
  {
    return (m_devices.size()==1)? m_devices[0]: nullptr;
  }

  device*
  get_device() const;

private:
  const cl_context_properties* m_props;
  std::vector<device*> m_devices;
};

} // namespace funkycl


#endif // __CONTEXT_H

