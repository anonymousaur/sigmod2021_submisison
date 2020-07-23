# Stashes points into the outlier buffer if they decrease scan overhead.

import numpy as np
import sys
import os
import time
import math 
import continuous_correlation_map as ccm
from grid_stasher import GridStasher
from single_column_stasher import SingleColumnStasher 
from progress.bar import ShadyBar

class Stasher(object):
    # pts is a numpy array of shape n x 2. The 0th column is the mapped dimension, and the 1st
    # column is the target dimension.
    # k is the average length of the ranges on the mapped dimension.
    # buckets is an integer list of bucket ids for each point (size = (len(pts),))
    def __init__(self, pts, k, mapped_buckets, target_buckets, mode='flattened'):
        self.maxs, self.mins = np.max(pts, axis=0), np.min(pts, axis=0)
        self.pts = pts
        self.nbuckets_mapped = mapped_buckets
        self.nbuckets_mapped = min(self.nbuckets_mapped, len(np.unique(pts)))
        kr = math.ceil(self.nbuckets_mapped * k / (self.pts.max() - self.pts.min()))
        self.kr = min(self.nbuckets_mapped, kr)
        
        print("Bucketizing")
        self.buckets = self.bucketize(target_buckets, mode)
        self.overhead_list = [[] for _ in range(len(self.buckets))]

    def bucketize(self, bucket_ids, mode='flattened'):
        # Bucket along the target dimension as the primary key. Secondary key is the mapped dimension
        # value.
        mmax, mmin = self.pts.max(), self.pts.min()
        if mode == 'flattened':
            mapped_ptls = np.linspace(0, 100, self.nbuckets_mapped+1)
            self.mapped_buckets = np.percentile(self.pts, mapped_ptls)
        elif mode == 'equispaced':
            self.mapped_buckets = np.linspace(mmin, mmax, self.nbuckets_mapped+1)
        else:
            print("Mode %s not recognized" % mode)
            sys.exit(1)
        self.mapped_buckets[-1] = np.nextafter(mmax, mmax+1)
       
        # Sort by bucket id then by mapped value
        sortkey = bucket_ids*(mmax - mmin + 1) + self.pts
        self.sort_ixs = np.argsort(sortkey)
        bucket_ids = bucket_ids[self.sort_ixs]
        self.pts = self.pts[self.sort_ixs]

        max_bucket = bucket_ids[-1]+1
        buckets = [(0,0)] * max_bucket
        end = 0
        for i in range(max_bucket):
            start = end
            end = np.searchsorted(bucket_ids, i, side='right')
            buckets[i] = (start, end)
        return buckets

    def buckethash(self, row, col):
        return row * self.nbuckets_mapped + col

    # Uses the single column stasher.
    def stash_outliers(self, max_pts_to_stash):
        stash_size = 0
        initial_overhead_si = 0
        final_overhead_si = 0
        initial_overhead = 0
        final_overhead = 0
        total_true_pts = 0
        outlier_ixs = []
        bar = ShadyBar(max=len(self.buckets))
        for b in self.buckets:
            col_pts = self.pts[b[0]:b[1]]
            mb_bounds = np.searchsorted(self.mapped_buckets, [col_pts[0], col_pts[-1]])
            mb_bounds[0] = max(0, mb_bounds[0]-self.kr)
            mb_bounds[1] = min(self.nbuckets_mapped, mb_bounds[1]+self.kr+1)
            map_buckets = self.mapped_buckets[mb_bounds[0]:mb_bounds[1]] 
            counts = np.histogram(col_pts, bins=map_buckets)[0]
            first_nz = 0
            #nz = np.nonzero(counts)[0]
            #first_nz, last_nz = max(0, nz.min()-self.kr), min(len(counts),nz.max()+self.kr)
            scs = SingleColumnStasher(counts, self.kr)
            initial_so, true_pts = scs.scan_overhead()
            total_true_pts += true_pts
            initial_overhead += initial_so
            while True:
                s, _, ixs = scs.pop_best()
                if s is None:
                    break
                stash_size += s
                ixs1, ixs2 = ixs[0] + first_nz, ixs[1] + first_nz
                start = np.searchsorted(col_pts, map_buckets[ixs1]) + b[0]
                end = np.searchsorted(col_pts, map_buckets[ixs2]) + b[0]
                outlier_ixs.extend(self.sort_ixs[start:end])
                if stash_size > max_pts_to_stash:
                    break
            final_so, _ = scs.scan_overhead()
            final_so_si, _ = scs.scan_overhead()
            final_overhead += final_so
            final_overhead_si += final_so_si
            bar.next()
        bar.finish()
        overhead_stats = {
                "final_overhead": final_overhead,
                "initial_overhead": initial_overhead,
                "final_overhead_with_index": final_overhead_si,
                "total_true_points": total_true_pts
                }
        return np.unique(outlier_ixs), overhead_stats

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


                
