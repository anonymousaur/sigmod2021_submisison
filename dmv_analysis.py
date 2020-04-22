import numpy as np
from collections import defaultdict


# Return the data (in string format) from the file
def extract(cols):
    filename = "dmv.csv"
    data = np.loadtxt(filename, dtype=str, delimiter=',', skiprows=1,
            usecols=cols)
    print("Finished loading data")
    return data

# Return the encoded data (ints) from the binary file
def extract_from_dump(cols):
    filename = "dmv_encoded.npy"
    data = np.load(filename)
    return data[:,cols]
    print("Finished loading data, shape: ", data.shape)
    return data

cols = extract((5, 4))
mzip = {}
vals = set()
for i,c in enumerate(cols):
    if c[0] not in mzip:
        mzip[c[0]] = defaultdict(int)
    mzip[c[0]][c[1]] += 1
    vals.add(c[1])

print("First col has %d unique vals" % len(mzip))
print("Second col has %d unique vals" % len(vals))
minor_cnt = 0
all_cnt = 0
for k, v in mzip.items():
    if len(v) > 1:
        vals = list(v.values())
        minor_cnt += sum(vals) - max(vals)
        all_cnt += sum(vals)
        #print(k,v)

print("Outliers: %d / %d = %.03f" % (all_cnt, minor_cnt,
    float(minor_cnt) / all_cnt))

