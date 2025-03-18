#!/bin/bash
for i in {1..1}
do
  if [ -e file.mig ]; then
    rm file.mig
  fi

  ./execute.sh -m -a "test vadd.xclbin" -s /tmp/solo5_socket_a &
  sleep 4 && echo -n "savevm file.mig" | socat -u - unix-connect:/tmp/solo5_socket_a
  sleep 1
done
