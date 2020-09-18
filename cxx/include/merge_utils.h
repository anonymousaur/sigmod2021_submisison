
#pragma once

#include <vector>

#include "types.h"


class MergeUtils {
  private:
    MergeUtils() {}

  public:
    static IndexRangeList Intersect(const IndexRangeList&, const IndexRangeList&);
    static std::vector<ScalarRange> Intersect(const std::vector<ScalarRange>&, const std::vector<ScalarRange>&);

    static IndexList Intersect(const IndexList&, const IndexList&);

    static PhysicalIndexSet Intersect(const PhysicalIndexSet&, const PhysicalIndexSet&);

    static IndexRangeList Union(const IndexRangeList&, const IndexRangeList&);

    static PhysicalIndexSet Union(const IndexRangeList&, const IndexList&);

    // Note: this does NOT deduplicate.
    static IndexList Union(const std::vector<const IndexList *> ix_lists);

    static IndexList Merge(const IndexRangeList&, const IndexList&);

    // Scalar ranges must be sorted.
    template <class ForwardIterator>
    static std::vector<ScalarRange> Coalesce(ForwardIterator begin, ForwardIterator end);
};

#include "../src/merge_utils.hpp"
