#include "outlier_index.h"

#include <algorithm>
#include <parallel/algorithm>
#include <cassert>
#include <unordered_map>
#include <string>
#include <iostream>

#include "types.h"

template <size_t D>
SingleDimensionalOutlierIndex<D>::SingleDimensionalOutlierIndex(size_t dim, const std::vector<size_t>& outlier_list)
    : column_(dim), buckets_(), outlier_list_(outlier_list) {}

template <size_t D>
std::vector<PhysicalIndexRange> SingleDimensionalOutlierIndex<D>::Ranges(const Query<D>& q) const {
    auto accessed = q.filters[column_];
    std::vector<PhysicalIndexRange> ranges;
    ranges.reserve(accessed.size()+1);
    for (Scalar val : accessed) {
        auto loc = buckets_.find(val);
        if (loc != buckets_.end()) {
            ranges.push_back(loc->second);
        }
    }
    ranges.push_back(outlier_range_);
    return ranges;
}

template <size_t D>
void SingleDimensionalOutlierIndex<D>::Init(std::vector<Point<D>>* data) {
    std::vector<std::pair<Scalar, size_t>> indices(data->size());
    size_t outlier_ix = 0;
    for (size_t i = 0; i < data->size(); i++) {
        Scalar ix_val = 0;
        if (outlier_ix >= outlier_list_.size() || i < outlier_list_[outlier_ix]) {
            ix_val = data->at(i)[column_];
        } else {
            ix_val = outlier_bucket_;
            outlier_ix++;
        }
        indices[i] = std::make_pair(ix_val, i);
    }

    // Sort by this array instead.
    std::sort(indices.begin(), indices.end(),
        [](const std::pair<Scalar, size_t>& a, const std::pair<Scalar, size_t>& b) -> bool {
            return a.first < b.first;
        });
    std::vector<Point<D>> data_cpy(data->size());
    std::transform(indices.begin(), indices.end(), data_cpy.begin(),
            [data](const auto& ix_pair) -> Point<D> {
                return data->at(ix_pair.second);
                });

    // Run through sorted data to build the index
    Scalar cur_ix_val = SCALAR_MIN;
    size_t cur_min_ix = 0;
    size_t cur_max_ix = 0;
    for (const auto item : indices) {
        if (cur_ix_val < item.first) {
            if (cur_ix_val == outlier_bucket_) {
                outlier_range_ = PhysicalIndexRange(cur_min_ix, cur_max_ix, false);
            } else {
                buckets_.emplace(cur_ix_val, PhysicalIndexRange(cur_min_ix, cur_max_ix, false));
            }
            cur_min_ix = cur_max_ix;
            cur_ix_val = item.first;
        }
        cur_max_ix++;
    }
    // Get the last one too
    if (cur_ix_val == outlier_bucket_) {
        outlier_range_ = PhysicalIndexRange(cur_min_ix, cur_max_ix, false);
    } else {
        buckets_.emplace(cur_ix_val, PhysicalIndexRange(cur_min_ix, cur_max_ix, false));
    }
    std::cout << "Index has " << buckets_.size() << " buckets + outliers" << std::endl;
    std::cout << "Outlier range: " << outlier_range_.start << " - " << outlier_range_.end << std::endl;
    outlier_list_.clear();    
    data->assign(data_cpy.begin(), data_cpy.end());
    ready_ = true;
}

