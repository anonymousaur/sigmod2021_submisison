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
from multiprocessing import Pool
from collections import defaultdict
from bucketer import Bucketer

OUTLIER_BUCKET = 1 << 50

# Uses the single column stasher.
# Not an instance method because we don't want it to send around tons of data.
def stash_outliers_single(args):
    # pts are specified as indexes of map buckets, in sorted order.
    pts = args["points"]
    offset = args["offset"]
    kr = args["k"]
    max_pts_to_stash = args["stash_limit"]
    target_bix = args["target_bucket_ix"];
    alpha = args["alpha"]
    stash_size = 0
    outlier_ixs = []
    if len(pts) == 0:
        return {
            "outlier_ixs": [],
            "final_overhead": 0,  # This computation is buggy 
            "initial_overhead": 0,
            "final_overhead_si": 0,
            "final_cost": 0,
            "total_true_pts": 0,
            "inlier_buckets": [],
            "outlier_iterations": []
            }

    mb_bounds = [pts[0]-kr, pts[-1]+kr+1]
    map_buckets = np.arange(mb_bounds[0], mb_bounds[1], dtype=np.int32)
    counts = np.histogram(pts, bins=map_buckets)[0]
    scs = SingleColumnStasher(counts, kr, alpha=alpha)
    initial_overhead, true_pts = scs.scan_overhead()
    start_ixs, end_ixs = [], []
    
    s, bucket_ixs = scs.pop_all_sort()
    stash_size = s
    starts = np.searchsorted(pts, bucket_ixs + mb_bounds[0])
    ends = np.searchsorted(pts, bucket_ixs + 1 + mb_bounds[0])
    #while max_pts_to_stash is None or stash_size < max_pts_to_stash:
    #    s, _, ixs = scs.pop_best()
    #    if s is None:
    #        break
    #    stash_size += s
    #    ixs1, ixs2 = ixs[0] + mb_bounds[0], ixs[1] + mb_bounds[0]
    #    start_ixs.append(ixs1)
    #    end_ixs.append(ixs2)
    #starts = np.searchsorted(pts, start_ixs) + offset
    #ends = np.searchsorted(pts, end_ixs) + offset
    for (s, e) in zip(starts, ends):
        outlier_ixs.extend(range(s, e))
    final_cost, final_so_si = scs.cost()
    inlier_buckets = scs.inlier_buckets() + mb_bounds[0]

    return {
        "target_bucket_id": target_bix,
        "outlier_ixs": outlier_ixs,
        "inlier_buckets": inlier_buckets,
        "final_overhead": 0,  # This computation is buggy 
        "initial_overhead": initial_overhead,
        "final_overhead_si": final_so_si,
        "final_cost": final_cost,
        "total_true_pts": true_pts,
        "outlier_iterations": scs.get_outliers_by_iteration()
        }

