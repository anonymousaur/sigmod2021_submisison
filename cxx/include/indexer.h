#pragma once

#include <iterator>
#include <algorithm>
#include <vector>

#include "types.h"
#include "dataset.h"

template <size_t D>
using PointIterator = typename std::vector<Point<D>>::iterator;

template <size_t D>
class Indexer {
    public:

    Indexer<D>() : columns_() {};    
    Indexer<D>(size_t dim) : columns_({dim}) {};
        
    // Given a query bounding box, specified by the bottom left point p1 and
    // bottom-right point p2, initialize an iterator, which successively returns
    // ranges of VirtualIndices to check.
    virtual std::vector<PhysicalIndexRange> Ranges(const Query<D>& query) const = 0;

    virtual void Init(PointIterator<D> start, PointIterator<D> end) = 0;

    // Size of the indexer in bytes
    virtual size_t Size() const = 0;
    
    virtual std::unordered_set<size_t> GetColumns() const { return columns_; }

    protected:
    // This may not be set on construction, but must be set after Init returns. 
    std::unordered_set<size_t> columns_;    
};
