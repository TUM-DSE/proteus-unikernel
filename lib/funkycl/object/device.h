#ifndef __DEVICE_H
#define __DEVICE_H

#include <memory>
#include <vector>

// #include <hw/fpga.hpp>
#include <hw/devices.hpp>

#include "object.h"
#include "platform.h"

namespace funkycl {
#define FUNKY_VFPGA_ID 1

class device : public _cl_device_id
{
public:
  device(funkycl::platform* pltf);
  ~device();


private:
  platform* platform;
  hw::FPGA* vfpga; // = hw::Devices::fpga(0);

};

// device* get_device();

// TODO: add a method to invoke hypercalls with vfpga 

} // namespace funkycl


#endif // __DEVICE_H

