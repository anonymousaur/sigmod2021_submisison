#include "single_column_rewriter.h"
#include <cassert>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

template <size_t D>
SingleColumnRewriter<D>::SingleColumnRewriter(const std::string& filename) 
    : cmap_(), mapped_dim_(), target_dim_() {
        Load(filename);
}

/*
 * The format of the file is:
 *  | mapped_dim target_dim
 *  | mapped_val num_targets
 *  | <list of space-separated targets>
 *  | mapped_val2 num_targets
 *  | <list of space-separated targets>
 *  ...
 */
template <size_t D>
void SingleColumnRewriter<D>::Load(const std::string& filename) {
    std::ifstream file(filename);
    assert (file.is_open());

    size_t num_entries = 0;
    // These getlines make sure we discard the newline after each line when parsing.
    std::string line;
    std::getline(file, line);
    std::istringstream head(line);

    head >> mapped_dim_ >> target_dim_;
    assert (mapped_dim_ != target_dim_
           && mapped_dim_ < D && target_dim_ < D);
    
    while (!file.eof()) {
        std::getline(file, line);
        if (line.empty()) {
            break;
        }    
        size_t num_targets;
        Scalar mapped_val;
        std::istringstream spec(line);
        spec >> mapped_val >> num_targets;

        std::getline(file, line);
        std::istringstream liness(line);
        Scalar target_val;
        std::vector<Scalar> targets;
        targets.reserve(num_targets);
        while (liness >> target_val) {
            targets.push_back(target_val);
        }
        assert (cmap_.find(mapped_val) == cmap_.end());
        assert (targets.size() == num_targets);
        cmap_[mapped_val] = targets;
    }
    std::cout << "Rewriter has " << cmap_.size() << " mapping entries" << std::endl;
}

template <size_t D>
Query<D> SingleColumnRewriter<D>::Rewrite(const Query<D>& q) const {
    if (q.filters[mapped_dim_].size() == 0) {
        return q;
    }
    Query<D> rewritten;
    // Deep copy the query filters.
    for (int i = 0; i < D; i++) {
        rewritten.filters[i] = q.filters[i];
    }
    std::vector<Scalar> targets;
    for (Scalar mval : q.filters[mapped_dim_]) {
        auto loc = cmap_.find(mval);
        if (loc != cmap_.end()) {
            targets.insert(targets.end(), loc->second.begin(), loc->second.end());
        }
    }
    std::sort(targets.begin(), targets.end());
    auto q_targets = q.filters[target_dim_];
    std::vector<Scalar> merged;
    if (q_targets.empty()) {
        merged = targets;
    } else {
        std::sort(q_targets.begin(), q_targets.end()); 
        // Intersect the two.
        size_t targets_ix = 0;
        size_t q_ix = 0;
        while (targets_ix < targets.size() && q_ix < q_targets.size()) {
            if (targets[targets_ix] < q_targets[q_ix]) {
                targets_ix++;
            } else if (q_targets[q_ix] < targets[targets_ix]) {
                q_ix++;
            } else {
                merged.push_back(targets[targets_ix]);
                targets_ix++;
                q_ix++;
            }
        }
    }
    rewritten.filters[target_dim_] = merged;
    return rewritten;
}

