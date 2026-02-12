#! /usr/bin/env python
# -*- coding: utf-8 -*-

import os
import time
import sys
import subprocess
import statistics as stat
import csv
import asyncio
import re

def avg(nums):
    return sum(nums)/len(nums)

def stddev(nums):
    if (len(nums) <= 1):
        return 0.0

    return stat.stdev(nums)

def update_bitstream_link(new_fpga):
    link_dst = os.readlink("/tmp/bitstream_0.ukvm")
    link_dst = re.sub("u.*-fast", new_fpga, link_dst)
    subprocess.run(["ln", "-sf", link_dst, "/tmp/bitstream_0.ukvm"])

async def async_issue_state_cmds(socket_path, snapshot_file, log_filename, rep):
    save_fpga_cmd = f"echo savevm {snapshot_file} | socat -u - unix-connect:{socket_path}"

    # Wait for the line that starts with 'wait for migration'
    # to appear in the log before sending the save command
    log = open(log_filename, "r")
    log_line = ""
    line_matches = 0

    print("subprocess is running...")
    # In iteration n the file has to contain the line n + 1 times to continue
    while line_matches < rep + 1:
        log_line = log.readline()
        if not log_line:
            await asyncio.sleep(0.001)
        elif log_line.startswith("wait for migration"):
            line_matches += 1

    await asyncio.sleep(1)

    print("writing savevm cmd...")
    save_fpga_process = await asyncio.create_subprocess_shell(
            save_fpga_cmd,
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE,
            )
    results = await asyncio.gather(save_fpga_process.communicate())

async def clear_page_cache():
    clear_cache_cmd = 'sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches"'
    clear_cache_process = await asyncio.create_subprocess_shell(clear_cache_cmd)
    await clear_cache_process.wait()

    if clear_cache_process.returncode ==0:
        print("page cache is cleared.")

