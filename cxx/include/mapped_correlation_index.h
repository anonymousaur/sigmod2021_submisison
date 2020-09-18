#pragma once

#include <vector>
#include <map>
#include <string>
#include <unordered_map>

#include "correlation_indexer.h"
#include "types.h"

template <size_t D>
class MappedCorrelationIndex : public CorrelationIndexer<D> {
  public:
    MappedCorrelationIndex(const std::string& mapping_file, const std::string& target_bucket_file);

    virtual void Init(ConstPointIterator<D> start, ConstPointIterator<D> end) override {
        data_size_ = std::distance(start, end);
        ready_ = true;
    }

    PhysicalIndexSet Ranges(const Query<D>& q) const override; 
    
    size_t Size() const override {
        size_t s = 0;
        for (auto it = mapping_.begin(); it != mapping_.end(); it++) {
            s += it->second.size() * sizeof(PhysicalIndexRange);
            s += sizeof(ScalarRange);
        }
        // BTree overhead (5 bytes per entry)
        return mapping_.size() * 10 + s;
    }

    // This is ONLY for use in special cases. Usually the column is automatically set from the
    // mapping file.
    void SetColumn(size_t col) {
        this->column_ = col;
    }

  private:
    // Load the contents of the files specified in the constructor, used to construct a mapping we
    // can use.
    void Load(const std::string&, const std::string&);
    
    // Number of data points
    size_t data_size_;
    
    // True when the index has been initialized.
    bool ready_;
    
    // Mapping from mapped dimension bucket value (left bound) to range of indices it corresponds to.
    // TODO: use a CPP B-Tree here (https://code.google.com/archive/p/cpp-btree/) for
    // memory-efficient and fast accesses.
    btree::btree_map<ScalarRange, IndexRangeList, ScalarRangeComp> mapping_; 
    btree::btree_map<ScalarRange, std::vector<int32_t>, ScalarRangeComp> mapping_lst_;
    std::vector<std::pair<int32_t, PhysicalIndexRange>> target_buckets_;
};

#include "../src/mapped_correlation_index.hpp"
