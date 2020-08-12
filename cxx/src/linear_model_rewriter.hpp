#include "linear_model_rewriter.h"

#include <iostream>
#include <fstream>
#include <cassert>
#include <cmath>

#include "types.h"
#include "file_utils.h"

template <size_t D>
LinearModelRewriter<D>::LinearModelRewriter(const std::string& filename) {
    Load(filename);
}

template <size_t D>
void LinearModelRewriter<D>::Load(const std::string& filename) {
    std::ifstream file(filename);
    assert (file.is_open());
    assert (FileUtils::NextLine(file) == "linear");

    auto dims = FileUtils::NextArray<size_t>(file, 2);
    mapped_dim_ = dims[0];
    target_dim_ = dims[1];
    auto coeffs = FileUtils::NextArray<double>(file, 2);
    linear_coeffs_.first = coeffs[0];
    linear_coeffs_.second = coeffs[1];
    auto offset = FileUtils::NextArray<double>(file, 1);
    model_offset_ = offset[0];
}

template <size_t D>
Query<D> LinearModelRewriter<D>::Rewrite(const Query<D>& q) const {
    if (!q.filters[mapped_dim_].present) {
        return q;
    }
    assert (q.filters[mapped_dim_].is_range);
    Scalar start = q.filters[mapped_dim_].ranges[0].first;
    Scalar end = q.filters[mapped_dim_].ranges[0].second;   

    double offset = linear_coeffs_.second > 0 ? model_offset_ : -model_offset_;
    double tstart = linear_coeffs_.first + linear_coeffs_.second * start - offset;
    double tend = linear_coeffs_.first + linear_coeffs_.second * end + offset;
    if (tend < tstart) {
        std::swap(tstart, tend);
    }

    // Intersect this with the existing ranges in the filter.
    QueryFilter target_qf = q.filters[target_dim_];
    std::pair<double, double> range = {0,0};
    if (!target_qf.present) {
        range = {tstart, tend};
    } else {
        assert (target_qf.is_range);
        ScalarRange existing = target_qf.ranges[0];
        double mod_first = fmax((double)(existing.first), tstart);
        double mod_second = fmin((double)(existing.second), tend);
        range = {mod_first, mod_second};
    }
    
    std::vector<ScalarRange> ranges;
    if (range.second > range.first) {
        ScalarRange srange;
        srange.first = (Scalar)(range.first);
        srange.second = (Scalar)(ceil(range.second));
        ranges.push_back(srange);
    } 
    Query<D> rewritten = q;
    rewritten.filters[target_dim_] = {.present=true, .is_range=true, .ranges=ranges};
    return rewritten; 
}

