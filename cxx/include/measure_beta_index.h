#pragma once

#include <vector>
#include <random>

#include "primary_indexer.h"
#include "dataset.h"
#include "types.h"

/*
 * A clustered index on one dimension that uses no auxiliary data structure. The data is sorted and
 * the desired range is found using binary search on the endpoints.
 */
template <size_t D>
class MeasureBetaIndex : public PrimaryIndexer<D> {
  public:
    MeasureBetaIndex(size_t dim) : column_(dim), gen_(), minval_(), maxval_() {}

    void Init(PointIterator<D> start, PointIterator<D> end) override {
        minval_ = std::numeric_limits<Scalar>::max();
        maxval_ = std::numeric_limits<Scalar>::lowest();
        for (auto it = start; it != end; it++) {
            minval_ = std::min(minval_, (*it)[column_]);
            maxval_ = std::max(maxval_, (*it)[column_]);
        }
        std::cout << "Maxval = " << maxval_ << std::endl;
        data_size_ = std::distance(start, end);
        match_size_dist_ = std::uniform_real_distribution<float>(0, log2(0.5*maxval_));
        range_size_dist_ = std::uniform_real_distribution<float>(0, log2(data_size_));
        match_start_dist_ = std::uniform_int_distribution<int>(0, maxval_);
        range_start_dist_ = std::uniform_int_distribution<int>(0, data_size_);
        secondary_->Init(start, end);
    }

    void SetAuxiliaryIndex(std::unique_ptr<SecondaryIndexer<D>> secondary) {
        secondary_ = std::move(secondary);
    }

    PhysicalIndexSet Ranges(Query<D>& q) override {
        size_t range_size = pow(2., range_size_dist_(gen_));
        size_t range_start = range_start_dist_(gen_);
        std::cout << "Range " << range_start << " - " << range_start + range_size << std::endl;

        Scalar match_size = pow(2., match_size_dist_(gen_));
        Scalar match_start = match_start_dist_(gen_);
        std::cout << "Match range " << match_start << " - " << match_start + match_size << std::endl;

        q.filters[column_] = {.present = true, .is_range=true, .ranges={{match_start, match_start + match_size}}};
       
        // We don't care about accuracy here.
        return PhysicalIndexSet({{range_start, std::min(data_size_, range_start + range_size)}},
                                secondary_->Matches(q));
    } 

    size_t Size() const override {
        return 0;
    }

    void WriteStats(std::ofstream& statsfile) override {
        statsfile << "primary_index_type: measure_beta_index_" << column_ << std::endl;
    }

  protected:
    std::default_random_engine gen_;
    std::uniform_int_distribution<int> match_start_dist_;
    std::uniform_int_distribution<int> range_start_dist_;
    std::uniform_real_distribution<float> match_size_dist_;
    std::uniform_real_distribution<float> range_size_dist_;
    Scalar minval_;
    Scalar maxval_;
    size_t column_;
    size_t data_size_;
    // True when the index has been initialized.
    bool ready_;
    std::unique_ptr<SecondaryIndexer<D>> secondary_;
};

