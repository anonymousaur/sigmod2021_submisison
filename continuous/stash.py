# Stashes points into the outlier buffer if they decrease scan overhead.

import numpy as np
import sys
import os
import time
import math 
import continuous_correlation_map as ccm

class Stasher(object):
    # pts is a numpy array of shape n x 2. The 0th column is the mapped dimension, and the 1st
    # column is the target dimension.
    # k is the average length of the ranges on the mapped dimension.
    def __init__(self, pts, k, mdim, tdim, custom_buckets=None):
        self.tdim, self.mdim = tdim, mdim
        pts = pts[:, [mdim, tdim]]
        self.maxs, self.mins = np.max(pts, axis=0), np.min(pts, axis=0)
        self.nbuckets_mapped = 100
        self.nbuckets_mapped = min(self.nbuckets_mapped,
                len(np.unique(pts[:,0])))
        self.nbuckets_target = 100
        self.nbuckets_target = min(self.nbuckets_target,
                len(np.unique(pts[:,1])))
        self.pts = pts
        print("Bucketizing")
        self.buckets = self.bucketize()
        self.overhead_list = [[] for _ in range(self.nbuckets_target)]
        kr = math.ceil(self.nbuckets_mapped * k / (self.maxs[0] - self.mins[0]))
        self.kr = min(self.nbuckets_mapped, kr)
        print("Building grid")
        self.build_grid()
        print("Building convolution grid")
        self.cg = ConvolutionGrid(self.grid, self.kr) 

    def bucketize(self):
        # Bucket along the target dimension as the primary key. Secondary key is the mapped dimension
        # value.
        colm = self.pts[:,0]
        colt = self.pts[:,1]
        sortkey = np.zeros_like(self.pts, dtype=int)
        colm_max = colm.max()
        colt_max = colt.max()
        mapped_ptls = np.linspace(0, 100, self.nbuckets_mapped+1)
        #self.mapped_buckets = np.percentile(colm, mapped_ptls)
        self.mapped_buckets = np.linspace(colm.min(), colm_max, self.nbuckets_mapped+1)
        self.mapped_buckets[-1] = np.nextafter(colm_max, colm_max+10)
        target_ptls = np.linspace(0, 100, self.nbuckets_target+1)
        #self.target_buckets = np.percentile(colt, target_ptls)
        self.target_buckets = np.linspace(colt.min(), colt_max, self.nbuckets_target+1)
        self.target_buckets[-1] = np.nextafter(colt_max, colt_max+10)
        print(self.mapped_buckets)
        print(self.target_buckets)
        sortkey[:, 1] = np.searchsorted(self.target_buckets, colt, side='right') - 1
        sortkey[:, 0] = np.searchsorted(self.mapped_buckets, colm, side='right') - 1
        #sortkey[:,1] = (self.nbuckets_target * 0.999 * \
        #        (colt - self.mins[1]) / (self.maxs[1] - self.mins[1])).astype(int)
        #sortkey[:,0] = (self.nbuckets_mapped * 0.999 * \
        #        (colm - self.mins[0]) / (self.maxs[0] - self.mins[0])).astype(int)
        return sortkey

    def build_grid(self):
        self.grid = np.zeros((self.nbuckets_target, self.nbuckets_mapped), dtype=np.int32)
        for b in self.buckets:
            self.grid[b[1], b[0]] += 1

    def buckethash(self, row, col):
        return row * self.nbuckets_mapped + col

    def stash_all_outliers(self, max_pts_to_stash):
        stash_size = 0
        buckets_to_stash = []
        so = self.cg.scan_overhead()
        data = [(0, so)]
        while True:
            s, b, ixs = self.cg.pop_best()
            if s is None:
                break
            stash_size += s
            so = self.cg.scan_overhead()
            data.append((stash_size, so))
            for i in range(ixs[1], ixs[2]):
                buckets_to_stash.append(self.buckethash(ixs[0], i))
            if stash_size > max_pts_to_stash:
                break

        all_pts_hashes = self.buckethash(self.buckets[:,1], self.buckets[:, 0])
        outlier_ix_mask = np.isin(all_pts_hashes, buckets_to_stash)
        return np.where(outlier_ix_mask)[0], data
            
    def get_mapping(self):
        m = ccm.ContinuousCorrelationMap(self.mdim, self.tdim)
        grid = self.cg.grid
        # For each mapped column, get the ranges it corresponds to
        for j in range(grid.shape[1]):
            ranges = []
            in_range = False
            rstart, rend = 0, 0
            for i in range(grid.shape[0]):
                if grid[i,j] > 0:
                    if in_range:
                        rend = self.target_buckets[i+1]
                    else:
                        in_range = True
                        rstart = self.target_buckets[i]
                        rend = self.target_buckets[i+1]
                else:
                    if (in_range):
                        ranges.append((rstart, rend))
                    in_range = False
            # finish the last range
            if in_range:
                ranges.append((rstart, rend))
            m.add_mapping(self.mapped_buckets[j],
                          self.mapped_buckets[j+1],
                          ranges)
        return m


                
