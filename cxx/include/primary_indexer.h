#pragma once

#include <iterator>
#include <algorithm>
#include <vector>

#include "types.h"
#include "dataset.h"
#include "indexer.h"


template <size_t D>
class PrimaryIndexer : public Indexer<D> {
    public:

    PrimaryIndexer<D>() : columns_() {};    
    PrimaryIndexer<D>(size_t dim) : columns_({dim}) {};
        
    // Given a query bounding box, specified by the bottom left point p1 and
    // bottom-right point p2, initialize an iterator, which successively returns
    // ranges of VirtualIndices to check.
    virtual PhysicalIndexSet Ranges(Query<D>& query) = 0;

    virtual void Init(PointIterator<D> start, PointIterator<D> end) = 0;

    // Size of the indexer in bytes
    virtual size_t Size() const override = 0;
    
    virtual std::unordered_set<size_t> GetColumns() const { return columns_; }

    IndexerType Type() const override { return IndexerType::Primary; }

    protected:
    // This may not be set on construction, but must be set after Init returns. 
    std::unordered_set<size_t> columns_;    
};
