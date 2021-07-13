#include "config.h"
#include <CL/opencl.h>
#include "object/object.h"
#include "object/device.h"
#include "object/memory.h"

namespace funkycl {

/* Memory Object APIs */
static cl_mem 
clCreateBuffer(cl_context   context,
               cl_mem_flags flags,
               size_t       size,
               void *       host_ptr,
               cl_int *     errcode_ret)
{
  auto buffer =
    std::make_unique<funkycl::buffer>(cl_to_funkycl(context), flags, size, host_ptr);

  auto device = cl_to_funkycl(context)->get_device();

  // funky_msg::mem_info  minfo_in1(1, funky_msg::BUFFER, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,  source_in1, DATA_SIZE*sizeof(int));
  // funky_msg::mem_info  minfo_in2(2, funky_msg::BUFFER, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,  source_in2, DATA_SIZE*sizeof(int));
  // funky_msg::mem_info  minfo_out(3, funky_msg::BUFFER, CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, hw_results, DATA_SIZE*sizeof(int));

  // funky_msg::mem_info* mems[] = {&minfo_in1, &minfo_in2, &minfo_out};
  // funky_msg::request dummy_memory_req(funky_msg::MEMORY, 3, (void **)mems);
  // request_q->push(dummy_memory_req);

  /* send a MEMORY request */
  // TODO: obtain an index of buffer 
  static unsigned int index=1;

  funky_msg::mem_info  minfo(index, funky_msg::BUFFER, flags,  host_ptr, size);
  funky_msg::mem_info* mems[] = {&minfo};
  funky_msg::request memory_req(funky_msg::MEMORY, 3, (void **)mems);
  device->vfpga_send_request(memory_req);
  index++;

  if(errcode_ret)
    *errcode_ret = CL_SUCCESS;

  return buffer.release();
}

} // funkycl


CL_API_ENTRY cl_mem CL_API_CALL
clCreateBuffer(cl_context   context,
               cl_mem_flags flags,
               size_t       size,
               void *       host_ptr,
               cl_int *     errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
  return funkycl::clCreateBuffer
    (context, flags, size, host_ptr, errcode_ret);
}


