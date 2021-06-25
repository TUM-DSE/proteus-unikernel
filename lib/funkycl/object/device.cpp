#include "device.h"

namespace funkycl
{

device::device(platform* pltf) 
  : response_q(std::make_unique<buffer::Reader<funky_msg::response>>(FUNKY_MSG_QUEUE_MAX_ELEMS)),
    request_q(std::make_unique<buffer::Writer<funky_msg::request>>(FUNKY_MSG_QUEUE_MAX_ELEMS))
{
  m_platform = pltf;
}

void device::init_vfpga_backend(std::vector<unsigned char>& bitstream)
{
  auto& vfpga = hw::Devices::fpga(0);

  /* send a TRANSFER request */
  uint32_t data1=10, data2=20;
  void* ptrs[] = {&data1, &data2};

  funky_msg::request dummy_transfer_req(funky_msg::TRANSFER, 2, ptrs);

  // sending a dummy request
  // request_q->push(12345);
  request_q->push(dummy_transfer_req);

  /* send an EXEC request */
  std::string dummy_kernel("i_am_kernel");
  funky_msg::request dummy_exec_req(funky_msg::EXEC, dummy_kernel.c_str(), 0, NULL);

  request_q->push(dummy_exec_req);

  // std::vector<uint8_t> dummy_bs = {0xde, 0xad, 0xbe, 0xef};
  // vfpga.init(dummy_bs.data(), dummy_bs.size(),
  vfpga.init(bitstream.data(), bitstream.size(),
      request_q->get_baseaddr(), request_q->get_mmsize(),
      response_q->get_baseaddr(), response_q->get_mmsize());

  // sending a dummy request
  // auto out = response_q->pop();
  // while(out == NULL) // buffer is empty
  //   out = response_q->pop();

  // printf("INFO: read %d from ukvm.\n", *out);
}

device::~device()
{
  // TODO: release cl_device objects
}


} // funkycl


