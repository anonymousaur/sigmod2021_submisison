#include "categorical_query_engine.h"

#include <vector>

#include "types.h"
#include "visitor.h"

template <size_t D>
CategoricalQueryEngine<D>::CategoricalQueryEngine(
        Dataset<D>* dataset, Indexer<D>* indexer, Rewriter<D>* rewriter)
    : dataset_(dataset), indexer_(indexer), rewriter_(rewriter) {}

template <size_t D>
void CategoricalQueryEngine<D>::Execute(const Query<D>& q, Visitor<D>& visitor) {
    std::vector<size_t> query_dimensions;
    std::vector<std::unordered_set<Scalar>> value_sets;
    for (size_t i = 0; i < dataset_->NumDims(); i++) {
        if (q.filters[i].size() > 0) {
            query_dimensions.push_back(i);
            value_sets.push_back(std::unordered_set(q.filters[i].begin(), q.filters[i].end()));
        }
    }
    Query<D> new_q = rewriter_->Rewrite(q);
    for (PhysicalIndexRange range : indexer_->Ranges(new_q)) {
        for (PhysicalIndex p = range.start; p < range.end; p += 64UL) {
            size_t true_end = std::min(range.end, p + 64UL);
            // A way to get the last true_end - p bits set to 1.
            uint64_t valids = 1ULL + (((1ULL << (true_end - p - 1)) - 1ULL) << 1);
            for (size_t i = 0; i < query_dimensions.size(); i++) {
                valids &= dataset_->GetCoordInSet(p, true_end,
                        query_dimensions[i], value_sets[i]);
            }
            visitor.visitRange(dataset_, p, true_end, valids); 
        }
    }
}

