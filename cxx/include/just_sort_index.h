#pragma once

#include <vector>

#include "indexer.h"
#include "dataset.h"
#include "types.h"

/*
 * An index that just sorts the data along the given column but doesn't evaluate
 * queries. Used for the outlier indexer.
 */
template <size_t D>
class JustSortIndex : public Indexer<D> {
  public:
    JustSortIndex(size_t dim) : Indexer<D>(dim), column_(dim) {}

    std::vector<PhysicalIndexRange> Ranges(const Query<D>& q) const override {
       return {{0, data_size_}};
    }

    void Init(PointIterator<D> start, PointIterator<D> end) override;

    size_t Size() const override {
        return 2*sizeof(size_t);
    }


  protected:
    size_t column_;
    size_t data_size_; 

};

#include "../src/just_sort_index.hpp" 

