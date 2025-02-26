#! /usr/bin/env python
# -*- coding: utf-8 -*-

import os
import time
import sys
import subprocess
import statistics as stat
import csv

def avg(nums):
    return sum(nums)/len(nums)

def stddev(nums):
    if (len(nums) <= 1):
        return 0.0

    return stat.stdev(nums)

result_dir = sys.argv[1]
out_csv = open(sys.argv[2], 'a')
in_csv = open(sys.argv[3], 'r')
repeat = int(sys.argv[4])
bitstream_dir = sys.argv[5]
fpga = sys.argv[6]
speed = sys.argv[7]
exec_cmd_base = sys.argv[8:]

app_list = csv.reader(in_csv)
clk = time.CLOCK_MONOTONIC
dict_results = dict()

# repeat execution
for cnt in range(repeat):
    in_csv.seek(0)

    # to avoid executing the same workload continuously, all workloads run in order in one loop
    # otherwise, FPGA reconfiguration is skipped from the second execution
    for i,row in enumerate(app_list):
        app_name = row[0]

        log_filename = f"{result_dir}/{app_name}-{fpga}-{speed}.log"
        log = open(log_filename, 'a')
        os.chdir(app_name)
        # print("current app dir: ", os.getcwd())

        # add time_list for each app to the dict only once
        if app_name not in dict_results:
            dict_results.update({app_name: list()})

        bitstream = f"{bitstream_dir}/{app_name}/{fpga}-{speed}/bitstream"

        if not os.path.isfile(bitstream):
            print(f"Found no bitstream for {app_name}")
            dict_results[app_name].append(float("NaN"))
            os.chdir("../")
            continue

        # prepare exec command
        exec_cmd = exec_cmd_base.copy()
        if app_name == "cl_gmem_2banks":
            exec_cmd.append("-x")
        # virtual bitstream (see below), so the program just gets some dummy bitstream path
        exec_cmd.append("dummy-bitstream")
        # arg can be empty
        for arg in row[1:]:
            if arg:
                exec_cmd.append(arg)

        if app_name in ["cl_wide_mem_rw_2x", "cl_wide_mem_rw_4x"]:
            # memory type argument
            mem_arg = "0"
            if "ddr" in speed:
                mem_arg = "1"
            exec_cmd.append(mem_arg)
            # enable out-of-order execution for OpenCL command queue in monitor
            exec_cmd.insert(1, "--ooo")

        # link the bitstream to /tmp/bitstream_0.ukvm, expected location by funky-monitor
        subprocess.run(["ln", "-sf", f"{bitstream}", "/tmp/bitstream_0.ukvm"])

        # measure time
        print(app_name, end=", ")
        t1  = time.clock_gettime(clk)
        subprocess.run(exec_cmd, stdout=log, stderr=log)
        t2  = time.clock_gettime(clk)
        print(t2-t1, end=", \n")
        dict_results[app_name].append(t2-t1)

        os.chdir("../")

# Add detailed timing data from applications' stdout and write results to csv.
# Each application prints the header followed by the data in the next line.
detailed_times_header = "app_name,kernel_input_data_size,kernel_output_data_size,iterations,time_cpu,data_to_fpga_time_ocl,kernel_time_ocl,data_to_host_time_ocl\n"
in_csv.seek(0)
for i,row in enumerate(app_list):
    writer = csv.writer(out_csv)
    app_name = row[0]
    detailed_times = []
    log_filename = f"{result_dir}/{app_name}-{fpga}-{speed}.log"
    log = open(log_filename, 'r')
    lines = log.readlines()

    # calculate avg, stdev
    times = dict_results[app_name].copy();

    times.append(avg(times))
    times.append(stddev(times))

    for i in range(len(lines)):
        if lines[i] == detailed_times_header:
            detailed_times.append(lines[i+1])

    if not detailed_times:
        print(f"Failed to find detailed time measurements in {log_filename}")
        for _ in range(7):
            times.append(float("NaN"))
    else:
        # data sizes and iterations are the same for each run
        values = detailed_times[0].split(",")
        for val in values[1:4]:
            times.append(val.strip())

        times_cpu = []
        times_to_fpga = []
        times_kernel = []
        times_to_host = []

        # calculate avg and stddev for FPGA times
        for line in detailed_times:
            values = line.split(",")
            times_cpu.append(float(values[4]))
            times_to_fpga.append(float(values[5]))
            times_kernel.append(float(values[6]))
            times_to_host.append(float(values[7]))

        times.append(avg(times_cpu))
        times.append(stddev(times_cpu))
        times.append(avg(times_to_fpga))
        times.append(stddev(times_to_fpga))
        times.append(avg(times_kernel))
        times.append(stddev(times_kernel))
        times.append(avg(times_to_host))
        times.append(stddev(times_to_host))

    # write values to csv
    times.insert(0, app_name)
    writer.writerow(times)
