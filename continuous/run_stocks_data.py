import run_dataset as rdset
import gen_workload as gwl
import os
import numpy as np
from bucketer import Schema

target_ids = np.fromfile('stocks/target_buckets_octree_0_4.bin', dtype=np.int32)

args1 = {
    "name": "stocks",
    "datafile": "/home/ubuntu/correlations/continuous/stocks/stocks_data_sort_octree_0_4.bin",
    "ncols": 7,
    "target_spec": [Schema(4, bucket_ids=target_ids)],
    "map_spec": Schema(5, bucket_width=100),
    "k": 1,
    "alphas": [10],
    }

args2 = {
    "name": "stocks",
    "datafile": "/home/ubuntu/correlations/continuous/stocks/stocks_data_sort_octree_0_4.bin",
    "ncols": 7,
    "target_spec": [Schema(4, bucket_ids=target_ids)],
    "map_spec": Schema(3, bucket_width=100),
    "k": 1,
    "alphas": [10],
    }

args3 = {
    "name": "stocks",
    "datafile": "/home/ubuntu/correlations/continuous/stocks/stocks_data_sort_octree_0_4.bin",
    "ncols": 7,
    "target_spec": [Schema(4, bucket_ids=target_ids)],
    "map_spec": Schema(2, bucket_width=100),
    "k": 1,
    "alphas": [10],
    }


for args in [args1, args2, args3]:
    rdset.run(args)
    #gwl.gen_from_spec(args, "uniform", 1000,
    #        os.path.join(args["name"], "queries_%d_w%d_uniform.dat" % (args["map_spec"].dim,
    #        args["map_spec"].bucket_width))
    #gwl.gen_from_spec(args, "point", 10000,
    #        os.path.join(args["name"], "queries_%d_w%d_point.dat" % (args["map_spec"].dim,
    #        args["map_spec"].bucket_width))