class Stasher(object):
    # pts is a numpy array of shape n x 2. The 0th column is the mapped dimension, and the 1st
    # column is the target dimension.
    # k is the average length of the ranges on the mapped dimension.
    # buckets is an integer list of bucket ids for each point (size = (len(pts),))
    def __init__(self, data, mapped_schema, target_schemas, k, alpha=1):
        map_bucketer = Bucketer([mapped_schema], data)
        self.mapped_buckets = map_bucketer.get_ids()
        target_bucketer = Bucketer(target_schemas, data)
        self.target_buckets = target_bucketer.get_ids()
        
        self.ccm = ccm.ContinuousCorrelationMap()
        self.ccm.set_mapped_bucketer(map_bucketer)
        self.ccm.set_target_bucketer(target_bucketer)
        print("Done finding bucket IDs")
        
        self.kr = k
        self.alpha = alpha
        self.bucket_ranges = self.bucketize()
        self.num_mapped_buckets = self.mapped_buckets.max()+1
        self.data = data
        self.outlier_indexes = []
        self.mapped_dim = map_bucketer.spec[0].dim
        self.target_dims = [s.dim for s in target_bucketer.spec]

    def bucketize(self):
        # Bucket along the target dimension as the primary key. Secondary key is the mapped dimension
        # value.
        # Sort by bucket id then by mapped value
        mmax = self.mapped_buckets.max() + 1
        sortkey = self.target_buckets * mmax + self.mapped_buckets
        print("Max sortkey = ", sortkey.max())
        print("# Target buckets = %d" % self.target_buckets.max())
        print("# Mapped buckets = %d" % self.mapped_buckets.max())
        self.sort_ixs = np.argsort(sortkey)
        self.mapped_buckets = self.mapped_buckets[self.sort_ixs]
        self.target_buckets = self.target_buckets[self.sort_ixs]
        uq, first, counts = np.unique(self.target_buckets, return_index=True, return_counts=True)
        self.bucket_ids = uq

        buckets = [(0,0)] * len(uq)
        for i, (u, f, c) in enumerate(zip(uq, first, counts)):
            assert c > 0
            buckets[i] = (f, f+c) 
        return buckets

    def buckethash(self, row, col):
        return row * self.nbuckets_mapped + col

    def stash_outliers_parallel(self, nproc, max_outliers_per_bucket=None):
        print("Finding outliers with nproc = %d, alpha=%d, k = %d, max outliers = %s" % \
                (nproc, self.alpha, self.kr, str(max_outliers_per_bucket)))
        cuml_outliers = []
        cuml_stats = defaultdict(float)
        
        def chunks():
            for bix, b in zip(self.bucket_ids, self.bucket_ranges):
                yield {
                    "points": self.mapped_buckets[b[0]:b[1]],
                    "offset": b[0],
                    "k": self.kr,
                    "target_bucket_ix": bix,
                    "stash_limit": max_outliers_per_bucket,
                    "alpha": self.alpha
                    }
    
        # Map each mapped bucket to a list of target buckets that have inlier points.
        bucketmap = defaultdict(list)
        outlier_seq = None
        total_cost = 0

        with Pool(processes=nproc) as pool:
            inc = 10
            bar = ShadyBar(max=len(self.bucket_ids)/inc)
            i = 0
            cs = max(1, int(len(self.bucket_ids)/nproc/50))
            for r in pool.imap_unordered(stash_outliers_single, chunks(), chunksize=cs):
                cuml_outliers.extend(np.unique(r['outlier_ixs']))
                cuml_stats["final_overhead"] += r["final_overhead"]
                cuml_stats["initial_overhead"] += r["initial_overhead"]
                cuml_stats["final_overhead_with_index"] += r["final_overhead_si"]
                cuml_stats["total_true_points"] += r["total_true_pts"]
                target_bix = r["target_bucket_id"]
                # TODO: do this at the end (in write_mapping) so we can use it for CM too
                for mb in r["inlier_buckets"]:
                    bucketmap[mb].append(target_bix)
                if outlier_seq is None:
                    outlier_seq = np.array(r["outlier_iterations"])
                else:
                    outlier_seq += np.array(r["outlier_iterations"])
                total_cost += r["final_cost"]
                i += 1
                if i % inc == 0:
                    bar.next()
            bar.finish()

        print("Outlier sequence:", outlier_seq)
        print("Total cost:", total_cost)

        self.outlier_indexes = cuml_outliers
        # ndices in the original dataset
        orig_outlier_list = self.sort_ixs[cuml_outliers]
        self.ccm.set_bucket_map(bucketmap)
        return orig_outlier_list, cuml_stats

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

    def compute_overhead(self, mapped_inlier_buckets, target_inlier_buckets):
        mapmax = mapped_inlier_buckets.max()
        uq_targets, target_counts = np.unique(target_inlier_buckets, return_counts=True)
        bar = ShadyBar("Computing full overhead", max=mapmax-self.kr+1)
        overhead = 0
        for i in range(mapmax-self.kr+1):
            matches = np.logical_and(mapped_inlier_buckets >= i, mapped_inlier_buckets < i+self.kr)
            accessed_target_buckets = target_inlier_buckets[matches]
            unique_accessed = np.unique(accessed_target_buckets)
            accessed_ixs = np.searchsorted(uq_targets, unique_accessed)
            total_pts_accessed = target_counts[accessed_ixs].sum()
            overhead += total_pts_accessed - matches.sum()
            bar.next()
        bar.finish()
        return overhead

    def write_mapping(self, prefix):
        map_str = str(self.mapped_dim) + "_" + "_".join(str(td) for td in self.target_dims)
        prefix += "." + map_str

        rel_dims = [self.mapped_dim]
        rel_dims.extend(self.target_dims)
        # Mapped dimension is always the 0th column in the new datset. The target dimensions are in
        # order of their schema after that.
        self.data = self.data[:, rel_dims]

        self.ccm.mapped_dim = 0
        self.ccm.target_dims = list(range(1, len(rel_dims)))
        mapping_file = prefix + ".mapping"
        f = open(mapping_file, 'w')
        self.ccm.write(f)
        f.close()

        ntarget_buckets = self.target_buckets.max() + 1
        self.data = self.data[self.sort_ixs, :]
        self.target_buckets[self.outlier_indexes] = OUTLIER_BUCKET
        six = np.argsort(self.target_buckets)
        self.data = self.data[six, :]
        self.target_buckets = self.target_buckets[six]
        self.mapped_buckets = self.mapped_buckets[six]
        uq_buckets, first, count = np.unique(self.target_buckets, return_index=True,
                return_counts=True)


        # Validate that the data is consistent
        prev = 0
        outlier_start_ix = len(self.data)
        for ub, f, c in zip(uq_buckets, first, count):
            assert f == prev
            prev = f + c
            if ub == OUTLIER_BUCKET:
                outlier_start_ix = f
                assert f+c == len(self.data)
        
        # Make sure we got a valid outlier list
        assert len(self.outlier_indexes) == 0 or outlier_start_ix < len(self.data)
        
        #true_overhead = self.compute_overhead(self.mapped_buckets[:outlier_start_ix],
        #        self.target_buckets[:outlier_start_ix])
        #print("Got true overhead", true_overhead)

        # Dump information for the indexer
        datafile = prefix + ".data"
        outlier_list_file = prefix + ".outliers"
        target_bucket_file = prefix + ".targets"
        
        self.data.tofile(datafile)
        np.arange(outlier_start_ix, len(self.data)).astype(int).tofile(outlier_list_file)
        
        # Write the mapping from target bucket to physical index range in the sorted data 
        fobj = open(target_bucket_file, 'w')
        fobj.write("target_index_ranges\t%d\n" % len(uq_buckets))
        for ub, f, c in zip(uq_buckets, first, count):
            if ub == OUTLIER_BUCKET:
                continue
            fobj.write("%d\t%d\t%d\n" % (ub, f, f+c))
        
