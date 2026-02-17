#!/bin/bash

dev=tap100

if ! ip link show "$dev" &>/dev/null; then
  sudo ip tuntap add "$dev" mode tap
  sudo ip link set dev "$dev" up
  sudo ip addr add 10.0.0.1/24 dev "$dev"
fi

dev=tap0

if ! ip link show "$dev" &>/dev/null; then
  sudo ip tuntap add "$dev" mode tap
  sudo ip link set dev "$dev" up
  sudo ip addr add 10.0.1.1/24 dev "$dev"
fi

dev=tap1

if ! ip link show "$dev" &>/dev/null; then
  sudo ip tuntap add "$dev" mode tap
  sudo ip link set dev "$dev" up
  sudo ip addr add 10.0.2.1/24 dev "$dev"
fi

sudo rm /tmp/bitstream_*.ukvm
