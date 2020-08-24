#pragma once

#include <vector>
#include <memory>

#include "types.h"
#include "primary_indexer.h"
#include "secondary_indexer.h"
#include "correlation_indexer.h"

/*
 * An index that combines other indexes together. It allows at most one primary index and multiple
 * secondary indexes.
 */
template <size_t D>
class CompositeIndex : public PrimaryIndexer<D> {
  public:
    CompositeIndex(size_t gap_threshold);

    bool SetPrimaryIndex(std::unique_ptr<PrimaryIndexer<D>> primary_index);

    bool AddCorrelationIndex(std::unique_ptr<CorrelationIndexer<D>> corr_index);

    bool AddSecondaryIndex(std::unique_ptr<SecondaryIndexer<D>> index);

    virtual void Init(PointIterator<D> start, PointIterator<D> end) override;

    PhysicalIndexSet Ranges(const Query<D>& q) const override; 
    
    size_t Size() const override {
        size_t s = 0;
        if (primary_index_ != NULL) {
            s += primary_index_->Size();
        }
        for (auto& ci : correlation_indexes_) {
            s += ci->Size();
        }
        for (auto& si : secondary_indexes_) {
            s += si->Size();
        }
        return s;
    }

    // Given scan ranges from the primary and secondary indexes, take their intersection to figure
    // out which indexes should actually be accessed.
    // Public for testing.
    std::vector<PhysicalIndexRange> Merge(const std::vector<PhysicalIndexRange>&,
            const std::vector<size_t>&) const;

    // Given two sets of 
    std::vector<PhysicalIndexRange> Intersect(const std::vector<PhysicalIndexRange>&,
            const std::vector<PhysicalIndexRange>&) const;

    // Given two sets of indexes to scan from secondary indexes, intersect them to get the ones that
    // actually match.
    // Public for testing.
    std::vector<size_t> Intersect(const std::vector<size_t>&, const std::vector<size_t>&) const;

  private:
    // If consecutive matching indexes are at or below this gap threshold, includes them in a single
    // range. Otherwise, truncates the old range and starts a new one.
    size_t gap_threshold_;
    // Number of points indexed. 
    size_t data_size_;

    // Once the primary index has been given to the composite index, it should no longer be
    // modified.
    std::unique_ptr<PrimaryIndexer<D>> primary_index_;
    std::vector<std::unique_ptr<SecondaryIndexer<D>>> secondary_indexes_;
    std::vector<std::unique_ptr<CorrelationIndexer<D>>> correlation_indexes_;
};

#include "../src/composite_index.hpp"
