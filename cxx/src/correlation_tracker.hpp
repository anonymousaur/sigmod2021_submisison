#include "correlation_tracker.h"

#include "utils.h"
#include "file_utils.h"

template <size_t D>
CorrelationTracker<D>::CorrelationTracker(const std::string& mapping_file)
    : column_(), storage_weight_(1.0), map_buckets_(), target_buckets_() {
    Load(mapping_file);
}

template <size_t D>
void CorrelationTracker<D>::Load(const std::string& mapping_file, const std::string& bucket_file) {
    std::ifstream file(mapping_file);
    assert (file.is_open());
    assert (FileUtils::NextLine(file) == "continuous-0");
    
    auto header = FileUtils::NextArray<std::string>(file, 3);
    assert (header[0] == "source");
    column_ = std::stoi(header[1]);
    size_t s = std::stoi(header[2]);
    // The last element is the rightmost bound on the data.
    // Read everything as a double to be safe and then convert to scalar
    
    std::cout << "CorrelationTracker: reading " << s << " mapped buckets" << std::endl;
    for (size_t i = 0; i < s; i++) {
        auto arr = FileUtils::NextArray<double>(file, 3);
        size_t mapix = (size_t)arr[0];
        ScalarRange r = {(Scalar)arr[1], (Scalar)arr[2]};
        map_buckets_.emplace(mapix, r);
    }

    header = FileUtils::NextArray<std::string>(file, 2);
    std::cout << header[0] << std::endl;
    assert (header[0] == "mapping");
    s = std::stoi(header[1]);
    std::unordered_map<int32_t, std::set<int32_t>> inlier_buckets;
    for (size_t i = 0; i < s; i++) {
        auto arr = FileUtils::NextArray<size_t>(file);
        for (auto it = arr.begin() + 1; it != end(); it++) {
            auto loc = inlier_buckets.find(*it);
            if (loc == inlier_buckets.end()) {
                loc == inlier_buckets.emplace(*it, std::set<int32_t>{});
            }
            loc->second.push_back(arr[0]);
        }
    } 
    std::cout << "Finished loading mapping-file" << std::endl;
   
    std::ifstream bucketfile(bucket_file); 
    assert (bucketfile.is_open());
    header = FileUtils::NextArray<std::string>(bucketfile, 2);
    assert (header[0] == "all-buckets");
    s = std::stoi(header[1]);
    for (size_t i = 0; i < s; i++) {
        auto arr = FileUtils::NextArray<int32_t>(bucketfile, 3);
        int32_t target_bucket_ix = arr[0];
        int32_t mapped_bucket_ix = arr[1];
        int32_t num_points = arr[2]; 
        bool outlier_bucket = true;
        auto loc = inlier_buckets.find(target_bucket_ix);
        assert (loc != inlier_buckets.end());
        if (loc->second.find(mapped_bucket_ix) != loc->second.end()) {
            outlier_bucket = false;
        }
        loc = target_buckets_.find(target_bucket_ix);
        if (loc == target_buckets_.end()) {
            loc = target_buckets_.emplace(loc, TargetBucket());
        }
        if (outlier_bucket) {
            loc->AddPoints(mapped_bucket_ix, num_points);
        } else {
            loc->AddPoints(mapped_bucket_ix, num_points);
        }
    }
    for (auto it = inlier_buckets.begin(); it != inlier_buckets.end(); it++) {
        size_t want_size = it->second.size();
        size_t got_size = target_buckets_[it->first].num_inlier_buckets_;
        AssertWithMessage(want_size == got_size, "Disagreement over inlier / outlier assignment");
    }
    std::cout << "Finished loading target-buckets-file" << std::endl;
}

template <size_t D>
int32_t CorrelationTracker<D>::MapBucketFor(Scalar val) {
    auto loc = map_buckets_.upper_bound(val);
    AssertWithMessage(loc != map_buckets_.begin(), "Point found outside existing domain");
    loc--;
    return loc->second;
}

template <size_t D>
void CorrelationTracker<D>::InsertWithTargetBucketSplit(const TargetNode& original,
        const std::vector<TargetNode> splits) {
    auto orig_loc = target_buckets_.find(original.first);
    assert (orig_loc != target_buckets_.end());
    TargetBucket tb = orig_loc->second;
    target_buckets_.erase(orig_loc);
    for (const auto& new_bucket : splits) {
        auto loc = target_buckets_.find(new_bucket.first);
        AssertWithMessage(loc == target_buckets_.end(), "Split added a bucket with an existing ID");
        TargetBucket tb(storage_factor_);
        target_buckets_.emplace(new_bucket.first, tb);
        InsertEntireBucket(new_bucket);
    }
}

