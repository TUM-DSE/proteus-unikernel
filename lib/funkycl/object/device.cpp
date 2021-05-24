#include "device.h"

namespace funkycl
{


device::device(platform* pltf) 
{
  m_platform = pltf;
  vfpga = &hw::Devices::fpga(0);
}

device::~device()
{
  // TODO: release cl_device objects
}


} // funkycl


