#include "secondary_btree_index.h"

#include <iostream>

template <size_t D>
SecondaryBTreeIndex<D>::SecondaryBTreeIndex(size_t dim)
    : SecondaryIndexer<D>(dim), btree_() {}

template <size_t D>
std::vector<size_t> SecondaryBTreeIndex<D>::Matches(const Query<D>& q) const {
    std::vector<size_t> idxs;
    auto filter = q.filters[this->column_];
    if (!filter.present) {
        std::vector<size_t> idxs(data_size_);
        std::iota(idxs.begin(), idxs.end(), 0);
        return idxs;
    }
    if (filter.is_range) {
        for (ScalarRange r : filter.ranges) {
            auto startit = btree_.lower_bound(r.first);
            auto endit = btree_.upper_bound(r.second);
            for (auto it = startit; it != endit; it++) {
                idxs.push_back(it->second);
            }
        }
    }
    else {
        for (Scalar val : filter.values) {
            auto startit = btree_.lower_bound(val);
            auto endit = btree_.upper_bound(val);
            for (auto it = startit; it != endit; it++) {
                idxs.push_back(it->second);
            }
        }
    }
    // Leave these unsorted for now. Anyone using them can sort if necessary.
    return idxs;
}

template <size_t D>
void SecondaryBTreeIndex<D>::Init(ConstPointIterator<D> start, ConstPointIterator<D> end) {
    size_t i = 0;
    for (auto it = start; it != end; it++, i++) {
        btree_.emplace((*it)[this->column_], i);
    }
    data_size_ = i;
}

