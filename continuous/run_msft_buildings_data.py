import run_dataset as rdset

args = {
    "name": "msft_buildings",
    "datafile": "/home/ubuntu/data/msft_buildings/msft_buildings.bin",
    "ncols": 3,
    "map_dims": (1, 30000),
    "target_dims": [(2, 30000)],
    "k": 1,
    "alphas": [1, 5, 10, 15, 20, 30, 50],
    "run_cm": False
    }

rdset.run(args)

