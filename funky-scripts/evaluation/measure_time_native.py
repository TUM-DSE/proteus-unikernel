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

        log = open(result_dir+"/"+app_name+".log", 'a')
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

        # measure time
        print(app_name, end=", ")
        t1  = time.clock_gettime(clk)
        subprocess.run(exec_cmd, stdout=log, stderr=log)
        t2  = time.clock_gettime(clk)
        print(t2-t1, end=", \n")
        dict_results[app_name].append(t2-t1)

        os.chdir("../")

print(dict_results)

# Write results to csv
in_csv.seek(0)
for i,row in enumerate(app_list):
    writer = csv.writer(out_csv)
    app_name = row[0]

    # calculate avg, stdev
    times = dict_results[app_name].copy();

    if len(times) > 1:
        avg_time = sum(times)/len(times)
        stddev   = stat.stdev(times)
        times.append(avg_time)
        times.append(stddev)

    # write values to csv
    times.insert(0, app_name)
    print(times)
    writer.writerow(times)

