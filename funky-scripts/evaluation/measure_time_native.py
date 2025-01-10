#! /usr/bin/env python
# -*- coding: utf-8 -*-

import os
import time
import sys
import subprocess
import statistics as stat
import csv

result_dir = sys.argv[1]
out_csv = open(sys.argv[2], 'a')
in_csv = open(sys.argv[3], 'r')
repeat = int(sys.argv[4])
bitstream_dir = sys.argv[5]
fpga = sys.argv[6]
speed = sys.argv[7]

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
        exec_cmd = [f"./{app_name}", bitstream]

        # memory type argument
        if app_name in ["cl_wide_mem_rw_2x", "cl_wide_mem_rw_4x"]:
            mem_arg = "0"
            if "ddr" in speed:
                mem_arg = "1"
            exec_cmd.append(mem_arg)

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
detailed_times_header = "app_name,kernel_input_data_size,iterations,data_to_fpga_time_cpu,kernel_time_cpu,data_to_host_time_cpu,data_to_fpga_time_ocl,kernel_time_ocl,data_to_host_time_ocl\n"
in_csv.seek(0)
for i,row in enumerate(app_list):
    writer = csv.writer(out_csv)
    app_name = row[0]
    detailed_times = None
    log_filename = f"{result_dir}/{app_name}-{fpga}-{speed}.log"
    log = open(log_filename, 'r')
    lines = log.readlines()

    # calculate avg, stdev
    times = dict_results[app_name].copy();

    if len(times) == 1:
        times.append(times[-1]) # average is just the one measurement
        times.append(0) # stddev
    else:
        avg_time = sum(times)/len(times)
        stddev   = stat.stdev(times)
        times.append(avg_time)
        times.append(stddev)

    for i in range(len(lines)):
        if lines[i] == detailed_times_header:
            detailed_times = lines[i+1]
            break

    if detailed_times is None:
        print(f"Failed to find detailed time measurements in {log_filename}")
        for _ in range(5):
            times.append(float("NaN"))
    else:
        values = detailed_times.split(",")
        for val in values[1:]:
            times.append(val.strip())

    # write values to csv
    times.insert(0, app_name)
    writer.writerow(times)

