# Plots roofline for a single processor based on three files from Stopwatch:
#   1. File containing single precision floating point operations
#   2. File containing double precision floating point operations
#   3. File containing L2 cache accesses. This is used to estimate arithmetic intensity


import argparse
import sys
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from decimal import Decimal
from adjustText import adjust_text


def create_proc_df(dp_data_file, sp_data_file, cache_data_file):
    proc_dp = pd.read_csv(dp_data_file)
    proc_sp = pd.read_csv(sp_data_file)
    proc_cache = pd.read_csv(cache_data_file)

    proc = proc_dp.join(proc_sp["PAPI_SP_OPS"]).join(proc_cache["PAPI_L2_DCR"])
    proc.drop("TOTAL_REAL_MICROSECONDS", axis=1, inplace=True)

    proc["TOTAL_REAL_MICROSECONDS_1"] = proc_dp["TOTAL_REAL_MICROSECONDS"]
    proc["TOTAL_REAL_MICROSECONDS_2"] = proc_sp["TOTAL_REAL_MICROSECONDS"]
    proc["TOTAL_REAL_MICROSECONDS_3"] = proc_cache["TOTAL_REAL_MICROSECONDS"]

    # Take a median of the three runtimes to compute an "average" runtime
    proc_m = proc.groupby(["NAME"])[["TOTAL_REAL_MICROSECONDS_1", "TOTAL_REAL_MICROSECONDS_2", "TOTAL_REAL_MICROSECONDS_3"]].apply(np.median)
    proc_m.name = "TOTAL_REAL_MICROSECONDS"

    proc = proc.join(proc_m, on=["NAME"])

    proc["FLOPS"] = 2* proc["PAPI_DP_OPS"] + proc["PAPI_SP_OPS"]
    proc["FLOPS/SEC"] = proc["FLOPS"] / (proc["TOTAL_REAL_MICROSECONDS"] * 1e-6)
    proc["FLOPS/BYTE"] = proc["FLOPS"] / (proc["PAPI_L2_DCR"] * 64)

    return proc


def plot_roofline(proc_df, peak_FLOPS, DRAM, L1, L2, L3, ax):
    total_time = proc_df["TOTAL_REAL_MICROSECONDS"].values[0] # Assumption that this is always GEMDM
    # Remove routines that take less than 5% of total time
    filtered_proc = proc_df.drop(proc_df.loc[(proc_df["TOTAL_REAL_MICROSECONDS"] / total_time) < 0.001].index)

    filtered_proc.plot.scatter(x="FLOPS/BYTE", y="FLOPS/SEC", loglog=True, ax=ax)

    x = list(filtered_proc["FLOPS/BYTE"])
    y = list(filtered_proc["FLOPS/SEC"])
    labels = list(filtered_proc["NAME"])

    texts = [ax.annotate(txt, (x[i], y[i])) for i, txt in enumerate(labels)]
    adjust_text(texts, arrowprops=dict(arrowstyle='-', color='k', alpha=0.5))

    # Plot max flops
    if peak_FLOPS:
        if L1:
            flop_x = np.array([peak_FLOPS/L1, max(x)])
        elif L2:
            flop_x = np.array([peak_FLOPS/L2, max(x)])
        elif L3:
            flop_x = np.array([peak_FLOPS/L3, max(x)])
        else:
            flop_x = np.array([peak_FLOPS/DRAM, max(x)])

        flop_y = 0 * flop_x + peak_FLOPS
        ax.plot(flop_x, flop_y, label=f'PEAK FLOPS - {Decimal(peak_FLOPS):.2E} FLOPS/sec')

    # Plot DRAM bandwidth
    if DRAM:
        if peak_FLOPS:
            dram_x = np.array([0, min([peak_FLOPS/DRAM, max(x)])])
        else:
            dram_x = np.array([0, min([max(y)/DRAM, max(x)])])
        dram_y = DRAM * dram_x
        ax.plot(dram_x, dram_y, label=f'DRAM - {Decimal(DRAM):.2E} Bytes/sec')

    # Plot L3 bandwidth
    if L3:
        if peak_FLOPS:
            l3_x = np.array([0, peak_FLOPS/L3])
        else:
            l3_x = np.array([0, max(y)/L3])
        l3_y = L3 * l3_x
        ax.plot(l3_x, l3_y, label=f'L3 - {Decimal(L3):.2E} Bytes/sec')

    # Plot L2 bandwidth
    if L2:
        if peak_FLOPS:
            l2_x = np.array([0, peak_FLOPS/L2])
        else:
            l2_x = np.array([0, max(y)/L2])
        l2_y = L2 * l2_x
        ax.plot(l2_x, l2_y, label=f'L2 - {Decimal(L2):.2E} Bytes/sec')

    # Plot L1 bandwidth
    if L1:
        if peak_FLOPS:
            l1_x = np.array([0, peak_FLOPS/L1])
        else:
            l1_x = np.array([0, max(y)/L1])
        l1_y = L1 * l1_x
        ax.plot(l1_x, l1_y, label=f'L1 - {Decimal(L1):.2E} Bytes/sec')


parser = argparse.ArgumentParser()

sp_file_arg = parser.add_argument('sp_filename', help="Path of file containing single precision FLOP measurement")
dp_file_arg = parser.add_argument('dp_filename', help="Path of file containing double precision FLOP measurement")
cache_file_arg = parser.add_argument('cache_filename', help="Path of file containing L2 cache access measurement")

flop_arg = parser.add_argument("-f", "--flops", help="Max FLOPs per second for one processor", type=float)
dram_arg = parser.add_argument("-d", "--dram", help="DRAM bandwidth in bytes per second for one processor", type=float)
l1_arg = parser.add_argument("-l1", help="L1 cache bandwidth in bytes per second for one processor", type=float)
l2_arg = parser.add_argument("-l2", help="L2 cache bandwidth in bytes per second for one processor", type=float)
l3_arg = parser.add_argument("-l3", help="L3 cache bandwidth in bytes per second for one processor", type=float)


args = parser.parse_args()

print("parsing data")
proc = create_proc_df(args.dp_filename, args.sp_filename, args.cache_filename)

fig, ax1 = plt.subplots(nrows=1, ncols=1)

width, height = fig.get_size_inches()
scaling_factor = 2.3

fig.set_figheight(height*scaling_factor)
fig.set_figwidth(width*scaling_factor)

print("generating plot")
plot_roofline(proc, args.flops, args.dram, args.l1, args.l2, args.l3, ax1)

plt.legend()

plot_name = "roofline.png"
plt.savefig(plot_name, dpi=400)
print(f"plot saved to {plot_name}")
