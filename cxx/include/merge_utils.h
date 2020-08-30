
#pragma once

#include <vector>

#include "types.h"


class MergeUtils {
  private:
    MergeUtils() {}

  public:
    static IndexRangeList Intersect(const IndexRangeList&, const IndexRangeList&);

    static IndexList Intersect(const IndexList&, const IndexList&);

    static PhysicalIndexSet Intersect(const PhysicalIndexSet&, const PhysicalIndexSet&);

    static IndexRangeList Union(const IndexRangeList&, const IndexRangeList&);

    static PhysicalIndexSet Union(const IndexRangeList&, const IndexList&);

    // Note: this does NOT deduplicate.
    static IndexList Union(const std::vector<const IndexList *> ix_lists);

    static IndexList Merge(const IndexRangeList&, const IndexList&);

};

#include "../src/merge_utils.hpp"
