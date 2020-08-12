from stash import Stasher
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import sys
import os
from bucketer import Schema

datafile = "/home/ubuntu/data/msft_buildings/msft_buildings.bin"
#datafile = "/home/ubuntu/data/stocks/eod_data/stocks_eod_daily_sample.bin"
data = np.fromfile(datafile, dtype=int).reshape(-1, 3)
print("Loaded %d data points" % len(data))

target_specs = [Schema(2, bucket_width=30000)]
map_spec = Schema(1, bucket_width=30000)
# Each query goes over one bucket in the mapped dimension
k = 1

s = Stasher(data, map_spec, target_specs, k, alpha=1.8)
print("Finished initializing Stasher, finding outliers")
outlier_ixs, stats = s.stash_outliers_parallel(15, max_outliers_per_bucket=None)
uix = np.unique(outlier_ixs, return_counts=True)
print("Got %d outliers" % len(outlier_ixs))
print("True points: %d" % stats["total_true_points"])
print("Overhead: %d => %d (%d with index)" % (stats["initial_overhead"],
                                        stats["final_overhead"],
                                        stats["final_overhead_with_index"]))
s.write_mapping("msft_buildings/msft_buildings_w30k")
