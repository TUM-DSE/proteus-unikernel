#include "device.h"

#include <iostream>

namespace funkycl
{

device::device(platform* pltf) 
  : response_q(std::make_unique<buffer::Reader<funky_msg::response>>(FUNKY_MSG_QUEUE_MAX_ELEMS)),
    request_q(std::make_unique<buffer::Writer<funky_msg::request>>(FUNKY_MSG_QUEUE_MAX_ELEMS)),
    init_flag(false)
{
  m_platform = pltf;
}

void device::init_vfpga(std::vector<unsigned char>& bitstream)
{
  auto& vfpga = hw::Devices::fpga(0);

  /* send a TRANSFER request */
  uint32_t data1=10, data2=20;
  void* ptrs[] = {&data1, &data2};

  funky_msg::request dummy_transfer_req(funky_msg::TRANSFER, 2, ptrs);
  request_q->push(dummy_transfer_req);

  /* send an EXEC request */
  // funky_msg::arg_info info;

  std::string dummy_kernel("i_am_kernel");
  funky_msg::request dummy_exec_req(funky_msg::EXEC, dummy_kernel.c_str(), 0, NULL);
  request_q->push(dummy_exec_req);

  /* send a SYNC request */
  funky_msg::request dummy_sync_req(funky_msg::SYNC);
  request_q->push(dummy_sync_req);

  /* do a hypercall */
  vfpga.init(bitstream.data(), bitstream.size(),
      request_q->get_baseaddr(), request_q->get_mmsize(),
      response_q->get_baseaddr(), response_q->get_mmsize());

  /* receive a dummy SYNC response */
  // auto res = response_q->pop();
  // while(res == NULL) // buffer is empty
  //   res = response_q->pop();

  init_flag = true;
  return;
}

void device::free_vfpga()
{
  // TODO: do a hypercall to release FPGA
  std::cout << "TBD: free_vfpga()!!!!!!" << std::endl;

  init_flag = false;
  return;
}

bool device::is_initialized(void)
{
  return init_flag;
}

device::~device()
{
  // TODO: release cl_device objects
}


} // funkycl


