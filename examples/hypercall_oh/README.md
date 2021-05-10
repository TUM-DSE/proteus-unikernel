# how to measure hypercall overhead
  git clone git@github.com:AtsushiKoshiba/funky-solo5.git
  cd funky-solo5/vfpga-dev
  make FUNKY_MACROS=-DEVAL_HYPERCALL_OH

  cd <here>
  ./build.sh build
  ./execute.sh build ~/funky-solo5/ukvm/ukvm-bin > hypercall_oh.csv

