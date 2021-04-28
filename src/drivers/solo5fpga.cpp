#define DEBUG
#define DEBUG2
#include "solo5fpga.hpp"

// #include <hw/pci.hpp>
// #include <fs/common.hpp>
#include <cassert>
#include <stdlib.h>

#include <statman>

extern "C" {
#include <solo5.h>
}

Solo5FPGA::Solo5FPGA()
  : hw::FPGA()
{
  // struct solo5_fpga_info fpgai;
  // solo5_fpga_info(&fpgai);
  INFO("Solo5FPGA", "Funky Virt FPGA");
}


int Solo5FPGA::init(uint8_t* bitstream, size_t count) {
  solo5_result_t res;

  // INFO("Solo5FPGA", "Entering init()... \n");

  // auto* data = (uint8_t*) buffer->data();

  res = solo5_fpga_init();

  if (res != SOLO5_R_OK) {
    return -1;
  }

  return 0;
}

// Unused
void Solo5FPGA::deactivate()
{
  INFO("Solo5FPGA", "deactivate");
}

#include <kernel/solo5_manager.hpp>

struct Autoreg_solo5fpga {
  Autoreg_solo5fpga() {
    Solo5_manager::register_fpga(&Solo5FPGA::new_instance);
  }
} autoreg_solo5fpga;
