import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import random
from datetime import datetime, timedelta
from pytz import utc
import sys
import itertools
import os

matplotlib.rcParams['figure.figsize'] = [6,5]
plt.rc('font', size=28)          # controls default text sizes
plt.rc('axes', titlesize=30)     # fontsize of the axes title
plt.rc('axes', labelsize=28)    # fontsize of the x and y labels
plt.rc('xtick', labelsize=32)    # fontsize of the tick labels
plt.rc('ytick', labelsize=32)    # fontsize of the tick labels
plt.rc('legend', fontsize=34)    # legend fontsize

suffix = sys.argv[1]
BASE_DIR = "/home/ubuntu/correlations/continuous/chicago_taxi"
OUTLIER_DIR = "/home/ubuntu/correlations/paper/experiments/punchline/chicago_taxi"

data = np.fromfile(os.path.join(BASE_DIR, "chicago_taxi_sort_%s.bin" % suffix), dtype=int).reshape(-1, 9)
num_pts = len(data)
ixs = random.sample(range(len(data)), 100000)
print("Done sampling")

name = {
        0: "Start time",
        1: "End time",
        2: "Duration", 
        3: "Distance",
        4: "Metered Fare",
        5: "Tips",
        8: "Total Fare",
        }

def load_outliers(dim0, dim1):
    filename = "hermit_outliers_%d.bin" % dim0
    filename = os.path.join(OUTLIER_DIR, filename)
    if not os.path.exists(filename):
        return None
    yield np.fromfile(filename, dtype=int)



def split_sample(data, ixs, outliers):
    sample_outliers = np.intersect1d(ixs, outliers)
    sample_inliers = np.setdiff1d(ixs, outliers)
    return data[sample_inliers,:], data[sample_outliers,:]

def nonulls(data1, data2):
    return data1, data2
    


def plot_outliers(pair, outliers):
    print("%d => %d: %f%% outliers" % (pair[0], pair[1], 100*len(outliers)/num_pts))
    if outliers is None:
        print("Couldn't load outliers for %d => %d" % pair)
        sys.exit(0)
    inlier_data, outlier_data = split_sample(data, ixs, outliers) 
    inliers_d1, inliers_d2 = inlier_data[:, pair[0]], inlier_data[:, pair[1]]
    inliers_d1, inliers_d2 = nonulls(inliers_d1, inliers_d2)
    outliers_d1, outliers_d2 = outlier_data[:, pair[0]], outlier_data[:, pair[1]]
    outliers_d1, outliers_d2 = nonulls(outliers_d1, outliers_d2) 

    ax = plt.gca()
    ax.scatter(inliers_d1, inliers_d2, s=.01, c='blue')
    ax.scatter(outliers_d1, outliers_d2, s=.1, c='red')
    #if pair[0] not in [0, 1, 5, 6, 7, 8]:
    #    ax.set_xlim(xmin=0)
    #    ax.set_ylim(ymin=0)
    ax.set_xticklabels([])
    ax.set_yticklabels([])
    #ax.set_xlabel(name[pair[0]])
    #ax.set_ylabel(name[pair[1]])
    if pair[0] in [0, 1]:
        ax.set_xlim(1357020900, 1596227400)
    elif pair[0] == 2:
        ax.set_xlim(0, 3600)
    elif pair[0] == 3:
        ax.set_xlim(0, 400)
    elif pair[0] == 4:
        ax.set_xlim(0, 7000)
    elif pair[0] == 5:
        ax.set_xlim(0, 2000)
    elif pair[0] == 8:
        ax.set_xlim(0, 10000)
    
    if pair[1] in [0,1]:
        ax.set_ylim(1357020900, 1596227400)
    elif pair[1] == 3:
        ax.set_ylim(0, 1000)
    elif pair[1] == 4:
        ax.set_ylim(0, 7000)
    elif pair[1] == 5:
        ax.set_ylim(0, 2000)
    elif pair[1] == 8:
        ax.set_ylim(0, 10000)
    output_dir = '/home/ubuntu/correlations/paper/experiments/punchline/chicago_taxi'
    f = os.path.join(output_dir, 'plots/hermit_outliers_%s_%d_%d.png' % \
            (suffix, pair[0], pair[1]))
    plt.savefig(f, pad_inches=0)
    print("Wrote", f)

plt.clf()
#pairs = [(10, 9), (11, 9), (12, 9), (13, 9), (14, 9)]
pairs = [(4,8), (5,8)]
for pair in pairs:
    plt.clf()
    for out in load_outliers(pair[0], pair[1]):
        plot_outliers(pair, out) 

