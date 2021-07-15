#include "device.h"

#include <iostream>
#include <vector>
#include <cstdlib>

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

  // data for cmd_queue test (vadd)
#define DATA_SIZE 4096
  int* sw_results = static_cast<int*>(std::aligned_alloc(4096, DATA_SIZE*sizeof(int)));
  int* hw_results = static_cast<int*>(std::aligned_alloc(4096, DATA_SIZE*sizeof(int)));

  // function for cmd_queue test (vadd)
  auto test_send_requests = [&, this]()
  {
    int* source_in1 = static_cast<int*>(std::aligned_alloc(4096, DATA_SIZE*sizeof(int)));
    int* source_in2 = static_cast<int*>(std::aligned_alloc(4096, DATA_SIZE*sizeof(int)));

    /* initialize input data */
    std::generate(source_in1, &source_in1[DATA_SIZE], std::rand);
    std::generate(source_in2, &source_in2[DATA_SIZE], std::rand);

    for (int i = 0; i < DATA_SIZE; i++) {
      sw_results[i] = source_in1[i] + source_in2[i];
      hw_results[i] = 0;
    }

    /* send a MEMORY request */
    funky_msg::mem_info  minfo_in1(1, funky_msg::BUFFER, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,  source_in1, DATA_SIZE*sizeof(int));
    funky_msg::mem_info  minfo_in2(2, funky_msg::BUFFER, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,  source_in2, DATA_SIZE*sizeof(int));
    funky_msg::mem_info  minfo_out(3, funky_msg::BUFFER, CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, hw_results, DATA_SIZE*sizeof(int));

    funky_msg::mem_info* mems[] = {&minfo_in1, &minfo_in2, &minfo_out};
    funky_msg::request dummy_memory_req(funky_msg::MEMORY, 3, (void **)mems);
    request_q->push(dummy_memory_req);

    /* send a TRANSFER request */
    int in_mem_ids[] = {1, 2};
    funky_msg::transfer_info trans_input(in_mem_ids, 2, 0);
    funky_msg::request transfer_input_req(funky_msg::TRANSFER, (void *)&trans_input);
    request_q->push(transfer_input_req);

    /* send an EXEC request */
    int size = DATA_SIZE;
    funky_msg::arg_info arg0(0, minfo_in1.id);
    funky_msg::arg_info arg1(1, minfo_in2.id);
    funky_msg::arg_info arg2(2, minfo_out.id);
    funky_msg::arg_info arg3(3, -1, &size, sizeof(size));
    funky_msg::arg_info* args[] = {&arg0, &arg1, &arg2, &arg3};

    std::string dummy_kernel("vadd");
    funky_msg::request dummy_exec_req(funky_msg::EXECUTE, dummy_kernel.c_str(), dummy_kernel.length(), 4, (void **)args);
    request_q->push(dummy_exec_req);

    /* send a TRANSFER request */
    int out_mem_ids[] = {3};
    funky_msg::transfer_info trans_output(out_mem_ids, 1, CL_MIGRATE_MEM_OBJECT_HOST);
    funky_msg::request transfer_output_req(funky_msg::TRANSFER, (void *)&trans_output);
    request_q->push(transfer_output_req);

    /* send a SYNC request */
    funky_msg::request dummy_sync_req(funky_msg::SYNC);
    request_q->push(dummy_sync_req);

    return;
  };

  // function for cmd_queue test (vadd)
  auto test_wait_for_response = [&, this]()
  {
    /* receive a dummy SYNC response */
    auto res = response_q->pop();
    while(res == NULL) // buffer is empty
      res = response_q->pop();

    if(res->get_response_type() == funky_msg::SYNC)
      std::cout << "GUEST: sync is done." << std::endl;

    // Compare the results of the Device to the simulation
    bool match = true;
    for (int i = 0; i < DATA_SIZE; i++) {
      if (hw_results[i] != sw_results[i]) {
        std::cout << "Error: Result mismatch" << std::endl;
        std::cout << "i = " << i << " CPU result = " << sw_results[i]
          << " Device result = " << hw_results[i] << std::endl;
        match = false;
        break;
      }
    }

    std::cout << "TEST " << (match ? "PASSED" : "FAILED") << std::endl;
    return; 
  };

  /* test: send requests to execute vadd on FPGA */
  // test_send_requests();

  std::cout << "GUEST: sending fpga_init hypercall request..." << std::endl;

  /* do a hypercall */
  vfpga.init(bitstream.data(), bitstream.size(),
      request_q->get_baseaddr(), request_q->get_mmsize(),
      response_q->get_baseaddr(), response_q->get_mmsize());

  /* test: check the output results */
  // test_wait_for_response();

  init_flag = true;
  return;
}

void 
device::free_vfpga()
{
  // TODO: do a hypercall to release FPGA
  DEBUG_PRINT("TBD: free_vfpga()!!");

  init_flag = false;
  return;
}

bool 
device::is_initialized(void)
{
  return init_flag;
}

bool 
device::vfpga_send_request(funky_msg::request& req)
{
  DEBUG_STREAM("req: " << req.get_request_type() << ", addr:" << &req);

  if (req.get_request_type() == funky_msg::MEMORY)
  {
    int num;
    auto mems = (funky_msg::mem_info**) req.get_meminfo_array(num);
    auto mem = (funky_msg::mem_info*) mems[0];
    DEBUG_STREAM("Before pushing Memreq: addr=" << mem << ", num=" << num << ", index=" << mem->id << ", MemType=" << mem->type << ", flags=" << mem->flags << ", host_ptr=" << mem->src << ", size=" << mem->size);

  }


  return request_q->push(req);
}

funky_msg::response* 
device::vfpga_get_response()
{
  return response_q->pop();
}

int 
device::vfpga_handle_requests()
{
  auto& vfpga = hw::Devices::fpga(0);

  /* do a hypercall */
  return vfpga.handle_requests();
}

device::~device()
{
  // TODO: release cl_device objects
}


} // funkycl


