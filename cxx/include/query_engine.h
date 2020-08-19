/*
 * The query engine that puts everything together to answer categorial queries, i.e. queries of the
 * type ... WHERE A in (a1, a2, a3) AND ...
 */
#pragma once

#include <vector>
#include <memory>

#include "types.h"
#include "indexer.h"
#include "rewriter.h"
#include "dataset.h"
#include "visitor.h"

template <size_t D>
class QueryEngine {
  public:
    QueryEngine(std::unique_ptr<Dataset<D>> dataset,
            std::unique_ptr<Indexer<D>> indexer);
    
    QueryEngine(std::unique_ptr<Dataset<D>> dataset,
            std::unique_ptr<Indexer<D>> indexer,
            std::unique_ptr<Rewriter<D>> rewriter);

    void Execute(const Query<D>& q, Visitor<D>& visitor);

    long long ScannedPoints() const {
        return scanned_points_;
    }

  private:
    std::unique_ptr<Dataset<D>> dataset_;
    std::unique_ptr<Indexer<D>> indexer_;
    std::unique_ptr<Rewriter<D>> rewriter_;
    // The columns that are indexed by this indexer, saved for faster access.
    std::unordered_set<size_t> columns_;

    long long scanned_points_;
};

#include "../src/query_engine.hpp"

