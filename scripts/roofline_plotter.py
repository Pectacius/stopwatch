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

    proc = proc_dp.join(proc_sp["PAPI_SP_OPS"]).join(proc_cache["PAPI_L2_DCR"]).join(proc_cache["PAPI_L3_TCM"])
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


def plot_roofline(proc_df, peak_FLOPS, DRAM, L2, ax, txt_rot):
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
    flop_x = np.array([peak_FLOPS/L2, max(x)])
    flop_y = 0 * flop_x + peak_FLOPS
    ax.plot(flop_x, flop_y, label=f'PEAK FLOPS - {Decimal(peak_FLOPS):.2E} FLOPS/sec')

    # Plot DRAM bandwidth
    dram_x = np.array([0, min([peak_FLOPS/DRAM, max(x)])])
    dram_y = DRAM * dram_x
    ax.plot(dram_x, dram_y, label=f'DRAM - {Decimal(DRAM):.2E} Bytes/sec')

    # Plot L2 bandwidth
    l2_x = np.array([0, peak_FLOPS/L2])
    l2_y = L2 * l2_x
    ax.plot(l2_x, l2_y, label=f'L2 - {Decimal(L2):.2E} Bytes/sec')


if len(sys.argv) != 4:
    print(f"Usage: python3 {sys.argv[0]} <path_to_sp_flops_measurement> <path_to_dp_flops_measurement> <path_to_cache_measurement>")
else:
    sp_measurement_path = sys.argv[1]
    dp_measurement_path = sys.argv[2]
    cache_measurement_path = sys.argv[3]
    proc = create_proc_df(dp_measurement_path, sp_measurement_path, cache_measurement_path)

    fig, ax1 = plt.subplots(nrows=1, ncols=1)

    width, height = fig.get_size_inches()
    scaling_factor = 2.3

    fig.set_figheight(height*scaling_factor)
    fig.set_figwidth(width*scaling_factor)

    PEAK_FLOPS_PER_PROC = 2726 / 40 *1e9
    PEAK_DRAM_PER_PROC =  205.5 / 40 *1e9
    PEAK_L2_PER_PROC = 3067.5 / 40 *1e9


    plot_roofline(proc, PEAK_FLOPS_PER_PROC, PEAK_DRAM_PER_PROC, PEAK_L2_PER_PROC, ax1, 0)

    plt.legend()

    plt.savefig('roofline.png', dpi=400)
