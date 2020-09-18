#include "secondary_btree_index.h"

#include <iostream>

template <size_t D>
SecondaryBTreeIndex<D>::SecondaryBTreeIndex(size_t dim)
    : SecondaryIndexer<D>(dim), use_index_subset_(false), btree_() {}

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
    data_size_ = std::distance(start, end);
    if (!use_index_subset_) {
        size_t i = 0;
        for (auto it = start; it != end; it++, i++) {
            btree_.insert(std::make_pair((*it)[this->column_], i));
        }
    } else {
        std::cout << "Using provided index list to load B+ tree" << std::endl;
        for (size_t ix : index_subset_) {
            btree_.insert(std::make_pair((*(start+ix))[this->column_], ix));
        }
        index_subset_.clear();
    }
    std::cout << "SecondaryBTreeIndex on " << this->column_ << " loaded " << btree_.size() << " points"
        << " and total size " << Size() << std::endl;
}

