#pragma once

#include <vector>
#include <map>
#include <string>

#include "secondary_indexer.h"
#include "types.h"

template <size_t D>
class BucketedSecondaryIndex : public SecondaryIndexer<D> {
  public:
    enum BucketingStrategy { UNSET, CONST_WIDTH, FROM_FILE }; 

    // Sets the bucket width to use for bucketing the outliers.
    BucketedSecondaryIndex(size_t dim);
    // Optionally, allows to specify a subset of points to index.
    BucketedSecondaryIndex(size_t dim, IndexList subset);

    void Init(ConstPointIterator<D> start, ConstPointIterator<D> end) override;

    std::vector<size_t> Matches(const Query<D>& q) const override; 

    void SetBucketFile(const std::string& filename);
    void SetBucketWidth(Scalar width);
    
    size_t Size() const override {
        size_t s = 0;
        size_t keys = 0;
        for (auto it = buckets_.begin(); it != buckets_.end(); it++) {
            s += it->second.size();
            keys++;
        }
        s += keys * sizeof(Scalar) * 3; // Overhead
        return s;
    }

  private:
    // Add to the buckets_ map, but don't order this value with the other existing ones.
    void InsertUnsorted(size_t index, Scalar v);
    
    BucketingStrategy bucket_strat_;
    size_t data_size_;
    // True when the index has been initialized.
    Scalar bucket_width_;
    bool ready_;
    IndexList index_subset_;
    // For each map bucket (key is the start value of the map bucket range), 
    std::map<Scalar, IndexList> buckets_;
};

#include "../src/bucketed_secondary_index.hpp"
