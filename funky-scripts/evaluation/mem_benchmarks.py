#! /usr/bin/env python
# -*- coding: utf-8 -*-

import os
import datetime
import sys
import statistics as stat
import subprocess
import csv
import time


def avg(nums):
    return sum(nums)/len(nums)


def stddev(nums):
    if (len(nums) <= 1):
        return 0.0

    return stat.stdev(nums)


def parse_times(config, times_total, log_filename, csv_writer):
    # Add detailed timing data from applications' stdout and write results to csv.
    # Each application prints the header followed by the data in the next line.
    detailed_times_header = "app_name,iterations,buf_size,chunk_size,num_chunks,time_loop\n"
    detailed_times = []
    log = open(log_filename, 'r')
    lines = log.readlines()

    out_data = [config, len(times_total)]

    for i in range(len(lines)):
        if lines[i] == detailed_times_header:
            detailed_times.append(lines[i+1])

    if not detailed_times:
        print(f"Failed to find detailed time measurements in {log_filename}")
        for _ in range(8):
            out_data.append(float("NaN"))
    else:
        values = detailed_times[0].split(",")
        iterations = values[1]
        buf_size = values[2]
        chunk_size = values[3]
        num_chunks = values[4]
        out_data.append(iterations)
        out_data.append(buf_size)
        out_data.append(chunk_size)
        out_data.append(num_chunks)

        times_loop = []

        for line in detailed_times:
            values = line.split(",")
            times_loop.append(float(values[5]))

        # TODO: total, total stddev
        out_data.append(avg(times_total))
        out_data.append(stddev(times_total))
        out_data.append(avg(times_loop))
        out_data.append(stddev(times_loop))

    csv_writer.writerow(out_data)
    log.close()


if len(sys.argv) != 2:
    print(f"Usage:\n  {sys.argv[0]} <repetitions>")
    exit(1)


reps = int(sys.argv[1])
fpgas = ["u280-300-multic", "u280-ddr-300-multic"]
mem_flags = ["0", "1"]
apps = ["cl_wide_mem_rw_2x_pipelined", "cl_wide_mem_rw_4x_pipelined"]
proteus_dir = os.environ["PROTEUS_DIR"]
app_dir = f"{proteus_dir}/vitis-accel-examples/ocl_kernels"
# no overlapping & no opt, no overlapping & opt, overlapping & opt
args = ["-c 8", "-c 8 -o", "-c 1 -o"]

clk = time.CLOCK_MONOTONIC

date_time = datetime.datetime.now()
out_dir = "time_mem_" + date_time.strftime("%m%d%Y_%H%M%S")
print("Output directory:", out_dir)
script_dir = os.getcwd()

os.mkdir(out_dir)

print("Note: a dummy bitstream is programmed to the FPGA before each measurement")
print("to properly measure the bitstream programming time for multiple executions")

for app in apps:
    for fpga, mem_flag in zip(fpgas, mem_flags):
        bitstream = f"/share/felix/bitstreams/vitis-accel-examples/{app[:-10]}/{fpga}/bitstream"
        out_csv = open(f"{out_dir}/{fpga}.csv", 'a')
        csv_writer = csv.writer(out_csv)
        if app == apps[0]:
            csv_writer.writerow(["app_name", "runs", "iterations", "buf_size", "chunk_size", "num_chunks", "time_total", "time_total_stddev", "time_loop", "time_loop_stddev"])

        print(f"{app} {fpga}:")

        for arg in args:
            arg_name = arg.replace(" ", "_")
            config = f"{app}-{fpga}{arg_name}"
            log_filename = f"{out_dir}/{config}.log"
            log = open(f"{log_filename}", 'a')
            os.chdir(f"{app_dir}/{app}")

            exec_cmd = [f"./{app}", bitstream, mem_flag]
            for a in arg.split():
                exec_cmd.append(a)

            times_total = []

            for _ in range(reps):
                # Program a dummy bitstream to include bitstream programming time in measurements
                dummy_bs = "/share/felix/bitstreams/vitis-accel-examples/cl_helloworld/u280-fast/bitstream"
                subprocess.run([f"./{app}", dummy_bs, "0"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

                print(f"[{datetime.datetime.now()}]", ' '.join(str(s) for s in exec_cmd), end="", flush=True)

                t1  = time.clock_gettime(clk)
                subprocess.run(exec_cmd, stdout=log, stderr=log)
                t2  = time.clock_gettime(clk)
                time_total = t2 - t1
                times_total.append(time_total)
                print(f": {time_total:.2f} s")

            log.close()
            os.chdir(script_dir)

            parse_times(config, times_total, log_filename, csv_writer)

        out_csv.close()
