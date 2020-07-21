#pragma once

#include <iterator>
#include <algorithm>
#include <vector>

#include "types.h"
#include "dataset.h"
#include "indexer.h"

template <size_t D>
class DummyIndexer {
    public:

    DummyIndexer<D>() : Indexer<D>(), data_size_() {};    
        
    // Given a query bounding box, specified by the bottom left point p1 and
    // bottom-right point p2, initialize an iterator, which successively returns
    // ranges of VirtualIndices to check.
    virtual std::vector<PhysicalIndexRange> Ranges(const Query<D>& query) const override {
        return {0, data_size_};
    }

    virtual void Init(PointIterator<D> start, PointIterator<D> end) override {
        data_size_ = std::distance(start, end);
    }

    // Size of the indexer in bytes
    virtual size_t Size() const override {
        return 0;
    }
    
    virtual std::unordered_set<size_t> GetColumns() const { return columns_; }

    protected:
    size_t data_size_;
};
