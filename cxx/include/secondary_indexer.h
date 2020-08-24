#pragma once

#include <iterator>
#include <algorithm>
#include <vector>

#include "indexer.h"
#include "types.h"
#include "dataset.h"
   
template <size_t D>
using ConstPointIterator = typename std::vector<Point<D>>::const_iterator;

/*
 * Interface for a secondary index, which does not determine the layout of the data. Instead of
 * returning matching ranges, it can only return the indexes of the points that match.
 */
template <size_t D>
class SecondaryIndexer : public Indexer<D> {
  public:
    SecondaryIndexer<D>(size_t dim) { column_ = dim; }
      
    // Given a query bounding box, specified by the bottom left point p1 and
    // bottom-right point p2, initialize an iterator, which successively returns
    // ranges of VirtualIndices to check.
    virtual IndexList Matches(const Query<D>& query) const = 0;

    virtual void Init(ConstPointIterator<D> start,
            ConstPointIterator<D> end) = 0;

    // Size of the indexer in bytes
    virtual size_t Size() const override = 0;

    virtual size_t GetColumn() const { return column_; }

    IndexerType Type() const override { return IndexerType::Secondary; }

    protected:
    size_t column_;
};
