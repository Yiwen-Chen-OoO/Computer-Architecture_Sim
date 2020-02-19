#!/usr/bin/env python

import sys
import subprocess

from os.path import join
from os import path
from os import listdir

SIM="/home/shihao-song/Documents/Research/Tools_Development/Hybrid-eDRAM-PCM-Simulator/PCMSim"
CFG="/home/shihao-song/Documents/Research/Tools_Development/Hybrid-eDRAM-PCM-Simulator/configs/Skylake-Trace.cfg"
OUT_DIR="/home/shihao-song/Documents/Data-2/ECEC-623-P2-Traces"
IN_DIR="/home/shihao-song/Documents/Data/Tensorflow"

WORKLOADS = [w for w in listdir(IN_DIR)]
MPS = []
# Generate 5MP workloads
import random
for i in range(5):
    idx = random.sample(range(0, len(WORKLOADS)), 8)

    MP = []
    for i in idx:
        MP.append(join(IN_DIR, WORKLOADS[i]))

    MPS.append(MP)

task_id = int(sys.argv[1])

traces = ""
for w in MPS[task_id][:-1]:
    traces += w.split("/")[-1].split(".")[0]
    traces += "_AND_"
traces += MPS[task_id][-1].split("/")[-1].split(".")[0]

STATS_OUTPUT = join(OUT_DIR, traces + ".stats")
TRACE_OUTPUT = join(OUT_DIR, traces + ".mem_trace")

args = []
args.append(SIM)
args.append("--pcm-config")
args.append(CFG)
for w in MPS[task_id]:
    args.append("--cpu_trace")
    args.append(w)
args.append("--trace_output")
args.append(TRACE_OUTPUT)
args.append("--stat_output")
args.append(STATS_OUTPUT)

subprocess.call(args)