template <size_t D>
void CorrelationTracker<D>::InsertBatchList(const TargetNode& target,
        const IndexList& indexes) {
    // None of these insertions resulted in a target bucket split.
    std::map<int32_t, int32_t> added_pts;
    std::map<int32_t, IndexList> map_buckets;
    for (PhysicalIndex p : indexes) {
        Scalar val = dataset_->GetCoord(p, column_);
        int32_t map_ix = map_buckets_.upper_bound(val)->second;
        added_pts[map_ix]++;
        map_buckets[map_ix].push_back(p);
    }
    TargetBucket& tb = target_buckets_[target.first];
    std::vector<std::pair<int32_t, int32_t>> batch_pts(added_pts.begin(), added_pts.end());
    tb.AddPointsBatch(batch_pts);
    MergeDiffs(target, tb.Diffs(), map_buckets);
    tb.ResetDiffs();
}

template <size_t D>
std::vector<Action> CorrelationTracker<D>::InsertEntireBucket(const TargetNode& target) {
    std::vector<Scalar> vals;
    vals.reserve(range.end - range.start);
    for (size_t p = target.second->StartOffset(); p < target.second->EndOffset(); p++) {
        Scalar val = dataset_->GetCoord(p, column_);
        int32_t map_ix = map_buckets_.upper_bound(val)->second;
        added_pts[map_ix]++;
        map_buckets[ix].push_back(p);
    }
    TargetBucket& tb = target_buckets_[target.first];
    std::vector<std::pair<int32_t, int32_t>> batch_pts(added_pts.begin(), added_pts.end());
    tb.AddPointsBatch(batch_pts);
    MergeDiffs(target, tb.Diffs(), map_buckets);
    tb.ResetDiffs();
}

template <size_t D>
void CorrelationTracker<D>::MergeDiffs(const TargetNode& target, const DiffList& diffs,
        const std::map<int32_t, IndexList>& map_buckets) {
    std::map<int32_t, IndexList> bucket_scan;
    for (const auto& diff : diffs) {
        switch (diff.second) {
            case NEW_INLIER:
                Action a = {.diff_type = NEW_INLIER, .target_bucket_index = target_bucket_ix,
                    .indexes={}};
                diffs_[diff.first].push_back(a);
                break;
            case NEW_OUTLIER:
                Action a = {.diff_type = NEW_OUTLIER, .target_bucket_index = target_bucket_ix,
                    .indexes = map_buckets[diff.first]};
                AssertWithMessage(a.indexes.size() > 0, "New outlier bucket with no points");
                diffs_[diff.first].push_back(a);
                break;
            case REMAIN_INLIER:
                Action a = {.diff_type = REMAIN_INLIER, .target_bucket_index = target_bucket_ix,
                    .indexes = {}};
                diffs_[diff.first].push_back(a);
                break;
            case REMAIN_OUTLIER:
                Action a = {.diff_type = REMAIN_OUTLIER, .target_bucket_index = target_bucket_ix,
                    .indexes = map_buckets[diff.first]};
                AssertWithMessage(a.indexes.size() > 0, "REMAIN_OUTLIER bucket with no new points");
                diffs_[diff.first].push_back(a);
                break;
            case INLIER_TO_OUTLIER:
                // This is a special case - we have to scan the entire bucket to find the indexes
                // that we need to move
                bucket_scan[diff.first] = {};
                break;
            case OUTLIER_TO_INLIER:
                Action a = {.diff_type = OUTLIER_TO_INLIER, .target_bucket_index = target_bucket_ix,
                    .indexes = {}};
                diffs_[diff.first].push_back(a);
                break;
            default:
                AssertWithMessage(false, "Unrecognized Diff type encountered");
        } 
    }
    
    // Now, scan the whole bucket for the remaining buckets that we have to stash entirely.
    if (!bucket_scan.empty()) {
        for (PhysicalIndex p = target.second->StartOffset(); p < target->second->EndOffset(); p++) {
            Scalar val = dataset_->GetCoord(p, column_);
            int32_t map_ix = MapBucketFor(val);
            auto loc = bucket_scan.find(map_ix);
            if (loc != bucket_scan.end()) {
                loc->second.push_back(p);
            }
        }
        for (auto it = bucket_scan.begin(); it != bucket_scan.end(); it++) {
            Action a = {.diff_type = INLIER_TO_OUTLIER, .target_bucket_index = target.first,
                .indexes = std::move(it->second)};
            diffs_[it->first].push_back(a);
        }
    } 
}



