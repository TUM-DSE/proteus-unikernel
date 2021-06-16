#include "program.h"

namespace funkycl
{

program::
program(context* cntx, cl_uint num_devices, const cl_device_id* devices,
        const unsigned char** binaries, const size_t* lengths)
  : m_context(cntx)
{
  for (cl_uint i=0; i < num_devices; i++) {
    auto device = funkycl::cl_to_funkycl(devices[i]);
    m_devices.push_back(device);
    m_binaries.emplace(device, std::vector<unsigned char>{binaries[i], binaries[i] + lengths[i]});

    // TODO: send a reconf request to ukvm
    auto binary = m_binaries.find(device)->second;
    device->init_vfpga_backend(binary);
  }
}

program::
~program()
{
  // TODO: release something? 
}


} // funkycl


