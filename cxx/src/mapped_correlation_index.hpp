#include "mapped_correlation_index.h"

#include <cassert>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "file_utils.h"
#include "merge_utils.h"

template <size_t D>
MappedCorrelationIndex<D>::MappedCorrelationIndex(const std::string& mapping_filename,
        const std::string& target_buckets_filename) 
    : mapping_() {
    Load(mapping_filename, target_buckets_filename);
}

template <size_t D>
void MappedCorrelationIndex<D>::Load(const std::string& mapping_filename, 
        const std::string& target_buckets_filename) {
    std::ifstream file(mapping_filename);
    assert (file.is_open());
    assert (FileUtils::NextLine(file) == "continuous-0");
    
    auto header = FileUtils::NextArray<std::string>(file, 3);
    assert (header[0] == "source");
    this->column_ = std::stoi(header[1]);
    size_t s = std::stoi(header[2]);
    // The last element is the rightmost bound on the data.
    // Read everything as a double to be safe and then convert to scalar
    
    std::cout << "Reading " << s << " mapped buckets" << std::endl;
    std::unordered_map<size_t, ScalarRange> mapped_buckets;
    mapped_buckets.reserve(s); 
    for (size_t i = 0; i < s; i++) {
        auto arr = FileUtils::NextArray<double>(file, 3);
        size_t mapix = (size_t)arr[0];
        ScalarRange r = {(Scalar)arr[1], (Scalar)arr[2]};
        mapped_buckets.emplace(mapix, r);
    }

    header = FileUtils::NextArray<std::string>(file, 2);
    std::cout << header[0] << std::endl;
    assert (header[0] == "mapping");
    s = std::stoi(header[1]);
    assert (s <= mapped_buckets.size());
    std::unordered_map<size_t, std::vector<size_t>> bucket_mapping;
    bucket_mapping.reserve(s);
    for (size_t i = 0; i < s; i++) {
        auto arr = FileUtils::NextArray<size_t>(file);
        bucket_mapping.emplace(arr[0], std::vector<size_t>(arr.begin() + 1, arr.end()));
    } 
    std::cout << "Finished loading mapping-file" << std::endl;
   
    std::ifstream targetfile(target_buckets_filename); 
    assert (targetfile.is_open());
    header = FileUtils::NextArray<std::string>(targetfile, 2);
    assert (header[0] == "target_index_ranges");
    s = std::stoi(header[1]);
    std::unordered_map<size_t, PhysicalIndexRange> targets;
    targets.reserve(s);
    for (size_t i = 0; i < s; i++) {
        auto arr = FileUtils::NextArray<size_t>(targetfile, 3);
        targets.emplace(arr[0], PhysicalIndexRange(arr[1], arr[2]));
    }
    std::cout << "Finished loading target-buckets-file" << std::endl;

    for (auto mapit = mapped_buckets.cbegin(); mapit != mapped_buckets.cend(); mapit++) {
        ScalarRange mb = mapit->second;
        auto loc = bucket_mapping.find(mapit->first);
        // Can do something else here - we know that if a mapping is present, then there's at least
        // one point that lies in it, even if it's an outlier. If a mapping is not present, we know
        // there's nothing even in the outlier index.
        if (loc == bucket_mapping.end()) {
            // Nothing to be done here.
            continue;
        }
        std::vector<size_t> tbs = loc->second;
        
        IndexRange tixs;
        for (size_t j = 0; j < tbs.size(); j++) {
            auto it = targets.find(tbs[j]);
            // If the key isn't found, there's an inconsistency: it corresponds to an inlier bucket
            // but we missed it when writing out the target buckets.
            assert (it != targets.end());
            PhysicalIndexRange r = it->second;
            // There must be points contained here, otherwise we shouldn't have written this bucket.
            assert (r.end > r.start);
            if (tixs.size() > 0 && r.start == tixs.back().end) {
                // These ranges are consecutive, so merge them.
                tixs.back().end = r.end;
            } else {
                tixs.push_back(r);
            }
        }
        tixs.shrink_to_fit();

        mapping_.emplace(mb, tixs);
    }

    std::cout << "Finished coaslescing map" << std::endl;
};

template <size_t D>
IndexRange MappedCorrelationIndex<D>::Ranges(const Query<D>& q) const {
    size_t col = this->column_;
    if (!q.filters[col].present) {
        return {{0, data_size_}};
    }
    assert (q.filters[col].is_range);
    assert (q.filters[col].ranges.size() == 1);
    ScalarRange sr = q.filters[col].ranges[0];

    auto startit = mapping_.upper_bound({sr.first, sr.first});
    if (startit == mapping_.end())  {
        return {};
    }
    auto endit = mapping_.upper_bound({sr.second, sr.second});
    // Since startit is an upper bound, we need to go back one to include the whole range.
    if (startit != mapping_.begin()) {
        startit--;
    }
    // Map buckets may not be continuous, so check that the end of this bucket is larger than the
    // start of the queried range. If not, increment back up.
    if (startit->first.second < sr.first) {
        startit++;
    }
    IndexRange ranges;
    for (auto it = startit; it != endit; it++) {
        /*std::cout << "Matched map bucket " << it->first << " with "
            << it->second.size() << " ranges:" << std::endl;
        for (auto r : it->second) {
            std::cout << "\t" << r.start << " - " << r.end << std::endl;
        }*/
        ranges = MergeUtils::Union(ranges, it->second);
        /*std::cout << "Merged ranges:" << std::endl;
        for (auto r : ranges) {
            std::cout << "\t" << r.start << " - " << r.end << std::endl;
        }*/
    }
    return ranges;
}



