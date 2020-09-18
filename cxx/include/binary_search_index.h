#pragma once

#include <vector>

#include "primary_indexer.h"
#include "dataset.h"
#include "types.h"

/*
 * A clustered index on one dimension that uses no auxiliary data structure. The data is sorted and
 * the desired range is found using binary search on the endpoints.
 */
template <size_t D>
class BinarySearchIndex : public PrimaryIndexer<D> {
  public:
    BinarySearchIndex(size_t dim);

    void Init(PointIterator<D> start, PointIterator<D> end) override;

    PhysicalIndexSet Ranges(Query<D>& q) override; 

    void SetData(Dataset<D>* dataset_);

    size_t Size() const override {
        return 0;
    }

    void WriteStats(std::ofstream& statsfile) override {
        statsfile << "primary_index_type: rbinary_search_index_" << column_ << std::endl;
    }

  protected:
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
