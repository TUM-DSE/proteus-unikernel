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


def parse_times(config, reps, buf_size, mem_limit, log_filename, csv_writer):
    # Add detailed timing data from applications' stdout and write results to csv.
    # Each application prints the header followed by the data in the next line.
    detailed_times_header = "app_name,num_chunks,chunk_size,kernel_input_data_size," \
        "kernel_output_data_size,iterations,time_cpu,data_to_fpga_time_ocl,kernel_time_ocl," \
        "data_to_host_time_ocl\n"
    detailed_times = []
    log = open(log_filename, 'r')
    lines = log.readlines()

    out_data = [config, reps, buf_size, mem_limit]

    for i in range(len(lines)):
        if lines[i] == detailed_times_header:
            detailed_times.append(lines[i+1])

    if not detailed_times:
        print(f"Failed to find detailed time measurements in {log_filename}")
        for _ in range(11):
            out_data.append(float("NaN"))
    else:
        values = detailed_times[0].split(",")
        num_chunks = values[1]
        out_data.append(num_chunks)
        chunk_size = values[2]
        out_data.append(chunk_size)
        input_data_size = values[3]
        out_data.append(input_data_size)

        times_cpu = []
        times_to_fpga = []
        times_kernel = []
        times_to_host = []

        for line in detailed_times:
            values = line.split(",")
            times_cpu.append(float(values[6]))
            times_to_fpga.append(float(values[7]))
            times_kernel.append(float(values[8]))
            times_to_host.append(float(values[9]))

        out_data.append(avg(times_cpu))
        out_data.append(stddev(times_cpu))
        out_data.append(avg(times_to_fpga))
        out_data.append(stddev(times_to_fpga))
        out_data.append(avg(times_kernel))
        out_data.append(stddev(times_kernel))
        out_data.append(avg(times_to_host))
        out_data.append(stddev(times_to_host))

    csv_writer.writerow(out_data)
    log.close()


if len(sys.argv) != 2:
    print(f"Usage:\n  {sys.argv[0]} <repetitions>")
    exit(1)


reps = int(sys.argv[1])
fpgas = ["u280-fast", "u280-ddr-fast", "u280-ddr-opt-fast"]

app = "cl_wide_mem_rw_strm_oversub"
# First run is ignored, just to have the bitstream already programmed for subsequent runs
args = {
    "buf_sizes": [1, 2048, 2048, 2048, 2048, 2048,
                  2048, 2048, 2048, 2048, 2048],
    "mem_limits": [10, 1000000000, 4096, 2048, 1024, 512,
                   1000000000, 4096, 2048, 1024, 512],
    "flags": ["", "", "", "", "", "",
              "-o", "-o", "-o", "-o", "-o",]
}
proteus_dir = os.environ["PROTEUS_DIR"]
app_dir = f"{proteus_dir}/vitis-accel-examples/ocl_kernels"
print(f"Application: {app_dir}/{app}")

bitstream_dir = f"/share/felix/bitstreams/vitis-accel-examples/{app}"
date_time = datetime.datetime.now()
out_dir = "time_oversub_" + date_time.strftime("%m%d%Y_%H%M%S")
print("Output directory:", out_dir)
script_dir = os.getcwd()

os.mkdir(out_dir)

for fpga in fpgas:
    bitstream = f"{bitstream_dir}/{fpga}/bitstream"
    out_csv = open(f"{out_dir}/{fpga}.csv", 'a')
    csv_writer = csv.writer(out_csv)
    csv_writer.writerow(["app_name", "runs", "buf_size", "mem_limit", "num_chunks", "chunk_size",
                         "input_size", "time_total", "time_total_stddev", "time_to_fpga",
                         "time_to_fpga_stddev", "time_kernel", "time_kernel_stddev", "time_to_host",
                         "time_to_host_stddev"])

    print(f"{fpga}:")

    for i in range(len(args["buf_sizes"])):
        # Optimized DDR version always uses -o, run warmup run 0, skip run 1 - 5
        if fpga == "u280-ddr-opt-fast" and i > 0 and i < 6:
            continue

        buf_size = args["buf_sizes"][i]
        mem_limit = args["mem_limits"][i]
        flags = args["flags"][i]
        config = f"{app}-{fpga}-buf{buf_size}-lim{mem_limit}{flags}"
        log_filename = f"{out_dir}/{config}.log"
        log = open(f"{log_filename}", 'a')
        os.chdir(f"{app_dir}/{app}")

        exec_cmd = [f"./{app}", bitstream, "-s", str(buf_size), "-m", str(mem_limit)]
        if args["flags"][i]:
            exec_cmd.append(flags)
            if fpga == "u280-ddr-opt-fast":
                exec_cmd.append("-d")

        # First run for programming bitstream only
        if i == 0:
            print(f"[{datetime.datetime.now()}] Warmup run for programming bitstream")
            subprocess.run(exec_cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            log.close()
            os.chdir(script_dir)
            continue

        for _ in range(reps):
            print(f"[{datetime.datetime.now()}]", ' '.join(str(s) for s in exec_cmd))
            subprocess.run(exec_cmd, stdout=log, stderr=log)

        log.close()
        os.chdir(script_dir)

        parse_times(config, reps, buf_size, mem_limit, log_filename, csv_writer)

    out_csv.close()
