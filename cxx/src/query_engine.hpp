#include "query_engine.h"

#include <vector>
#include <unordered_set>

#include "types.h"
#include "visitor.h"

template <size_t D>
QueryEngine<D>::QueryEngine(
        std::unique_ptr<Dataset<D>> dataset,
        std::unique_ptr<Indexer<D>> indexer)
    : QueryEngine<D>(std::move(dataset), std::move(indexer), nullptr) {}

template <size_t D>
QueryEngine<D>::QueryEngine(
        std::unique_ptr<Dataset<D>> dataset,
        std::unique_ptr<Indexer<D>> indexer,
        std::unique_ptr<Rewriter<D>> rewriter)
    : dataset_(std::move(dataset)),
      indexer_(std::move(indexer)),
      rewriter_(std::move(rewriter)),
      columns_(indexer_->GetColumns()) {}

template <size_t D>
void QueryEngine<D>::Execute(const Query<D>& q, Visitor<D>& visitor) {
    std::vector<size_t> categorical_query_dimensions;
    std::vector<size_t> range_query_dimensions;
    std::vector<std::unordered_set<Scalar>> value_sets;
    // Because we're dealing with rewriting, ranges are inexact.
    bool exact = false;
    for (size_t i = 0; i < dataset_->NumDims(); i++) {
        if (q.filters[i].present) {
            // If the filtered dimension isn't indexed, this isn't an exact query anymore.
            if (q.filters[i].is_range) {
                assert (q.filters[i].ranges.size() == 1);
                range_query_dimensions.push_back(i);
            } else {
                categorical_query_dimensions.push_back(i);
                value_sets.emplace_back(q.filters[i].values.begin(),
                        q.filters[i].values.end());
            }
        }
    }
    Query<D> newq = rewriter_ ? rewriter_->Rewrite(q) : q;
    for (PhysicalIndexRange range : indexer_->Ranges(q)) {
        if (exact) {
            visitor.visitExactRange(dataset_.get(), range.start, range.end);
        }
        else {
            for (PhysicalIndex p = range.start; p < range.end; p += 64UL) {
                size_t true_end = std::min(range.end, p + 64UL);
                // A way to get the last true_end - p bits set to 1.
                uint64_t valids = 1ULL + (((1ULL << (true_end - p - 1)) - 1ULL) << 1);
                for (size_t i = 0; i < categorical_query_dimensions.size(); i++) {
                    valids &= dataset_->GetCoordInSet(p, true_end,
                            categorical_query_dimensions[i], value_sets[i]);
                }
                for (size_t d : range_query_dimensions) {
                    auto r = q.filters[d].ranges[0];
                    valids &= dataset_->GetCoordInRange(p, true_end,
                            d, r.first, r.second);
                }
                visitor.visitRange(dataset_.get(), p, true_end, valids); 
            }
        }
    }
}

