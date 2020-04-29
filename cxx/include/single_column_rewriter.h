/*
 * Maps a single column to another, A -> B. Any values of A that appear in the query are mapped to
 * their corresponding values of B, which is then intersected with the existing B values in the
 * query to form the full rewritten query.
 */

#pragma once

#include <string>

#include "types.h"
#include "rewriter.h"

template <size_t D>
class SingleColumnRewriter : public Rewriter<D> {
  public:
    // Loads the one-to-one rewriter from a file.
    SingleColumnRewriter(const std::string& filename);

    size_t MappedDim() const {
        return mapped_dim_;
    }
    
    size_t TargetDim() const {
        return target_dim_;
    }

    Query<D> Rewrite(const Query<D>& q) const override;

  protected:
    void Load(const std::string& filename);

  private:
    std::unordered_map<Scalar, std::vector<Scalar>> cmap_;
    // The rewriter maps values in mapped_dim_ to values in target_dim_ and adds those target_dim_
    // values to the query.
    size_t mapped_dim_;
    size_t target_dim_;
};

#include "../src/single_column_rewriter.hpp"
