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
repeat = int(sys.argv[3])
exec_cmd_base = sys.argv[4:]

chunk_num_list = [1, 2, 4, 8, 16, 32, 64, 128, 256]
clk = time.CLOCK_MONOTONIC
dict_results = dict()

# evaluate different chunk nums
for row in enumerate(chunk_num_list):
    chunk_num = row[1]
    log_filename = f"{result_dir}/chunk-{chunk_num}.log"
    log = open(log_filename, 'a')

    # prepare exec command
    exec_cmd = exec_cmd_base.copy()
    exec_cmd.append(f"fir.xclbin {chunk_num}")

    # repeat execution
    for cnt in range(repeat):
        print(chunk_num, end=", ")
        t1  = time.clock_gettime(clk)
        subprocess.run(exec_cmd, stdout=log, stderr=log)
        t2  = time.clock_gettime(clk)
        print(t2-t1, end=", \n")
        # dict_results[app_name].append(t2-t1)

# First, write a header to the csv file
csv_header = ["chunk_num", "total_time_avg[s]", "stddev", "sync_oh_avg[s]", "stddev", "loop_num", "signal_length[MB]", "length_per_chunk[MB]"]
writer = csv.writer(out_csv)
writer.writerow(csv_header)

# Add detailed timing data from applications' stdout and write results to csv.
# Each application prints the header followed by the data in the next line.
detailed_times_header = "num_of_chunks,signal_length[MB],signal_length_per_chunk[MB],total_time[s],sync_oh[s]\n"

# add each line
for row in enumerate(chunk_num_list):
    chunk_num = row[1]

    detailed_times = []
    log_filename = f"{result_dir}/chunk-{chunk_num}.log"
    log = open(log_filename, 'r')
    lines = log.readlines()

    # add a list for each chunk setup to the dict
    dict_results.update({chunk_num: list()})
    times = dict_results[chunk_num].copy();

    for i in range(len(lines)):
        if lines[i] == detailed_times_header:
            detailed_times.append(lines[i+1])

    if not detailed_times:
        print(f"Failed to find detailed time measurements in {log_filename}")
        for _ in range(7):
            times.append(float("NaN"))
    else:
        values = detailed_times[0].split(",")

        # calculate avg and stddev for times 
        times_total = []
        times_sync_oh = []
        for line in detailed_times:
            values = line.split(",")
            times_total.append(float(values[3]))
            times_sync_oh.append(float(values[4]))

        # add numbers to the line
        # times.append(chunk_num)
        times.append(avg(times_total))
        times.append(stddev(times_total))
        times.append(avg(times_sync_oh))
        times.append(stddev(times_sync_oh))
        times.append(repeat)
        times.append(values[1])
        times.append(float(values[2]))

    # write values to csv
    times.insert(0, chunk_num)
    writer.writerow(times)
    print(times)
