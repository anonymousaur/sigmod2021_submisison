import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import os
import numpy as np
import sys
sys.path.insert(1, '/home/ubuntu/correlations/paper/plot_scripts')
from plot_utils import *
from plot_range_ratio_breakdown import *
import re

MARKERSIZE = 10
DATASET = "chicago_taxi"

matplotlib.rcParams['figure.figsize'] = [8, 6]
plt.rc('font', size=40)          # controls default text sizes
plt.rc('axes', titlesize=40)     # fontsize of the axes title
plt.rc('axes', labelsize=24)    # fontsize of the x and y labels
plt.rc('xtick', labelsize=22)    # fontsize of the tick labels
plt.rc('ytick', labelsize=22)    # fontsize of the tick labels
plt.rc('legend', fontsize=20)    # legend fontsize

all_results = parse_results('/home/ubuntu/correlations/paper/experiments/punchline/chicago_taxi/results')
results_corrix = match(all_results, CORRIX(DATASET))
results_corrix = match(results_corrix, lambda r: 'primary_8' in r["name"])
for s in ["0001", "001", "01", "05"]:
    results_corrix_s01 = match(results_corrix, \
            lambda r: r["workload"].endswith("queries_4_5_s%s.dat" % s))
    plot_range_ratio_breakdown(DATASET, results_corrix_s01, "plots/scanned_pts_by_alpha_s%s.png" % s)



