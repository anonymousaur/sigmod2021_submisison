import run_dataset as rdset
import gen_workload as gwl
import os

args1 = {
    "name": "chicago_taxi",
    "datafile": "/home/ubuntu/data/chicago_taxi/chicago_taxi_trips.bin",
    "ncols": 9,
    "map_dims": (4, 100),
    "target_dims": [(2, 60)],
    "k": 1,
    "alphas": [-1, 1, 5, 10, 15, 20, 30, 50],
    }

args2 = {
    "name": "chicago_taxi",
    "datafile": "/home/ubuntu/data/chicago_taxi/chicago_taxi_trips.bin",
    "ncols": 9,
    "map_dims": (4, 100),
    "target_dims": [(3, 10)],
    "k": 1,
    "alphas": [-1, 1, 5, 10, 15, 20, 30, 50],
    }

args3 = {
    "name": "chicago_taxi",
    "datafile": "/home/ubuntu/data/chicago_taxi/chicago_taxi_trips.bin",
    "ncols": 9,
    "map_dims": (2, 60),
    "target_dims": [(3, 10)],
    "k": 1,
    "alphas": [-1, 1, 5, 10, 15, 20, 30, 50],
    }

args4 = {
    "name": "chicago_taxi",
    "datafile": "/home/ubuntu/data/chicago_taxi/chicago_taxi_trips.bin",
    "ncols": 9,
    "map_dims": (5, 100),
    "target_dims": [(3, 10)],
    "k": 1,
    "alphas": [-1, 1, 5, 10, 15, 20, 30, 50],
    }

for args in [args1, args2, args3, args4]:
    #rdset.run(args)
    gwl.gen_from_spec(args, "uniform", 1000,
            os.path.join(args["name"], "queries_%d_w%d_uniform.dat" % args["map_dims"]))
    gwl.gen_from_spec(args, "point", 10000,
            os.path.join(args["name"], "queries_%d_point.dat" % args["map_dims"][0]))

