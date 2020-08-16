import run_dataset as rdset

args = {
    "name": "chicago_taxi",
    "datafile": "/home/ubuntu/data/chicago_taxi/chicago_taxi_trips.bin",
    "ncols": 9,
    "map_dims": (4, 100),
    "target_dims": [(2, 60)],
    "k": 1,
    "alphas": [1, 5, 10, 15, 20, 30, 50],
    "run_cm": True
    }

rdset.run(args)

