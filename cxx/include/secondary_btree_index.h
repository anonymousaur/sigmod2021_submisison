#pragma once

#include <vector>
#include <map>

#include "secondary_indexer.h"
#include "types.h"

template <size_t D>
class SecondaryBTreeIndex : public SecondaryIndexer<D> {
  public:
    SecondaryBTreeIndex(size_t dim);
    // Only index the indices specified by 'subset'
    SecondaryBTreeIndex(size_t dim, IndexList subset);

    void Init(ConstPointIterator<D> start, ConstPointIterator<D> end) override;

    std::vector<size_t> Matches(const Query<D>& q) const override; 
    
    size_t Size() const override {
        size_t unique_keys = 0;
        size_t total_entries = btree_.size();
        Scalar prev_key = std::numeric_limits<Scalar>::lowest();
        for (auto it = btree_.begin(); it != btree_.end(); it++) {
            assert (it->first >= prev_key);
            if (it->first > prev_key) {
                unique_keys++;
                prev_key = it->first;
            }
        }
        // Based on: https://code.google.com/archive/p/cpp-btree/
        return 3*unique_keys*sizeof(Scalar) + total_entries*sizeof(size_t);
    }

  private:
    // Number of data points
    size_t data_size_;
    // True when the index has been initialized.
    bool ready_;
    IndexList index_subset_;
    // A map from dimension value to index range where points with that dimension value that are NOT
    // outliers can be found.
    std::multimap<Scalar, size_t> btree_;
};

#include "../src/secondary_btree_index.hpp"
