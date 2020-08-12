/*
 * Maps a single column to another, A -> B. Any values of A that appear in the query are mapped to
 * their corresponding values of B, which is then intersected with the existing B values in the
 * query to form the full rewritten query.
 */

#pragma once

#include <string>

#include "types.h"
#include "rewriter.h"
#include <vector>

template <size_t D>
class LinearModelRewriter : public Rewriter<D> {
  public:
    // Loads the one-to-one rewriter from a file.
    LinearModelRewriter(const std::string& filename);

    size_t MappedDim() const {
        return mapped_dim_;
    }
    
    size_t TargetDim() const {
        return target_dim_;
    }

    Query<D> Rewrite(const Query<D>& q) const override;

    size_t Size() const override {
        return 3*sizeof(double) + 2*sizeof(size_t);
    }    

  protected:
    void Load(const std::string& filename);

  private:
    // The width around the line that's considered inlier territory. Everything outside of this
    // offset is an outlier.
    // For inliers:
    // target_dim is within linear_coeffs.[1, mapped_dim] +/- model_offset
    double model_offset_;
    // Coefficients of the linear map. The ith coefficient corresponds to the ith-degree term.
    std::pair<double, double> linear_coeffs_;
    // The rewriter maps values in mapped_dim_ to values in target_dim_ and adds those target_dim_
    // values to the query.
    size_t mapped_dim_;
    size_t target_dim_;
};

#include "../src/linear_model_rewriter.hpp"
