#ifndef __DEVICE_H
#define __DEVICE_H

#include <memory>
#include <vector>

// #include <hw/fpga.hpp>
#include <hw/devices.hpp>

#include "object.h"
#include "platform.h"

#include <buffer.hpp>
#include <funky_msg.h>

namespace funkycl {
#define FUNKY_VFPGA_ID 1
#define FUNKY_MSG_QUEUE_MAX_ELEMS 128

class device : public _cl_device_id
{
private:
  platform* m_platform;
  // hw::FPGA* vfpga; // = hw::Devices::fpga(0);

  // TODO: FunkyCL command class
  std::unique_ptr<buffer::Reader<funky_msg::response>> response_q;
  std::unique_ptr<buffer::Writer<funky_msg::request>> request_q;

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

