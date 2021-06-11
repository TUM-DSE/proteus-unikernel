#include "device.h"

namespace funkycl
{

device::device(platform* pltf) 
  : response_q(std::make_unique<buffer::Reader<int>>(100)),
    request_q(std::make_unique<buffer::Writer<int>>(100))
{
  m_platform = pltf;
  auto& vfpga = hw::Devices::fpga(0);

  printf("INFO: try to fpga_init...\n");

  std::vector<uint8_t> dummy_bs = {0xde, 0xad, 0xbe, 0xef};

  request_q->push(12345);

  vfpga.init(dummy_bs.data(), dummy_bs.size(),
      request_q->get_baseaddr(), request_q->get_mmsize(),
      response_q->get_baseaddr(), response_q->get_mmsize());

  // test a buffer read/write
  auto out = response_q->pop();
  while(out == NULL) // buffer is empty
    out = response_q->pop();

  printf("INFO: read %d from ukvm.\n", *out);

  printf("INFO: fpga_init is done.\n");
}

device::~device()
{
  // TODO: release cl_device objects
}


} // funkycl


