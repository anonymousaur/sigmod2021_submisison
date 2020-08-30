#pragma once

#include <algorithm>
#include <vector>
#include <map>
#include <string>

#include "correlation_indexer.h"
#include "types.h"
#include "mapped_correlation_index.h"
#include "secondary_indexer.h"

template <size_t D>
class CombinedCorrelationIndex : public CorrelationIndexer<D> {
  public:
    CombinedCorrelationIndex() : data_size_(0), ready_(false),
        mapped_index_(), outlier_index_() {}
    
    void SetMappedIndex(std::unique_ptr<MappedCorrelationIndex<DIM>> mix) {
        mapped_index_ = std::move(mix);
    }

    void SetOutlierIndex(std::unique_ptr<SecondaryIndexer<DIM>> six) {
       outlier_index_ = std::move(six);
    } 

    virtual void Init(ConstPointIterator<D> start, ConstPointIterator<D> end) override {
        assert (mapped_index_ && outlier_index_);
        mapped_index_->Init(start, end);
        outlier_index_->Init(start, end);
        this->column_ = mapped_index_->GetMappedColumn();
        std::cout << "Initialized Correlation Index with column " << this->column_ << std::endl;
        data_size_ = std::distance(start, end);
        ready_ = true;
    }

    PhysicalIndexSet Ranges(const Query<D>& q) const override {
        // TODO(vikram): since we're using bucketedSecondaryIndexer, this is already sorted.
        auto start = std::chrono::high_resolution_clock::now();
        IndexList lst = outlier_index_->Matches(q);
        auto mid = std::chrono::high_resolution_clock::now();
        PhysicalIndexSet mapped_ranges = mapped_index_->Ranges(q);
        auto end = std::chrono::high_resolution_clock::now();
        auto match_t = std::chrono::duration_cast<std::chrono::nanoseconds>(mid-start).count();
        auto range_t = std::chrono::duration_cast<std::chrono::nanoseconds>(end-mid).count();
        std::cout << "Range time (us): " << range_t / 1e3 << std::endl;
        std::cout << "Match time (us): " << match_t / 1e3 << std::endl;
        PhysicalIndexSet ret = MergeUtils::Union(mapped_ranges.ranges, lst);
        return ret;
    }
    
    size_t Size() const override {
        return mapped_index_->Size() +  outlier_index_->Size();
    }

  private:
    // Load the contents of the files specified in the constructor, used to construct a mapping we
    // can use.
    void Load(const std::string&, const std::string&);
    
    // Number of data points
    size_t data_size_;
    
    // True when the index has been initialized.
    bool ready_;
    // The combined index consists of two parts: one mapped index which produces only ranges in the
    // target dimension(s), and the outlier map that points to specific indices in the dataset to
    // check in addition to the ranges.
    std::unique_ptr<MappedCorrelationIndex<D>> mapped_index_;
    std::unique_ptr<SecondaryIndexer<D>> outlier_index_;
};

