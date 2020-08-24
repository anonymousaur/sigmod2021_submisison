#pragma once

#include <iterator>
#include <algorithm>
#include <vector>

#include "types.h"
#include "dataset.h"

template <size_t D>
class Indexer {
  public:

    Indexer<D>() {}
        
    // Size of the indexer in bytes
    virtual size_t Size() const = 0;

    virtual IndexerType Type() const = 0;
};
