#include "program.h"

namespace funkycl
{

  /** 
   * The constructor sends an init request to backend (ukvm) 
   *
   * OpenCL allows a context to have multiple program objects. 
   * However, the current version of FunkyCL supports that only a single program object is assigned to each device. 
   * It means a single program object contains all the kernels used by a guest application on the device. 
   *
   * */
program::
program(context* cntx, cl_uint num_devices, const cl_device_id* devices,
        const unsigned char** binaries, const size_t* lengths)
  : m_context(cntx)
{
  // TODO: check if the device (backend) is already initialized by another program.  
  for (cl_uint i=0; i < num_devices; i++) {
    auto device = funkycl::cl_to_funkycl(devices[i]);
    m_devices.push_back(device);
    m_binaries.emplace(device, std::vector<unsigned char>{binaries[i], binaries[i] + lengths[i]});

    /* The device (vFPGA) is initialized only once unless it is freed. */
    if(!device->is_initialized()) {
      auto binary = m_binaries.find(device)->second;
      device->init_vfpga(binary);
    }

    // TODO: if the device is already initialized but this program context is instanciated with different bitstream, notify to the backend
    //      (for future extension, which allows a single guest app to manage multiple programs)
  }
}

program::
~program()
{
  DEBUG_PRINT("debug");

  /* notify the backend to release FPGA */
  for (auto device : m_devices)
  {
    if(device->is_initialized())
      device->free_vfpga();
  }
}

context* 
program::get_context()
{
  return m_context; 
}

} // funkycl


