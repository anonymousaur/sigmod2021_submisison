import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import os
import numpy as np
import sys
from plot_utils import *
import re

def plot_range_ratio_breakdown(dataset, results, outfile):
    def get_alpha(r):
        m = re.search("_a([\.\d]+)_", r["name"])
        if m is None:
            return None
        return m.group(1)

    def get_selectivity(r):
        m = re.search("_s([\d]+)\.dat", r["workload"])
        return m.group(1) 

    sels = [get_selectivity(r) for r in results]
    assert all(x == sels[0] for x in sels), "Not all selectivities are the same!"
    sel = parse_selectivity(sels[0])
    target = sel * NUM_RECORDS[dataset] / 1e6

    def get_point_breakdown(r):
        return float(r["avg_scanned_points_in_range"]), \
                float(r["avg_scanned_points_in_list"])

    alphas = [get_alpha(r) for r in results]
    pts = [get_point_breakdown(r) for r in results]
    alphas_num = [float(a) for a in alphas]
    sort = np.argsort(alphas_num)
    print("Plotting %d results" % len(results))
    x = np.arange(len(alphas))
    plt.clf()
    labels = [alphas[ix] for ix in sort]
    frac = np.array([pts[ix][0] / (pts[ix][0] + pts[ix][1]) for ix in sort])
    ranges = np.array([pts[ix][0] for ix in sort]) / 1e6
    lists = np.array([pts[ix][1] for ix in sort]) / 1e6
    plt.bar(x, ranges, color=COLORS["corrix"], label="Range scans")
    plt.bar(x, lists, bottom=ranges, color=COLORS["corrix"], alpha=0.5, label="Point lookups")
    plt.axhline(target, color="#555555", linewidth=3, linestyle='--')
    plt.xticks(x, labels) 
    plt.xlabel(SYSTEM_NAME + r"'s $\alpha$")
    plt.ylabel("Points Scanned (x 1M)")
    plt.legend()
    plt.gca().xaxis.set_major_locator(plt.NullLocator())
    plt.gca().yaxis.set_major_locator(plt.NullLocator())
    plt.tight_layout()
    plt.savefig(outfile, pad_inches=0)

