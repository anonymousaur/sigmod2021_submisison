/*
 * The query engine that puts everything together to answer categorial queries, i.e. queries of the
 * type ... WHERE A in (a1, a2, a3) AND ...
 */
#pragma once

#include <vector>

#include "types.h"
#include "indexer.h"
#include "rewriter.h"
#include "dataset.h"
#include "visitor.h"

template <size_t D>
class CategoricalQueryEngine {
  public:
    CategoricalQueryEngine(Dataset<D>* dataset, Indexer<D>* indexer, Rewriter<D>* rewriter);

    void Execute(const Query<D>& q, Visitor<D>& visitor);

  private:
    Dataset<D>* dataset_;
    Indexer<D>* indexer_;
    Rewriter<D>* rewriter_;
};

#include "../src/categorical_query_engine.hpp"

