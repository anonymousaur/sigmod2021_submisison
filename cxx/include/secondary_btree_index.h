#pragma once

#include <vector>
#include <map>

#include "secondary_indexer.h"
#include "types.h"

template <size_t D>
class SecondaryBTreeIndex : public SecondaryIndexer<D> {
  public:
    SecondaryBTreeIndex(size_t dim);

    void Init(ConstPointIterator<D> start, ConstPointIterator<D> end) override;

    std::vector<size_t> Matches(const Query<D>& q) const override; 
    
    size_t Size() const override {
        size_t s = 0;
        size_t keys = 0;
        Scalar prev_key = std::numeric_limits<Scalar>::max();
        for (auto it = btree_.begin(); it != btree_.end(); it++) {
            s += sizeof(size_t);
            if (s == prev_key) {
                keys++;
            }
            prev_key = s;
        }
        // The extra factor of 2 is for the binary tree overhead
        return s + 2*keys*sizeof(Scalar);
    }

  private:
    // Number of data points
    size_t data_size_;
    // True when the index has been initialized.
    bool ready_;
    // A map from dimension value to index range where points with that dimension value that are NOT
    // outliers can be found.
    std::multimap<Scalar, size_t> btree_;
};

#include "../src/secondary_btree_index.hpp"
