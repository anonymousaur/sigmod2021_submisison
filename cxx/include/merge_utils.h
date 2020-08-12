
#pragma once

#include <vector>

#include "types.h"


class MergeUtils {
  private:
    MergeUtils() {}

  public:
    static IndexRange Intersect(const IndexRange&, const IndexRange&);

    static IndexList Intersect(const IndexList&, const IndexList&);

    static IndexRange Union(const IndexRange&, const IndexRange&);

    static IndexRange Merge(const IndexRange&, const IndexList&, size_t gap_threshold);

};

#include "../src/merge_utils.hpp"
