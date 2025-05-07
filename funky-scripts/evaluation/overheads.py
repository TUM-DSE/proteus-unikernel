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
settings = ["proteus", "native"]
app_dirs = {"proteus": "/home/felix/Projects/proteus/funky-unikernel/examples/benchmarks/oh_funkycl",
            "native": "/home/felix/Projects/vitis-accel-examples/ocl_kernels/oh_funkycl"}

bitstream_dir = "/share/felix/bitstreams/vitis-accel-examples/cl_helloworld"
dummy_bitstream_dir = "/share/felix/bitstreams/vitis-accel-examples/cl_burst_rw"
timestamp = datetime.datetime.now()
out_dir = "time_overheads_" + timestamp.strftime("%m%d%Y_%H%M%S")
print("Output directory:", out_dir)
script_dir = os.getcwd()

os.mkdir(out_dir)

unikernel_csv_header = "buf_size,program_bs,kernel_alloc,kernel_setarg,kernel_enqueue,buf_alloc," \
    "init_transfer,transfer,finish,total\n"
worker_init_header = "worker_init[s],fpga_reconf[s],load_fpga[s]\n"
boot_time_line = "time elapsed before launching vCPU"

out_csv_header = "setting,fpga,runs,buf_size,program_bs,program_bs_stddev,kernel_alloc," \
    "kernel_alloc_stddev,kernel_setarg,kernel_setarg_stddev,kernel_enqueue,kernel_enqueue_stddev," \
    "buf_alloc,buf_alloc_stddev,init_transfer,init_transfer_stddev,transfer,transfer_stddev," \
    "finish,finish_stddev,total,total_stddev,worker_init,worker_init_stddev,unikernel_boot," \
    "unikernel_boot_stddev"
out_csv = open(f"{out_dir}/overheads.csv", 'a')
csv_writer = csv.writer(out_csv)
csv_writer.writerow(out_csv_header.split(','))

for setting in settings:
    for fpga in fpgas:
        bitstream = f"{bitstream_dir}/{fpga}/bitstream"
        dummy_bitstream = f"{dummy_bitstream_dir}/{fpga}/bitstream"

        print(f"{setting}, {fpga}:")

        log_filename = f"{out_dir}/overheads_{setting}_{fpga}.log"
        log = open(log_filename, 'a')

        os.chdir(app_dirs[setting])

        if setting == "proteus":
            dummy_bitstream_exec_cmd = ["./execute.sh", "-b", "build", "-t",
                                        f"{fpga.split('-')[0]}", "-i", dummy_bitstream,
                                        "-a", "funcycl-app a"]
            exec_cmd = ["./execute.sh", "-b", "build", "-t",
                        f"{fpga.split('-')[0]}", "-i", bitstream, "-a", "funcycl-app a"]
        else:
            dummy_bitstream_exec_cmd = ["./main", dummy_bitstream]
            exec_cmd = ["./main", bitstream]

        for _ in range(reps):
            # Program a different bitstream between runs
            subprocess.run(dummy_bitstream_exec_cmd, stdout=subprocess.DEVNULL,
                           stderr=subprocess.DEVNULL)
            print(f"[{datetime.datetime.now()}]", ' '.join(str(s) for s in exec_cmd))
            subprocess.run(exec_cmd, stdout=log, stderr=log)

        os.chdir(script_dir)
        log.close()

        # Parse unikernel data from log
        log = open(log_filename, 'r')
        uni_log_data = [[] for i in range(len(unikernel_csv_header.split(',')))]
        out_data = [setting, fpga, reps]
        lines = log.readlines()

        for i in range(len(lines)):
            if lines[i] == unikernel_csv_header:
                for j, num in enumerate(lines[i + 1].split(',')):
                    uni_log_data[j].append(float(num))

        for i in range(len(unikernel_csv_header.split(','))):
            out_data.append(avg(uni_log_data[i]))
            # No stddev for buf_size at i == 0
            if i > 0:
                out_data.append(stddev(uni_log_data[i]))

        # Parse monitor data from log
        mon_log_data = [[] for i in range(2)]

        if setting == "proteus":
            # Monitor output in seconds
            for i in range(len(lines)):
                if lines[i] == worker_init_header:
                    mon_log_data[0].append(float(lines[i + 1].split(",")[0]) * 1000)
                elif lines[i].startswith(boot_time_line):
                    mon_log_data[1].append(float(lines[i].split(":")[1].split(" ")[1]) * 1000)
        else:
            mon_log_data[0]=[0]
            mon_log_data[1]=[0]

        for i in range(len(mon_log_data)):
            out_data.append(avg(mon_log_data[i]))
            out_data.append(stddev(mon_log_data[i]))

        csv_writer.writerow(out_data)
        log.close()
