#pragma once

#include <vector>
#include <unordered_map>

#include "indexer.h"
#include "types.h"

template <size_t D>
class SingleDimensionalOutlierIndex : public Indexer<D> {
  public:
    SingleDimensionalOutlierIndex(size_t dim, const std::vector<size_t>& outlier_list);

    virtual void Init(std::vector<Point<D>>*) override;

    std::vector<PhysicalIndexRange> Ranges(const Query<D>& q) const override; 
    
    size_t Size() const override {
        return (buckets_.size() + 1) * sizeof(PhysicalIndexRange);
    }

  private:
    std::vector<size_t> outlier_list_;
    // True when the index has been initialized.
    bool ready_;
    // The index of the dimension this index is built on.
    size_t column_;
    // The bucket number we reserve for outliers.
    const Scalar outlier_bucket_ = SCALAR_MAX;
    // A map from dimension value to index range where points with that dimension value that are NOT
    // outliers can be found.
    std::unordered_map<Scalar, PhysicalIndexRange> buckets_;
    // The range of data indices corresponding to outliers.
    PhysicalIndexRange outlier_range_;
};

#include "../src/outlier_index.hpp"
