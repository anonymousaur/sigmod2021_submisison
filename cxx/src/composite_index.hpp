#include "composite_index.h"

#include <iostream>
#include <vector>
#include <cassert>

template <size_t D>
CompositeIndex<D>::CompositeIndex(size_t gap)
    : Indexer<D>(), gap_threshold_(gap), secondary_indexes_() {}


template <size_t D>
bool CompositeIndex<D>::SetPrimaryIndex(std::unique_ptr<Indexer<D>> index) {
    assert (index != NULL);
    primary_index_ = std::move(index);
    return true;
}

template <size_t D>
bool CompositeIndex<D>::AddSecondaryIndex(std::unique_ptr<SecondaryIndexer<D>> index) {
    assert (index != NULL);
    secondary_indexes_.push_back(std::move(index));
    return true;
}

template <size_t D>
void CompositeIndex<D>::Init(PointIterator<D> start, PointIterator<D> end) {
    if (primary_index_ != NULL) {
        primary_index_->Init(start, end);
        auto cols = primary_index_->GetColumns();
        this->columns_.insert(cols.begin(), cols.end());
    }
    for (auto& si : secondary_indexes_) {
        si->Init(start, end);
        this->columns_.insert(si->GetColumn());
    }
    data_size_ = std::distance(start, end);
    assert (gap_threshold_ > 0);
}

template <size_t D>
std::vector<PhysicalIndexRange> CompositeIndex<D>::Merge(const std::vector<PhysicalIndexRange>& ranges,
        const std::vector<size_t>& idxs) const {
    std::vector<PhysicalIndexRange> output;
    if (ranges.empty() || idxs.empty()) {
        return output;
    }
    size_t cur_range_ix = 0;
    PhysicalIndexRange running = {0, 0};
    for (size_t ix : idxs) {
       while (ix >= ranges[cur_range_ix].end) {
           cur_range_ix++;
       }
       if (cur_range_ix > ranges.size()) {
           break;
       }
       if (ix < ranges[cur_range_ix].start) {
           continue;
       }
       if (running.start >= running.end) {
           running.start = ix;
           running.end = ix+1;
       } else {
           if (running.end + gap_threshold_ > ix) {
               running.end = ix+1;
           } else {
               output.push_back(running);
               running = {ix, ix+1};
           }
       }
    }
    output.push_back(running);
    return output;
}

template <size_t D>
std::vector<size_t> CompositeIndex<D>::Intersect(const std::vector<size_t>& list1, const std::vector<size_t>& list2) const {
    std::vector<size_t> output;
    size_t i = 0, j = 0;
    while (i < list1.size() && j < list2.size()) {
        if (list1[i] < list2[j]) {
            i++;
        } else if (list1[i] > list2[j]) {
            j++;
        } else {
            output.push_back(list1[i]);
            i++;
            j++;
        }
    }
    return output;
}

template <size_t D>
std::vector<PhysicalIndexRange> CompositeIndex<D>::Ranges(const Query<D>& q) const {
    std::vector<PhysicalIndexRange> ranges = {{0, data_size_}};
    if (primary_index_ != NULL) {
        ranges = primary_index_->Ranges(q);
    }
    // Even though the query values may be sorted, the ranges we get might not be.
    std::sort(ranges.begin(), ranges.end(),
        [](const PhysicalIndexRange& a, const PhysicalIndexRange& b) {
            return a.start < b.start;
        });
    
    // For each secondary index, merge the secondary index matches into it.
    std::vector<size_t> matches;
    bool first_match = true;
    for (auto& si : secondary_indexes_) {
        if (not q.filters[si->GetColumn()].present) {
            continue;
        }
        std::vector<size_t> next_matches = si->Matches(q);
        std::sort(next_matches.begin(), next_matches.end());
        if (first_match) {
            matches = next_matches;
            first_match = false;
        } else {
            matches = Intersect(matches, next_matches);
        }
    }
    // Now merge the combined secondary indexes with the primary index range.
    if (!first_match) {
        ranges = Merge(ranges, matches);
    }
    return ranges;
}

