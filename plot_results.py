import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import itertools
import numpy as np
import os
import math

cols = [3,4,5,6,8]
plot_data = np.zeros((len(cols), len(cols)))
for p in itertools.permutations(range(len(cols)), 2):
    c1, c2 = cols[p[0]], cols[p[1]]
    fname = "data/tradeoff_%d_%d_y.dat" % (c1, c2)
    if os.path.exists(fname):
        data = np.loadtxt("data/tradeoff_%d_%d_y.dat" % (c1, c2))
        plot_data[p[0], p[1]] = math.log(data[0] / np.min(data))
    else:
        plot_data[p[0], p[1]] = 0


plt.xticks(range(len(cols)), cols)
plt.yticks(range(len(cols)), cols)
plt.ylabel('Target dimension')
plt.xlabel('Mapped dimension')
plt.imshow(plot_data, interpolation='nearest', cmap="Blues")
plt.savefig("plots/all_cols.png")

