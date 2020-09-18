import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import os
import numpy as np
import sys
sys.path.insert(1, '/home/ubuntu/correlations/paper/plot_scripts')
from plot_utils import *

MARKERSIZE = 10
DATASET = "chicago_taxi"

all_results = parse_results('/home/ubuntu/correlations/paper/experiments/punchline/chicago_taxi/results')

for suffix in ["s001", "s01", "s0001", "s05"]:
    queries_btree = "queries_4_5_%s.dat" % suffix
    results_btree = match(all_results, lambda r: "workload" in r and \
            r["workload"].endswith(queries_btree))

    queries_octree = "queries_1_3_4_5_%s.dat" % suffix
    results_octree = match(all_results, lambda r: "workload" in r and \
            r["workload"].endswith(queries_octree))

    full_scan = match(all_results, lambda r: "_full" in r["name"])[:1]
    
    match_p1000 = match(results_octree, lambda r: 'p1000_' in r["name"] or r["name"].endswith('p1000'))
    plot_pareto(DATASET, match_p1000 + full_scan, "plots/%s_pareto_p1000_%s.png" % (DATASET, suffix))
    
    match_btree = match(results_btree, lambda r: 'primary_8' in r["name"])
    plot_pareto(DATASET, match_btree+ full_scan, "plots/%s_pareto_primary_8_%s.png" % (DATASET, suffix))


