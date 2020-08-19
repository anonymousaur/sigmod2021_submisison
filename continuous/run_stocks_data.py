import run_dataset as rdset
import gen_workload as gwl
import os

args1 = {
    "name": "stocks",
    "datafile": "/home/ubuntu/data/stocks/eod_data/stocks_eod_daily_int64.bin",
    "ncols": 7,
    "target_dims": [(4, 100)],
    "map_dims": (5, 100),
    "k": 1,
    "alphas": [-1, 1, 5, 10, 15, 20, 30, 50],
    }

args2 = {
    "name": "stocks",
    "datafile": "/home/ubuntu/data/stocks/eod_data/stocks_eod_daily_int64.bin",
    "ncols": 7,
    "target_dims": [(2, 100)],
    "map_dims": (3, 100),
    "k": 1,
    "alphas": [-1, 1, 5, 10, 15, 20, 30, 50],
    }

args3 = {
    "name": "stocks",
    "datafile": "/home/ubuntu/data/stocks/eod_data/stocks_eod_daily_int64.bin",
    "ncols": 7,
    "target_dims": [(4, 100)],
    "map_dims": (3, 100),
    "k": 1,
    "alphas": [-1, 1, 5, 10, 15, 20, 30, 50],
    }


for args in [args1, args2, args3]:
    #rdset.run(args)
    gwl.gen_from_spec(args, "uniform", 1000,
            os.path.join(args["name"], "queries_%d_w%d_uniform.dat" % args["map_dims"]))
    gwl.gen_from_spec(args, "point", 10000,
            os.path.join(args["name"], "queries_%d_point.dat" % args["map_dims"][0]))

