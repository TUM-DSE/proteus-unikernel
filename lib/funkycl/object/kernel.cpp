/**
 * Copyright (C) 2016-2017 Xilinx, Inc
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You may
 * not use this file except in compliance with the License. A copy of the
 * License is located at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include "device.h"
#include "context.h"
#include "program.h"
#include "kernel.h"

#include <iostream>
#include <memory>
#include <stdexcept>

namespace funkycl {

void
kernel::clmem_argument::
set(size_t size, const void* arg)
{
  // if (size != sizeof(cl_mem))
  //   throw error(CL_INVALID_ARG_SIZE,"Invalid global_argument size for kernel arg");

  auto value = const_cast<void*>(arg);
  auto mem = value ? *static_cast<cl_mem*>(value) : nullptr;

  m_buffer = cl_to_funkycl(mem);
  m_set = true;
}

kernel::
kernel(program* prog, const std::string& name)
  : m_program(prog), m_name(name)
{
  static unsigned int id_count = 0;
  m_id = id_count++;
}

kernel::
~kernel()
{
  std::cout << "xocl::kernel::~kernel(" << m_id << std::endl;
}

// std::unique_ptr<kernel::argument>
void
kernel::create_argument(unsigned long idx)
{
  // FIXME: here is just for initial test: only available for vadd.xclbin (hello world).
  if(idx == 3)
    m_args.emplace_back(std::make_unique<kernel::clmem_argument>(this, idx));
  else
    m_args.emplace_back(std::make_unique<kernel::scalar_argument>(this, idx));
}

void
kernel::set_argument(unsigned long idx, size_t size, const void* arg)
{
  // FIXME: here is just for initial test: only available for vadd.xclbin (hello world).
  if(idx == 3)
    m_args.at(idx)->set(idx, size, arg);
  else
    m_args.at(idx)->set(idx, size, arg);
}

context*
kernel::
get_context() const
{
  return m_program->get_context();
}

} // xocl

