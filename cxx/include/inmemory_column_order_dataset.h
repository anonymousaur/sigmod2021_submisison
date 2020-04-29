/**
 * A column-ordered dataset, which stores a single coordinate for each point continguously.
 * Each coordinate is stored in a separate data structure.
 */

#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <cassert>

#include "dataset.h"
#include "datacube.h"
#include "types.h"
#include "disk_storage_manager.h"
#include "buffer_manager.h"

template <size_t D>
class InMemoryColumnOrderDataset : public Dataset<D> {
  
  public:
    explicit InMemoryColumnOrderDataset(const std::vector<Point<D>>& data, const std::vector<Datacube<D>>& cubes={}) {
        // Index the datacubes by the columns they will appear in.
        columns_.resize(D + cubes.size()); 
        for (const Point<D> &p : data) {
            for (size_t d = 0; d < D; d++) {
                columns_[d].push_back(p[d]);
            }
            for (size_t i = 0; i < cubes.size(); i++) {
                Datacube<D> dc = cubes[i];
                columns_[dc.index].push_back(dc.agg->operator()(p));
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
    std::vector<std::vector<Scalar>> columns_;
};

