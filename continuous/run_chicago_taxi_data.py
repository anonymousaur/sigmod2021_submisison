import run_dataset as rdset
import gen_workload as gwl
import os
import numpy as np
from bucketer import Schema

target_ids = np.fromfile('chicago_taxi_combined/target_buckets_octree_0_2_4.bin', dtype=np.int32)

args2 = {
    "name": "chicago_taxi_combined",
    "datafile": "/home/ubuntu/correlations/continuous/chicago_taxi_combined/chicago_taxi_sort_octree_0_2_4.bin",
    "ncols": 9,
    "map_spec": Schema(3, bucket_width=10),
    "target_spec": [Schema(4, bucket_ids=target_ids)],
    "k": 1,
    "alphas": [10], #-1, 1, 5, 10, 15, 20, 30, 50],
    }

args3 = {
    "name": "chicago_taxi_combined",
    "datafile": "/home/ubuntu/correlations/continuous/chicago_taxi_combined/chicago_taxi_sort_octree_0_2_4.bin",
    "ncols": 9,
    "map_spec": Schema(5, bucket_width=100),
    "target_spec": [Schema(4, bucket_ids=target_ids)],
    "k": 1,
    "alphas": [10],
    }

args4 = {
    "name": "chicago_taxi_combined",
    "datafile": "/home/ubuntu/correlations/continuous/chicago_taxi_combined/chicago_taxi_sort_octree_0_2_4.bin",
    "ncols": 9,
    "map_spec": Schema(5, bucket_width=100),
    "target_spec": [Schema(3, bucket_ids=target_ids)],
    "k": 1,
    "alphas": [10],
    }

for args in [args2, args3]:
    rdset.run(args)
    #gwl.gen_from_spec(args, "uniform", 1000,
    #        os.path.join(args["name"], "queries_%d_w%d_uniform.dat" % args["map_spec"]))
    #gwl.gen_from_spec(args, "point", 10000,
    #        os.path.join(args["name"], "queries_%d_point.dat" % args["map_spec"][0]))

