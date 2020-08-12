#pragma once

#include <vector>

#include "indexer.h"
#include "dataset.h"
#include "types.h"

/*
 * A clustered index on one dimension that uses no auxiliary data structure. The data is sorted and
 * the desired range is found using binary search on the endpoints.
 */
template <size_t D>
class BinarySearchIndex : public Indexer<D> {
  public:
    BinarySearchIndex(size_t dim);

    void Init(PointIterator<D> start, PointIterator<D> end) override;

    std::vector<PhysicalIndexRange> Ranges(const Query<D>& q) const override; 

    void SetData(Dataset<D>* dataset_);

    size_t Size() const override {
        return 0;
    }

  private:
    size_t LocateLeft(Scalar val) const;
    size_t LocateRight(Scalar val) const;

    // Used for internal purposes only.
    size_t column_;
    // True when the index has been initialized.
    bool ready_;
    // The dataset that we will use to do the binary search.
    std::vector<Scalar> sorted_data_;

};

#include "../src/binary_search_index.hpp"
