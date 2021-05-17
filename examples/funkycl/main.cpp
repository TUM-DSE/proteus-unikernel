// This file is a part of the IncludeOS unikernel - www.includeos.org
//
// Copyright 2015 Oslo and Akershus University College of Applied Sciences
// and Alfred Bratterud
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <os>
#include <sstream>
#include <cstdio>
#include <string>
#include <cassert>

#include <vector>


#define CL_TARGET_OPENCL_VERSION 120
// #include <hw/fpga.hpp>
#include <CL/opencl.h>

int main()
{
  cl_int status;
  
  // get number of platforms
  cl_uint num_platforms;
  status = clGetPlatformIDs(0, NULL, &num_platforms);

  // get a list of all platform ids
  std::vector<cl_platform_id> pids(num_platforms);
  status = clGetPlatformIDs(num_platforms, pids.data(), NULL);

  // get the name
  for (auto it = pids.begin(); it != pids.end(); it++)
  {
    // get platform name size
    size_t psize;
    status = clGetPlatformInfo(*it, CL_PLATFORM_NAME, 0, NULL, &psize);

    // get platform name
    std::vector<char> pname(psize);
    status = clGetPlatformInfo(*it, CL_PLATFORM_NAME, psize, (void *)pname.data(), NULL);

    std::cout << "Platform name: " << pname.data() << std::endl;
  }

  return 0;
}
