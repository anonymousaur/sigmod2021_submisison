#pragma once

#include <vector>
#include <map>
#include <unordered_map>
#include <string>

#include "types.h"

// Keeps track of all the information needed to tell if a new point is an inlier or outlier.
template <size_t D>
class CorrelationTracker {
  public:
    struct Action {
        DiffType diff_type;
        // This may be a target_bucket index to add or remove from the mapped correlation index,
        // depending on the action
        int32_t target_bucket_index;
        // These may be indexes to add or remove to the secondary index, depending on the action.
        std::vector<size_t> indexes;
    };
    typedef std::unordered_map<int32_t, std::vector<Action>> DiffMap;

    // The integer ID and pointer to the node in the primary index.
    typedef std::pair<int32_t, std::shared_ptr<PrimaryIndexNode>> TargetNode;


    // Create a correlation tracker by passing in two files:
    //  - one mapping file that defines the ranges of mapped buckets and which target buckets are
    //  inliers for each mapped bucket
    //  - one bucket file that lists all buckets (inliers and outliers) with the number of points in
    //  each.
    CorrelationTracker(const std::string& mapping_file, const std::string& bucket_file);

    // Records the addition of a new point in the tracker, with the given target bucket as
    // determined by the primary indexer. Returns the action for this point, which
    // the caller is expected to act upon.
    // Requires that the new point was already added to the dataset at index `insertion_ix`.
    Action Insert(PhysicalIndex insertion_ix, int32_t target_bucket);
    // It is more efficient to insert in bulk. 'target_buckets' has the same length as 'indexes',
    // with the ith element of each corresponding.
    void InsertBatchList(const TargetNode& target, const IndexList& indexes);

    // Lets the tracker know that a target bucket was split and that the correlations need to be
    // recomputed. In this case, we don't need to specify the individual points, since the entire
    // bucket will be redone.
    // Note: requires that the points in the original target bucket are already removed from the
    // mapped and secondary indexes by the caller.
    void InsertWithTargetBucketSplit(const TargetNode& original, const std::vector<TargetNode>& splits);
    
    // TODO(vikram): Fill in later
    // Action Delete(Point<D> point, int32_t target_bucket); 

    void SetDataset(std::shared_ptr<Dataset<D>> dset) {
        dataset_ = dset;
    }

    // Use the given target buckets to populate the mapping with the number of points in each.
    // The number of target buckets must match the size of the dataset.
    void Init(std::vector<size_t> target_buckets); 

    DiffMap Diffs() {
        return diffs_;
    }

    void ResetDiffs() {
        diffs_.clear();
    }

  private:

    void Load(const std::string& mapping_file);
    // Given a value, get its mapped bucekt.
    int32_t MapBucketFor(Scalar val);
    // Given diffs from updates to a target node, merge them into the current running diffs.    
    void MergeDiffs(const TargetNode& target, const DiffList& diffs, const std::map<int32_t, IndexList>& map_buckets);

    // Called as part of a target bucket split, re-indexes all the points in the split bucket.
    void InsertEntireBucket(const TargetNode& target);


    size_t column_;
    // Keeps track of information for each target bucket    
    DiffMap diffs_;
    std::unorderedmap<int32_t, TargetBucket> target_buckets_;    
    // Mapping from the left range of a mapped bucket to the bucket id.
    std::map<Scalar, int32_t> map_buckets_;
    std::shared_ptr<Dataset<D>> dataset_;

    
};

#include "../src/correlation_tracker.hpp"
