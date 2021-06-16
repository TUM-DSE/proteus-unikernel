#ifndef __PROGRAM_H
#define __PROGRAM_H

#include <memory>
#include <vector>
#include <map>

#include "object.h"
#include "platform.h"
#include "device.h"
#include "context.h"

#include "config.h"

namespace funkycl {
#define FUNKY_VFPGA_ID 1

class program : public _cl_program
{
private:
  context* m_context;
  std::vector<device*> m_devices;

  std::map<const device*,std::vector<unsigned char>> m_binaries; // multiple binaries can be loaded
  std::map<const device*,std::string> m_options;

  // TODO: implement clCreateProgramWithSource()
  // std::string m_source;

public:
  program(context* cntx, cl_uint num_devices, const cl_device_id* devices,
        const unsigned char** binaries, const size_t* lengths);
  ~program();
};

} // namespace funkycl

#endif // __PROGRAM_H

