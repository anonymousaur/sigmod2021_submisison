#pragma once

#include <vector>
#include <map>

#include "cpp-btree/btree_map.h"
#include "secondary_indexer.h"
#include "types.h"

template <size_t D>
class SecondaryBTreeIndex : public SecondaryIndexer<D> {
  public:
    SecondaryBTreeIndex(size_t dim);

    void SetIndexList(IndexList list) {
        index_subset_ = list;
        use_index_subset_ = true;
    }

    void Init(ConstPointIterator<D> start, ConstPointIterator<D> end) override;

    IndexList Matches(const Query<D>& q) const override; 
    
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
        // Each key (in a random insertion benchmark) carries an overhead of 10 bytes (for 64-bit
        // machines)
        return unique_keys*10 + total_entries*sizeof(PhysicalIndex);
    }

  private:
    // Number of data points
    size_t data_size_;
    // True when the index has been initialized.
    bool ready_;
    bool use_index_subset_;
    IndexList index_subset_;
    // A map from dimension value to index range where points with that dimension value that are
    // outliers can be found.
    btree::btree_multimap<Scalar, PhysicalIndex> btree_;
};

#include "../src/secondary_btree_index.hpp"
