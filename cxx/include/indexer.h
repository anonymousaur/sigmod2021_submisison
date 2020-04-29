#pragma once

#include <iterator>
#include <algorithm>

#include "types.h"
#include "dataset.h"
    

template <size_t D>
class Indexer {
   public:
    // Given a query bounding box, specified by the bottom left point p1 and
    // bottom-right point p2, initialize an iterator, which successively returns
    // ranges of VirtualIndices to check.
    virtual std::vector<PhysicalIndexRange> Ranges(const Query<D>& query) const = 0;

    virtual void Init(std::vector<Point<D>>* data) = 0;

    // Size of the indexer in bytes
    virtual size_t Size() const = 0;

    size_t dim;
};
