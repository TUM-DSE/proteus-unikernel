#! /usr/bin/env python
# -*- coding: utf-8 -*-

import os
import time
import sys
import subprocess
import statistics as stat
import csv
import asyncio

def avg(nums):
    return sum(nums)/len(nums)

def stddev(nums):
    if (len(nums) <= 1):
        return 0.0

    return stat.stdev(nums)

async def async_issue_state_cmds(socket_path, log_filename):
    save_fpga_cmd = f"echo save_fpga | socat -u - unix-connect:{socket_path}"
    load_fpga_cmd = f"echo load_fpga | socat -u - unix-connect:{socket_path}"

    log = open(log_filename, "r")
    log_line = ""

    print("subprocess is running...")
    while not log_line.startswith("wait for migration"):
        log_line = log.readline()
        if not log_line:
            await asyncio.sleep(0.001)

    await asyncio.sleep(1)

    print("writing save_fpga cmd...")
    save_fpga_process = await asyncio.create_subprocess_shell(
            save_fpga_cmd,
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE,
            )
    results = await asyncio.gather(save_fpga_process.communicate())

    await asyncio.sleep(5)
    print("writing load_fpga cmd...")
    load_fpga_process = await asyncio.create_subprocess_shell(
            load_fpga_cmd,
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE,
            )
    results = await asyncio.gather(load_fpga_process.communicate())

async def main():
    result_dir = sys.argv[1]
    out_csv = open(sys.argv[2], 'a')
    repeat = int(sys.argv[3])
    exec_cmd_base = sys.argv[4:]
    
    signal_size_list = [1, 50, 100, 200, 400, 600, 800, 1000] # MB (1024*1024 Bytes)
    # signal_size_list = [1, 50] # MB (1024*1024 Bytes)
    clk = time.CLOCK_MONOTONIC
    dict_results = dict()
    
    # evaluate different signal sizes
    for row in enumerate(signal_size_list):
        signal_size = row[1]
        log_filename = f"{result_dir}/signal-{signal_size}mb.log"
        log = open(log_filename, 'a')
    
        # prepare exec command
        exec_cmd = exec_cmd_base.copy()
        exec_cmd.append(f"fir.xclbin {signal_size}")
    
        # get socket name
        socket=""
        for item in exec_cmd:
            if item.startswith("--mon="): # Case-insensitive check
                socket = item[6:]
        # print(socket)
    
        # repeat execution
        for cnt in range(repeat):
            print(signal_size, end=", ")
            socket_task = asyncio.create_task(async_issue_state_cmds(socket, log_filename)) 
            t1  = time.clock_gettime(clk)

            print("spawning ukvm...")
            ukvm_task = await asyncio.create_subprocess_exec(*exec_cmd, 
                    stdout=log,stderr=log)
            await ukvm_task.wait()

            # subprocess.run(exec_cmd, stdout=log, stderr=log)
            t2  = time.clock_gettime(clk)
            print(t2-t1, end=", \n")
            # dict_results[app_name].append(t2-t1)
            
            await socket_task
    
    #    exit(1)
    
    # First, write a header to the csv file
    csv_header = ["signal_size[MB]", "save_fpga[s]", "stddev", "load_fpga[s]", "stddev", "sync_fpga[s]", "stddev", 
            "save_fpga_state[s]", "stddev", "load_fpga_reconf[s]", "stddev", "worker_init[s]", "stddev", "fpga_reconf[s]", "stddev", 
            "load_fpga_state[s]", "stddev", "loop_num"]
    writer = csv.writer(out_csv)
    writer.writerow(csv_header)
    
    # Add detailed timing data from applications' stdout and write results to csv.
    # Each application prints the header followed by the data in the next line.
    savefpga_detailed_header = "sync_fpga[s],sync_fpga_mem_only[s],save_fpga[s]\n"
    loadfpga_detailed_header = "worker_init[s],fpga_reconf[s],load_fpga[s]\n"
    savefpga_header = "save_fpga()[s]\n"
    loadfpga_header = "load_fpga()[s]\n"
    
    # add each line
    for row in enumerate(signal_size_list):
        signal_size = row[1]
    
        savefpga_detailed = []
        loadfpga_detailed = []
        savefpga = []
        loadfpga = []
        log_filename = f"{result_dir}/signal-{signal_size}mb.log"
        log = open(log_filename, 'r')
        lines = log.readlines()
    
        # add a list for each data size to the dict
        dict_results.update({signal_size: list()})
        times = dict_results[signal_size].copy();
    
        for i in range(len(lines)):
            if lines[i] == savefpga_detailed_header:
                savefpga_detailed.append(lines[i+1])
            elif lines[i] == loadfpga_detailed_header:
                loadfpga_detailed.append(lines[i+1])
            elif lines[i] == savefpga_header:
                savefpga.append(lines[i+1])
            elif lines[i] == loadfpga_header:
                loadfpga.append(lines[i+1])
    
        if not savefpga_detailed_header:
            print(f"Failed to find detailed time measurements in {log_filename}")
            for _ in range(17):
                times.append(float("NaN"))
        else:
            # calculate avg and stddev for every item 
            times_savefpga = []
            times_loadfpga = []
            times_syncfpga = []
            times_savefpga_state  = []
            times_loadfpga_reconf = []
            times_worker_init = []
            times_fpga_reconf = []
            times_loadfpga_state  = []

            # print(savefpga)
            # print(loadfpga)
            # print(savefpga_detailed)
            # print(loadfpga_detailed)

            for line in savefpga:
                values = line.split(",")
                times_savefpga.append(float(values[0]))

            for line in loadfpga:
                values = line.split(",")
                times_loadfpga_reconf.append(float(values[0]))

            for line in savefpga_detailed:
                values = line.split(",")
                times_syncfpga.append(float(values[0]))
                times_savefpga_state.append(float(values[2]))

            for line in loadfpga_detailed:
                values = line.split(",")
                times_worker_init.append(float(values[0]))
                times_fpga_reconf.append(float(values[1]))
                times_loadfpga_state.append(float(values[2]))
                times_loadfpga.append(float(values[0])+float(values[2]))
    
            # add numbers to the line
            # times.append(signal_size)
            times.append(avg(times_savefpga))
            times.append(stddev(times_savefpga))
            times.append(avg(times_loadfpga))
            times.append(stddev(times_loadfpga))
            times.append(avg(times_syncfpga))
            times.append(stddev(times_syncfpga))
            times.append(avg(times_savefpga_state))
            times.append(stddev(times_savefpga_state))
            times.append(avg(times_loadfpga_reconf))
            times.append(stddev(times_loadfpga_reconf))
            times.append(avg(times_worker_init))
            times.append(stddev(times_worker_init))
            times.append(avg(times_fpga_reconf))
            times.append(stddev(times_fpga_reconf))
            times.append(avg(times_loadfpga_state))
            times.append(stddev(times_loadfpga_state))
            times.append(repeat)
    
        # write values to csv
        times.insert(0, signal_size)
        writer.writerow(times)
        print(times)

if __name__ == "__main__":
    asyncio.run(main())
