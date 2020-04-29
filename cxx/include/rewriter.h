#pragma once

#include "types.h"

template <size_t D>
class Rewriter {
  public:
    virtual Query<D> Rewrite(const Query<D>& q) const = 0;
};

