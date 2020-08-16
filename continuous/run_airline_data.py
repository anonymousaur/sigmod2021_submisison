import run_dataset as rdset

args = {
    "name": "airline",
    "datafile": "/home/ubuntu/data/airlines/flight_data.bin",
    "ncols": 16,
    "map_dims": (3, 3600*6),
    "target_dims": [(0, 3600*6)],
    "k": 1,
    "alphas": [1, 5, 10, 15, 20, 30, 50],
    "run_cm": False
    }

rdset.run(args)

