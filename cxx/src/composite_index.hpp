#include "composite_index.h"

#include <iostream>
#include <vector>
#include <cassert>
#include <chrono>

#include "merge_utils.h"

template <size_t D>
CompositeIndex<D>::CompositeIndex(size_t gap)
    : PrimaryIndexer<D>(), gap_threshold_(gap), secondary_indexes_() {}


template <size_t D>
bool CompositeIndex<D>::SetPrimaryIndex(std::unique_ptr<PrimaryIndexer<D>> index) {
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
bool CompositeIndex<D>::AddCorrelationIndex(std::unique_ptr<CorrelationIndexer<D>> index) {
    assert (index != NULL);
    correlation_indexes_.push_back(std::move(index));
    return true;
}

template <size_t D>
void CompositeIndex<D>::Init(PointIterator<D> start, PointIterator<D> end) {
    if (primary_index_ != NULL) {
        primary_index_->Init(start, end);
        auto cols = primary_index_->GetColumns();
        this->columns_.insert(cols.begin(), cols.end());
    }
    for (auto& ci : correlation_indexes_) {
        ci->Init(start, end);
        this->columns_.insert(ci->GetMappedColumn());
    }
    for (auto& si : secondary_indexes_) {
        si->Init(start, end);
        this->columns_.insert(si->GetColumn());
    }
    data_size_ = std::distance(start, end);
}

template <size_t D>
PhysicalIndexSet CompositeIndex<D>::Ranges(const Query<D>& q) const {
    PhysicalIndexSet to_scan;
    if (primary_index_ != NULL) {
        to_scan = primary_index_->Ranges(q);
    } else {
        to_scan = PhysicalIndexSet({{0, data_size_}}, {}); 
    }
    bool full_scan = to_scan.ranges.size() == 1 &&
        to_scan.ranges[0].end - to_scan.ranges[0].start == data_size_;
   
    for (auto& ci : correlation_indexes_) {
        if (!q.filters[ci->GetMappedColumn()].present) {
            continue;
        }
        PhysicalIndexSet ixs = ci->Ranges(q);
        to_scan = full_scan ? std::move(ixs) : MergeUtils::Intersect(to_scan, ixs);
        full_scan = false;
    }

    // For each secondary index, merge the secondary index matches into it.
    std::vector<size_t> matches;
    // This is to make sure we sort the secondary index result only when we absolutely have to.
    bool needs_sort = true;
    for (auto& si : secondary_indexes_) {
        if (!q.filters[si->GetColumn()].present) {
            continue;
        }
        std::vector<size_t> next_matches = si->Matches(q);
        if (matches.empty()) {
            // Don't sort yet - wait until there other other matches or ranges that need
            // intersecting.
            matches = std::move(next_matches);
        } else {
            auto start = std::chrono::high_resolution_clock::now();
            if (needs_sort) {
                std::sort(matches.begin(), matches.end());
                needs_sort = false;
            }
            auto mid = std::chrono::high_resolution_clock::now();
            std::sort(next_matches.begin(), next_matches.end());
            // Only sort if we absolutely have to
            matches = MergeUtils::Intersect(matches, next_matches);
            auto end = std::chrono::high_resolution_clock::now();
            auto tt1 = std::chrono::duration_cast<std::chrono::nanoseconds>(mid - start).count();
            auto tt2 = std::chrono::duration_cast<std::chrono::nanoseconds>(end - mid).count();
            std::cout << "Sort time: " << tt1 << "ns, "
                << "Intersect time: " << tt2 << "ns" << std::endl;
        }
    }
    size_t indexes_scanned = matches.size() + to_scan.list.size();
    for (PhysicalIndexRange p : to_scan.ranges) {
        indexes_scanned += p.end - p.start;
    }
    //if (2*indexes_scanned > data_size_) {
    //    return {{{0, data_size_}}, {}};
    //}
    // This makes sure we don't sort the secondary index results unless there are non-trivial
    // primary index results to merge with.
    if (matches.size() == 0) {
        return to_scan;
    }
    if (full_scan) {
        return {.ranges = IndexRangeList(), .list = matches};
    }
    if (needs_sort) {
        std::sort(matches.begin(), matches.end());
    }
    // Now merge the combined secondary indexes with the primary index range.
    return MergeUtils::Intersect(to_scan, {{}, matches});
}

