#!/bin/bash

dev=tap100

if ! ip link show "$dev" &>/dev/null; then
  sudo ip tuntap add "$dev" mode tap user "$USER"
  sudo ip link set dev "$dev" up
  sudo ip addr add 10.0.0.1/24 dev "$dev"
fi
