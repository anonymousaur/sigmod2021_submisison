#include "binary_search_index.h"

#include <algorithm>
#include <parallel/algorithm>
#include <cassert>
#include <string>
#include <iostream>

#include "types.h"
#include "utils.h"

template <size_t D>
BinarySearchIndex<D>::BinarySearchIndex(size_t dim)
    : Indexer<D>(dim), column_(dim), ready_(false) {}

template <size_t D>
size_t BinarySearchIndex<D>::LocateLeft(Scalar val) const {
    size_t l = 0, r = sorted_data_.size();
    while (l < r) {
        size_t m = (l + r)/2;
        if (sorted_data_[m] < val) {
            l = m+1;
        } else {
            r = m;
        }
    }
    return l;
}

template <size_t D>
size_t BinarySearchIndex<D>::LocateRight(Scalar val) const {
    size_t l = 0, r = sorted_data_.size();
    while (l < r) {
        size_t m = (l + r)/2;
        if (sorted_data_[m] <= val) {
            l = m+1;
        } else {
            r = m;
        }
    }
    return l;
}

template <size_t D>
std::vector<PhysicalIndexRange> BinarySearchIndex<D>::Ranges(const Query<D>& q) const {
    auto accessed = q.filters[column_];
    if (!accessed.present) {
        return {{0, sorted_data_.size()}};
    }
    std::vector<PhysicalIndexRange> ranges;
    if (accessed.is_range) {
        ranges.reserve(accessed.ranges.size());
        size_t added = 0;
        for (ScalarRange r : accessed.ranges) {
            size_t lix = LocateLeft(r.first);
            size_t rix = LocateRight(r.second);
            if (added > 0 && lix == ranges[added-1].end) {
                ranges[added-1].end = rix;
            } else if (rix > lix) {
                ranges.emplace_back(lix, rix);
                added++;
            }
        }
    } else {
        ranges.reserve(accessed.values.size());
        size_t added = 0;
        for (Scalar val : accessed.values) {
            size_t lix = LocateLeft(val);
            size_t rix = LocateRight(val);
            if (added > 0 && lix == ranges[added-1].end) {
                ranges[added-1].end = rix;
            } else if (rix > lix) {
                ranges.emplace_back(lix, rix);
                added++;
            }
        }
    }
    return ranges;
}

template <size_t D>
void BinarySearchIndex<D>::Init(PointIterator<D> start, PointIterator<D> end) {
    size_t s = std::distance(start, end);
    std::vector<std::pair<Scalar, size_t>> indices(s);
    size_t i = 0;
    for (auto it = start; it != end; it++, i++) {
        Scalar ix_val = (*it)[column_];
        indices[i] = std::make_pair(ix_val, i);
    }

    // Sort by this array instead.
    std::sort(indices.begin(), indices.end(),
        [](const std::pair<Scalar, size_t>& a, const std::pair<Scalar, size_t>& b) -> bool {
            return a.first < b.first;
        });

    sorted_data_ = std::vector<Scalar>(s);
    std::vector<Point<D>> data_cpy(s);
    const auto start_tmp = start;
    std::transform(indices.begin(), indices.end(), data_cpy.begin(),
            [start](const auto& ix_pair) -> Point<D> {
                return *(start + ix_pair.second);
                });

    size_t col = this->column_;
    std::transform(data_cpy.cbegin(), data_cpy.cend(), sorted_data_.begin(),
            [col](const Point<D>& pt) -> Scalar {
                return pt[col];
                });
    std::copy(data_cpy.cbegin(), data_cpy.cend(), start);
    ready_ = true;
}




