#ifndef __DEVICE_H
#define __DEVICE_H

#include <memory>
#include <vector>

// #include <hw/fpga.hpp>
#include <hw/devices.hpp>

#include "object.h"
#include "platform.h"

#include <buffer.hpp>

namespace funkycl {
#define FUNKY_VFPGA_ID 1

class device : public _cl_device_id
{
private:
  platform* m_platform;
  // hw::FPGA* vfpga; // = hw::Devices::fpga(0);

  // TODO: FunkyCL command class
  std::unique_ptr<buffer::Reader<int>> response_q;
  std::unique_ptr<buffer::Writer<int>> request_q;

public:
  device(platform* pltf);
  ~device();

  // TODO: add a method to invoke hypercalls with vfpga 
  void init_vfpga_backend(std::vector<unsigned char>& bitstream);
  
};

// device* get_device();
//



} // namespace funkycl


#endif // __DEVICE_H

