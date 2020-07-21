from stash import Stasher
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import sys
import os


datafile = sys.argv[1]
k = float(sys.argv[2])
data = np.loadtxt(datafile)

s = Stasher(data, k)
print("Finished initializing Stasher, finding outliers")
inpts, outpts, data = s.stash_all_outliers(10000)

plt.scatter(inpts[:, 0], inpts[:, 1], s=1, c='blue')
plt.scatter(outpts[:, 0], outpts[:, 1], s=1, c='red')

fname = os.path.splitext(os.path.basename(datafile))[0]
plt.savefig(fname + "_k%.1f.png" % k)
data = np.array(data)
np.savetxt(fname + "_opt_data_k%.1f.dat" % k, data)

plt.clf()
plt.plot(data[:, 0], data[:, 1], '-')
plt.savefig(fname + "_descent_k%.1f.png" % k)

