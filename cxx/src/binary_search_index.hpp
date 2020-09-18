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
    : PrimaryIndexer<D>(dim), column_(dim), ready_(false) {}

template <size_t D>
size_t BinarySearchIndex<D>::LocateLeft(Scalar val) const {
    size_t l = 0, r = sorted_data_.size() - 1;
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
    size_t l = 0, r = sorted_data_.size() - 1;
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
PhysicalIndexSet BinarySearchIndex<D>::Ranges(Query<D>& q) {
    // Querying 
    auto accessed = q.filters[column_];
    if (!accessed.present) {
        return {.ranges = {{0, sorted_data_.size()}}, .list = IndexList()};
    }
    IndexRangeList ranges;
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
    return PhysicalIndexSet(ranges, IndexList());
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

    // Sort by this array instead, preserving the existing order as much as possible.
    std::stable_sort(indices.begin(), indices.end(),
        [](const std::pair<Scalar, size_t>& a, const std::pair<Scalar, size_t>& b) -> bool {
            return a.first < b.first;
        });
    bool data_modified = false;
    for (size_t i = 0; i < indices.size(); i++) {
        if (i != indices[i].second) {
            std::cout << "Index " << indices[i].second << " sorted into position " << i << std::endl;
            data_modified = true;
        }
    }

    sorted_data_ = std::vector<Scalar>(s);
    std::vector<Point<D>> data_cpy(s);
    const auto start_tmp = start;
    std::transform(indices.begin(), indices.end(), data_cpy.begin(),
            [start](const auto& ix_pair) -> Point<D> {
                return *(start + ix_pair.second);
                });

    size_t col = column_;
    std::transform(data_cpy.cbegin(), data_cpy.cend(), sorted_data_.begin(),
            [col](const Point<D>& pt) -> Scalar {
                return pt[col];
                });
    std::copy(data_cpy.cbegin(), data_cpy.cend(), start);
    AssertWithMessage(std::is_sorted(sorted_data_.begin(), sorted_data_.end()),
            "BinarySearchIndex data was not sorted");
    if (data_modified) {
        std::cout << "BinarySearchIndex modified data" << std::endl;
    } else {
        std::cout << "BinarySearchIndex did not modify data" << std::endl;
    }
    ready_ = true;
}




