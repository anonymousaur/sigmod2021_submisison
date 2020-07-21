/**
 * An abstraction for a dataset that allows access to points at a particular index.
 */

#include <unordered_set>

#include "types.h"

#pragma once

template <size_t D>
class Dataset { 
  public:
    // Get all coordinates of a particular data point.
    virtual Point<D> Get(size_t id) const = 0;
    // Get a single coordinate (in dimension `dim`) of the data point at location `id`.
    virtual Scalar GetCoord(size_t id, size_t dim) const = 0;
    
    // Return a bitstring denoting all the indices between start (inclusive) and end (exclusive),
    // whose coordinate at dimension `dim` is in the given set of values `vset`.
    // Precondition: end - start <= 64. If end - start < 64, the remaining most significant bits should be 0.
    virtual uint64_t GetCoordInSet(size_t start, size_t end, size_t dim, const std::unordered_set<Scalar>& vset) const {
        uint64_t valid = 0;
        for (size_t i = start; i < end; i++) {
            valid <<= 1;
            Scalar c = GetCoord(i, dim);
            valid |= (vset.find(c) != vset.end());
        }
        return valid;
    }
    
    // Return a bitstring denoting all the indices between start (inclusive) and end (exclusive),
    // whose coordinate at dimension `dim` falls between low (inclusive) and high (exclusive)
    // Precondition: end - start <= 64. If end - start < 64, the remaining most significant bits should be 0.
    virtual uint64_t GetCoordInRange(size_t start, size_t end, size_t dim, Scalar low, Scalar high) const {
        uint64_t valid = 0;
        for (size_t i = start; i < end; i++) {
            valid <<= 1;
            Scalar c = GetCoord(i, dim);
            valid |= c >= low && c < high;
        }
        return valid;
    }
    
    // Return a bitstring denoting all the indices between start (inclusive) and end (exclusive),
    // whose coordinate at dimension `dim` falls in one of the ranges in `rset`. 
    // Precondition: end - start <= 64. If end - start < 64, the remaining most significant bits should be 0.
    virtual uint64_t GetCoordInRanges(size_t start, size_t end, size_t dim, const RangeSet& rset) const {
        uint64_t valid = 0;
        for (size_t i = start; i < end; i++) {
            valid <<= 1;
            Scalar c = GetCoord(i, dim);
            valid |= rset.Test(c);
        }
        return valid;
    }
    
    virtual size_t Size() const = 0;
    virtual size_t NumDims() const = 0;
    // size of the dataset in bytes
    virtual size_t SizeInBytes() const = 0;
};

/*
 * A reference to a point, specified with a database pointer and
 * an index. The individual coordinates can be accessed using:
 *   dataset->GetCoord(idx, <dim>)
 */
template <size_t D>
struct PointRef {
    const Dataset<D>* dataset;
    size_t idx;
    PointRef(const Dataset<D>* d, size_t i) {
        dataset = d;
        idx = i;
    }
};

