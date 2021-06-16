#include "device.h"

namespace funkycl
{

device::device(platform* pltf) 
  : response_q(std::make_unique<buffer::Reader<int>>(100)),
    request_q(std::make_unique<buffer::Writer<int>>(100))
{
  m_platform = pltf;
}

void device::init_vfpga_backend(std::vector<unsigned char>& bitstream)
{
  auto& vfpga = hw::Devices::fpga(0);

  // sending a dummy request
  request_q->push(12345);

  // std::vector<uint8_t> dummy_bs = {0xde, 0xad, 0xbe, 0xef};
  // vfpga.init(dummy_bs.data(), dummy_bs.size(),
  vfpga.init(bitstream.data(), bitstream.size(),
      request_q->get_baseaddr(), request_q->get_mmsize(),
      response_q->get_baseaddr(), response_q->get_mmsize());

  // sending a dummy request
  auto out = response_q->pop();
  while(out == NULL) // buffer is empty
    out = response_q->pop();

  printf("INFO: read %d from ukvm.\n", *out);
}

device::~device()
{
  // TODO: release cl_device objects
}


} // funkycl


