#pragma once

#include <vector>
#include <map>
#include <string>

#include "indexer.h"
#include "types.h"

template <size_t D>
class MappedCorrelationIndex : public CorrelationIndexer<D> {
  public:
    MappedCorrelationIndex(const std::string& mapping_file, const std::string& target_bucket_file);

    virtual void Init(ConstPointIterator<D> start, ConstPointIterator<D> end) override {
        data_size_ = std::distance(start, end);
        ready_ = true;
    }

    std::vector<PhysicalIndexRange> Ranges(const Query<D>& q) const override; 
    
    size_t Size() const override {
        size_t s = 0;
        for (auto it = mapping_.begin(); it != mapping_.end(); it++) {
            s += it->second.size() * sizeof(PhysicalIndexRange);
            s += sizeof(ScalarRange);
        }
        // Factor of 2 is safe. Switch to the google c++ btree for a lower factor.
        return 2*s;
    }

  private:
    // Load the contents of the files specified in the constructor, used to construct a mapping we
    // can use.
    void Load(const std::string&, const std::string&);
    
    // Number of data points
    size_t data_size_;
    
    // Used for internal purposes only.
    size_t column_;    
    // True when the index has been initialized.
    bool ready_;
    
    // Mapping from mapped dimension bucket value (left bound) to range of indices it corresponds to.
    // TODO: use a CPP B-Tree here (https://code.google.com/archive/p/cpp-btree/) for
    // memory-efficient and fast accesses.
    std::map<ScalarRange, IndexRange, ScalarRangeComp> mapping_; 

};

#include "../src/mapped_correlation_index.hpp"
