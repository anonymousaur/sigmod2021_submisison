import numpy as np
import numpy as np
import matplotlib
import sys
import os
from bucketer import Schema
from stash import Stasher

def run(args):
    print(args)
    name = args["name"]
    datafile = args["datafile"]
    ncols = args["ncols"]
    map_spec = args["map_spec"]
    target_specs = args["target_spec"]
    k = args["k"]
    alphas = args["alphas"]
    
    data = np.fromfile(datafile, dtype=int).reshape(-1, ncols)
    print("Loaded %d data points" % len(data))
    
    s = Stasher(data, map_spec, target_specs, nproc=15)
    print("Finished initializing Stasher, finding outliers")
    for a in alphas:
        print("\n=========== alpha = %d ===========" % a)
        run_cm = a < 0
        if run_cm:
            a = 1
        outlier_bnd = 0 if run_cm else None
        outlier_ixs, stats = s.stash_outliers_parallel(k, a, max_outliers_per_bucket=outlier_bnd)
        uix = np.unique(outlier_ixs, return_counts=True)
        print("Got %d outliers" % len(outlier_ixs))
        print("True points: %d" % stats["total_true_points"])
        init_so = stats["initial_overhead"]
        final_so = stats["final_overhead_with_index"]
        print("Overhead: %d => %d (%.02f%% reduction)" % (init_so, final_so, 100*(init_so - final_so)/init_so))
        cm_prefix = "cm" if run_cm else "a%d" % a
        prefix = "%s/%s_%s.k%d.%d_%s" % (name, name, cm_prefix, k, map_spec.dim,
                '_'.join(str(td.dim) for td in target_specs)) 
        s.write_mapping(prefix)
        
        w = open(prefix + ".stats", 'w')
        for key, val in stats.items():
            w.write(key + ":" + str(val) + "\n")
        w.close()
    
    print("\n==================================\n")

