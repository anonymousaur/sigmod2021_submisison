import run_dataset as rdset

args = {
    "name": "stocks",
    "datafile": "/home/ubuntu/data/stocks/eod_data/stocks_eod_daily_int64.bin",
    "ncols": 7,
    "target_dims": [(4, 100)],
    "map_dims": (5, 100),
    "k": 1,
    "alphas": [1, 5, 10, 15, 20, 30, 50],
    "run_cm": False
    }

rdset.run(args)

