#! /usr/bin/env python
# -*- coding: utf-8 -*-

import os
import datetime
import sys
import statistics as stat
import subprocess
import csv


def avg(nums):
    return sum(nums)/len(nums)


def stddev(nums):
    if (len(nums) <= 1):
        return 0.0

    return stat.stdev(nums)


if len(sys.argv) != 2:
    print(f"Usage:\n  {sys.argv[0]} <repetitions>")
    exit(1)

reps = int(sys.argv[1])
fpgas = ["u50-fast", "u280-fast", "u280-ddr-fast"]
app_dir = "/home/felix/Projects/proteus/funky-unikernel/examples/benchmarks/oh_funkycl"

bitstream_dir = "/share/felix/bitstreams/vitis-accel-examples/cl_helloworld"
dummy_bitstream_dir = "/share/felix/bitstreams/vitis-accel-examples/cl_burst_rw"
timestamp = datetime.datetime.now()
out_dir = "time_overheads_" + timestamp.strftime("%m%d%Y_%H%M%S")
print("Output directory:", out_dir)
script_dir = os.getcwd()

os.mkdir(out_dir)

benchmark_csv_header = "buf_size,program_bs,kernel_alloc,kernel_setarg,kernel_enqueue,buf_alloc," \
    "init_transfer,transfer,finish,total\n"
out_csv_header = "fpga,runs," + benchmark_csv_header.rstrip()
out_csv = open(f"{out_dir}/overheads.csv", 'a')
csv_writer = csv.writer(out_csv)
csv_writer.writerow(out_csv_header.split(','))

for fpga in fpgas:
    bitstream = f"{bitstream_dir}/{fpga}/bitstream"
    dummy_bitstream = f"{dummy_bitstream_dir}/{fpga}/bitstream"

    print(f"{fpga}:")

    log_filename = f"{out_dir}/overheads_{fpga}.log"
    log = open(log_filename, 'a')

    os.chdir(app_dir)

    # For programming a different bitstream between runs
    dummy_bitstream_exec_cmd = ["./execute.sh", "-b", "build", "-t",
                                f"{fpga.split('-')[0]}", "-i", dummy_bitstream,
                                "-a", "funcycl-app a"]

    exec_cmd = ["./execute.sh", "-b", "build", "-t",
                f"{fpga.split('-')[0]}", "-i", bitstream, "-a", "funcycl-app a"]

    for _ in range(reps):
        subprocess.run(dummy_bitstream_exec_cmd, stdout=subprocess.DEVNULL,
                       stderr=subprocess.DEVNULL)
        print(f"[{datetime.datetime.now()}]", ' '.join(str(s) for s in exec_cmd))
        subprocess.run(exec_cmd, stdout=log, stderr=log)

    os.chdir(script_dir)
    log.close()

    # Parse data from log
    log = open(log_filename, 'r')
    log_data = [[] for i in range(len(benchmark_csv_header.split(',')))]
    out_data = [fpga, reps]
    lines = log.readlines()

    for i in range(len(lines)):
        if lines[i] == benchmark_csv_header:
            for j, num in enumerate(lines[i + 1].split(',')):
                log_data[j].append(float(num))

    # TODO: stddev
    for i in range(len(benchmark_csv_header.split(','))):
        out_data.append(avg(log_data[i]))

    csv_writer.writerow(out_data)
    log.close()
