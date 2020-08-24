/**
 * A column-ordered dataset, which stores a single coordinate for each point continguously.
 * Each coordinate is stored in a separate data structure.
 */

#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <unordered_set>
#include <cassert>

#include "dataset.h"
#include "datacube.h"
#include "types.h"

template <size_t D>
class InMemoryColumnOrderDataset : public Dataset<D> {
  
  public:
    explicit InMemoryColumnOrderDataset(std::vector<Point<D>> data) {
        // Index the datacubes by the columns they will appear in.
        for (const Point<D> &p : data) {
            for (size_t d = 0; d < D; d++) {
                columns_[d].push_back(p[d]);
            }
        }
    }

    Point<D> Get(size_t i) const override {
        Point<D> data;
        for (size_t d = 0; d < D; d++) {
            data[d] = GetCoord(i, d);
        }
        return data;
    }

    /**
     * Get coordinate `dim` of point at index `index`
     */
    Scalar GetCoord(size_t index, size_t dim) const override {
        return columns_[dim][index];
    }

    size_t Size() const override {
        return columns_[0].size();
    }

    size_t NumDims() const override {
        return D;
    }

    size_t SizeInBytes() const override {
        return Size() * NumDims() * sizeof(Scalar);
    }

private:
    std::array<std::vector<Scalar>, D> columns_;
};

