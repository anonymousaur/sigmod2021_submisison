from stash import Stasher
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import sys
import os
from bucketer import Bucketer

datafile = "/home/ubuntu/data/airlines/flight_data.bin"
k = 3600 * 24 * 30
data = np.fromfile(datafile, dtype=np.int64).reshape(-1, 16)
print("Loaded %d data points" % len(data))
target_dim = 0
mapped_dim = 1
target_nbuckets = 300000
mapped_nbuckets = 300000

bucketer = Bucketer(data, [(target_dim, target_nbuckets)], mode='flattened')
bucket_ids = bucketer.get_ids()

#target_ptls = np.linspace(0, 100, target_nbuckets+1)
#target_buckets = np.percentile(data[:,target_dim], target_ptls)
##target_buckets = np.linspace(data[:,target_dim].min(), data[:,target_dim].max(),
##        target_nbuckets+1)
#target_buckets[-1] = np.nextafter(data[:,target_dim].max(), np.inf)
#bucket_ids = np.digitize(data[:, target_dim], target_buckets[1:])

s = Stasher(data[:,mapped_dim], k, mapped_nbuckets, bucket_ids, mode='flattened')
print("Finished initializing Stasher, finding outliers")
outlier_ixs, stats = s.stash_outliers(10000)
uix = np.unique(outlier_ixs, return_counts=True)
print("Got %d outliers" % len(outlier_ixs))
print("True points: %d" % stats["total_true_points"])
print("Overhead: %d => %d (%d with index)" % (stats["initial_overhead"],
                                        stats["final_overhead"],
                                        stats["final_overhead_with_index"]))