async def main():
    result_dir = sys.argv[1]
    out_csv = open(sys.argv[2], 'a')
    repeat = int(sys.argv[3])
    src_fpga = sys.argv[4]
    exec_cmd_base = sys.argv[5:]

    dst_fpgas = os.getenv("PROTEUS_FPGAS", "u50-fast u280-fast u280-ddr-fast")
    dst_fpgas = dst_fpgas.split(" ")

    signal_size_list = [1000] # MB (1024*1024 Bytes)
    # signal_size_list = [1, 50] # MB (1024*1024 Bytes)
    # signal_size_list = [1000] # MB (1024*1024 Bytes)
    clk = time.CLOCK_MONOTONIC

    # get socket name
    socket=""
    for item in exec_cmd_base:
        if item.startswith("--mon="): # Case-insensitive check
            socket = item[6:]

    for dst_fpga in dst_fpgas:
        ### VM save
        update_bitstream_link(src_fpga)
        for row in enumerate(signal_size_list):
            signal_size = row[1]
            snapshot_file = f"mig_file_{signal_size}mb"
            log_savevm_filename = f"{result_dir}/savevm-signal-{signal_size}mb-{src_fpga}-to-{dst_fpga}.log"
            log_savevm = open(log_savevm_filename, 'a')

            # prepare exec command
            exec_cmd = exec_cmd_base.copy()
            exec_cmd.append(f"fir.xclbin {signal_size}")

            # delete the snapshot if exists
            if os.path.exists(snapshot_file):
                os.remove(snapshot_file)
                print(f"The snapshot {snapshot_file} is deleted.")

            # repeat execution
            for cnt in range(repeat):
                # spawn an async thread to issue savevm command
                socket_task = asyncio.create_task(async_issue_state_cmds(socket, snapshot_file, log_savevm_filename, cnt))

                # run benchmark
                print(signal_size, end=", ")
                t1  = time.clock_gettime(clk)
                print("spawning ukvm...")
                ukvm_task = await asyncio.create_subprocess_exec(*exec_cmd,stdout=log_savevm,stderr=log_savevm)
                await ukvm_task.wait()
                t2  = time.clock_gettime(clk)
                print(t2-t1, end=", \n")

                await socket_task

                # delete a saved snapshot
                if cnt != repeat-1:
                    if os.path.exists(snapshot_file):
                        os.remove(snapshot_file)
                        print(f"The snapshot {snapshot_file} is deleted.")
                else:
                    print(f"keep the last snapshot {snapshot_file} for VM load operations.")

        ### VM load
        update_bitstream_link(dst_fpga)
        for row in enumerate(signal_size_list):
            signal_size = row[1]
            snapshot_file = f"mig_file_{signal_size}mb"
            log_loadvm_filename = f"{result_dir}/loadvm-signal-{signal_size}mb-{src_fpga}-to-{dst_fpga}.log"
            log_loadvm = open(log_loadvm_filename, 'a')

            # prepare exec command
            exec_cmd = exec_cmd_base.copy()
            # change destination FPGA
            for i in range(len(exec_cmd)):
                if exec_cmd[i].startswith("--fpga"):
                    fpga_model = dst_fpga.split("-")[0]
                    exec_cmd[i] = f"--fpga={fpga_model}"

            exec_cmd.append(f"fir.xclbin {signal_size}")
            exec_cmd.insert(1, f"--load={snapshot_file}")
            # print(exec_cmd)

            # repeat execution
            for cnt in range(repeat):
                # clear page cache before the execution
                clear_cache_task = asyncio.create_task(clear_page_cache())
                await clear_cache_task

                # run benchmark
                print(signal_size, end=", ")
                t1  = time.clock_gettime(clk)
                print("spawning ukvm...")
                ukvm_task = await asyncio.create_subprocess_exec(*exec_cmd,stdout=log_loadvm,stderr=log_loadvm)
                await ukvm_task.wait()
                t2  = time.clock_gettime(clk)
                print(t2-t1, end=", \n")

    ### Save results
    dict_results = dict()

    # write a header to the csv file
    csv_header = ["signal_size[MB]", "src_fpga", "dst_fpga", "saved_page_size[Bytes]", "save_vm[s]", "stddev", "load_vm[s]", "stddev",
            "save_fpga[s]", "stddev", "load_fpga[s]", "stddev", "sync_fpga[s]", "stddev", "save_fpga_state[s]", "stddev",
            "worker_init[s]", "stddev", "fpga_reconf[s]", "stddev", "load_fpga_state[s]", "stddev", "loop_num"]
    writer = csv.writer(out_csv)
    # Only write header if file is empty
    if out_csv.tell() == 0:
        writer.writerow(csv_header)

    # Add detailed timing data from applications' stdout and write results to csv.
    # Each application prints the header followed by the data in the next line.
    savefpga_detailed_header = "sync_fpga[s],sync_fpga_mem_only[s],save_fpga[s]\n"
    loadfpga_detailed_header = "worker_init[s],fpga_reconf[s],load_fpga[s]\n"
    savefpga_header = "save_fpga()[s]\n"
    # loadfpga_header = "load_fpga()[s]\n"
    savevm_detailed_header = "saved page size[Bytes],savefpga()[s],savevm()[s]\n"
    loadvm_detailed_header = "loaded page size[Bytes],loadvm (page-only)[s],loadvm()[s]\n"

    for dst_fpga in dst_fpgas:
        # add each line
        for row in enumerate(signal_size_list):
            signal_size = row[1]

            savevm_detailed = []
            loadvm_detailed = []
            savefpga_detailed = []
            loadfpga_detailed = []
            savefpga = []
            # loadfpga = []
            log_savevm_filename = f"{result_dir}/savevm-signal-{signal_size}mb-{src_fpga}-to-{dst_fpga}.log"
            log_loadvm_filename = f"{result_dir}/loadvm-signal-{signal_size}mb-{src_fpga}-to-{dst_fpga}.log"
            log_savevm = open(log_savevm_filename, 'r')
            log_loadvm = open(log_loadvm_filename, 'r')
            lines_savevm = log_savevm.readlines()
            lines_loadvm = log_loadvm.readlines()

            # add a list for each data size to the dict
            dict_results.update({signal_size: list()})
            times = dict_results[signal_size].copy();

            times.append(src_fpga)
            times.append(dst_fpga)

            for i in range(len(lines_savevm)):
                if lines_savevm[i] == savefpga_detailed_header:
                    savefpga_detailed.append(lines_savevm[i+1])
                elif lines_savevm[i] == savefpga_header:
                    savefpga.append(lines_savevm[i+1])
                elif lines_savevm[i] == savevm_detailed_header:
                    savevm_detailed.append(lines_savevm[i+1])

            for i in range(len(lines_loadvm)):
                if lines_loadvm[i] == loadfpga_detailed_header:
                    loadfpga_detailed.append(lines_loadvm[i+1])
                elif lines_loadvm[i] == loadvm_detailed_header:
                    loadvm_detailed.append(lines_loadvm[i+1])
                # elif lines_loadvm[i] == loadfpga_header:
                #     loadfpga.append(lines_loadvm[i+1])

            if not savevm_detailed_header:
                print(f"Failed to find detailed time measurements in {log_savevm_filename}")
                for _ in range(len(csv_header)):
                    times.append(float("NaN"))
            else:
                # saved/loaded page sizes are the same for each run
                values = savevm_detailed[0].split(",")
                times.append(values[0].strip())
                # times.append(float(values[0].strip())/float(1024*1024)) # convert Bytes to MB

                # calculate avg and stddev for every item
                times_savevm = []
                times_loadvm = []
                times_savefpga = []
                times_loadfpga = []
                times_syncfpga = []
                times_savefpga_state  = []
                # times_loadfpga_reconf = []
                times_worker_init = []
                times_fpga_reconf = []
                times_loadfpga_state  = []

                # print(savefpga)
                # print(savefpga_detailed)
                # print(loadfpga_detailed)
                # print(savevm_detailed)
                # print(loadvm_detailed)

                for line in savevm_detailed:
                    values = line.split(",")
                    times_savevm.append(float(values[2]))

                for line in loadvm_detailed:
                    values = line.split(",")
                    times_loadvm.append(float(values[2]))

                for line in savefpga:
                    values = line.split(",")
                    times_savefpga.append(float(values[0]))

                # for line in loadfpga:
                #     values = line.split(",")
                #     times_loadfpga_reconf.append(float(values[0]))

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
                times.append(avg(times_savevm))
                times.append(stddev(times_savevm))
                times.append(avg(times_loadvm))
                times.append(stddev(times_loadvm))
                times.append(avg(times_savefpga))
                times.append(stddev(times_savefpga))
                times.append(avg(times_loadfpga))
                times.append(stddev(times_loadfpga))
                times.append(avg(times_syncfpga))
                times.append(stddev(times_syncfpga))
                times.append(avg(times_savefpga_state))
                times.append(stddev(times_savefpga_state))
                # times.append(avg(times_loadfpga_reconf))
                # times.append(stddev(times_loadfpga_reconf))
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
            print(csv_header)
            print(times)

if __name__ == "__main__":
    asyncio.run(main())
