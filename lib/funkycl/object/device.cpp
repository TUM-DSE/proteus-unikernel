#include "platform.h"
#include "device.h"

namespace funkycl
{


device::device(funkycl::platform* pltf) 
{
  platform = pltf;
  vfpga = &hw::Devices::fpga(0);
}

device::~device()
{
  // TODO: release cl_device objects
}


} // funkycl


