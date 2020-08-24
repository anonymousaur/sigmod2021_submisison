#include "merge_utils.h"


IndexRangeList MergeUtils::Intersect(const IndexRangeList& first, const IndexRangeList& second) {
    // Assumes both ranges are already sorted.
    IndexRangeList final_ranges;
    size_t i = 0, j = 0;
    size_t count = 0;
    PhysicalIndex cur_first = first[0].start, cur_second = second[0].start;
    bool i_start = true, j_start = true;
    bool in_range = false;
    PhysicalIndexRange cur_range;
    while (i < first.size() && j < second.size()) {
        PhysicalIndex cur_first, cur_second, popped;
        if (i_start) { cur_first = first[i].start; }
        else { cur_first = first[i].end; }
        
        if (j_start) { cur_second = second[j].start; }
        else { cur_second = second[j].end; }
        
        // Here, we don't care about the case when they're equal.
        // Since ranges in each input vector are not contiguous, if there's an equality here, it
        // won't change the final answer.
        if (cur_first < cur_second) {
            popped = cur_first;
            if (i_start) {
                count++;
                i_start = false;
            }
            else {
                count--;
                i++;
                i_start = true;
            }
        } else {
            popped = cur_second;
            if (j_start) {
                count++;
                j_start = false;
            }
            else {
                count--;
                j_start = true;
                j++;
            }
        }
        if (count == 2) {
            assert (!in_range);
            in_range = true;
            cur_range.start = popped;
        } else if (in_range) {
            assert (count == 1);
            cur_range.end = popped;
            final_ranges.push_back(cur_range);
            in_range = false;
        }
    }
    assert (count < 2);
    return final_ranges;
}

IndexList MergeUtils::Intersect(const IndexList& list1, const IndexList& list2) {
    IndexList output;
    size_t i = 0, j = 0;
    while (i < list1.size() && j < list2.size()) {
        if (list1[i] < list2[j]) {
            i++;
        } else if (list1[i] > list2[j]) {
            j++;
        } else {
            output.push_back(list1[i]);
            i++;
            j++;
        }
    }
    return output;
}

PhysicalIndexSet MergeUtils::Intersect(const PhysicalIndexSet& set1, const PhysicalIndexSet& set2) {
    // Assumes both ranges are already sorted.
    IndexRangeList final_ranges;
    IndexList final_list;
    final_list.reserve(set1.list.size() + set2.list.size());
    auto first_list_it = set1.list.cbegin();
    auto first_range_it = set1.ranges.cbegin();
    auto second_list_it = set2.list.cbegin();
    auto second_range_it = set2.ranges.cbegin();
    // We're just intersecting the lists here.    
    while (true) {
        bool first_list_done = first_list_it == set1.list.cend();
        bool second_list_done = second_list_it == set2.list.cend();
        if (first_list_done && second_list_done) {
            break;
        }
        if (!first_list_done && (second_list_done || *first_list_it < *second_list_it)) {
            bool second_range_ready = second_range_it != set2.ranges.cend();
            if (second_range_ready && *first_list_it >= second_range_it->end) {
                second_range_it++;
            } else if (second_range_ready && *first_list_it >= second_range_it->start) {
                final_list.push_back(*first_list_it);
                first_list_it++;
            } else {
                first_list_it++;
            }
        } else if (!second_list_done && (first_list_done || *second_list_it < *first_list_it)) {
            bool first_range_ready = first_range_it != set1.ranges.cend();
            if (first_range_ready && *second_list_it >= first_range_it->end) {
                first_range_it++;
            } else if (first_range_ready && *second_list_it >= first_range_it->start) {
                final_list.push_back(*second_list_it);
                second_list_it++;
            } else {
                second_list_it++;
            }
        } else {
            // list indexes are equal to each other.
            final_list.push_back(*first_list_it);
            first_list_it++;
            second_list_it++;
        }
    }
    final_list.shrink_to_fit();
    return { MergeUtils::Intersect(set1.ranges, set2.ranges), final_list };
}
    

IndexRangeList MergeUtils::Union(const IndexRangeList& first, const IndexRangeList& second) {
    // Assumes both ranges are already sorted.
    IndexRangeList final_ranges;
    size_t i = 0, j = 0;
    size_t count = 0;
    PhysicalIndex cur_first = first[0].start, cur_second = second[0].start;
    bool i_start = true, j_start = true;
    bool in_range = false;
    PhysicalIndexRange cur_range;
    while (i < first.size() || j < second.size()) {
        PhysicalIndex cur_first = std::numeric_limits<PhysicalIndex>::max(),
               cur_second = std::numeric_limits<PhysicalIndex>::max(),
               popped = 0;
        if (i < first.size()) {
            if (i_start) { cur_first = first[i].start; }
            else { cur_first = first[i].end; }
        }
        if (j < second.size()) {
            if (j_start) { cur_second = second[j].start; }
            else { cur_second = second[j].end; }
        }
        // If cur_first == cur_second, we want to prefer the one that is a start
        // of a range.
        if (cur_first < cur_second || (cur_first == cur_second && i_start)) {
            popped = cur_first;
            if (i_start) {
                count++;
                i_start = false;
            }
            else {
                count--;
                i++;
                i_start = true;
            }
        } else {
            popped = cur_second;
            if (j_start) {
                count++;
                j_start = false;
            }
            else {
                count--;
                j_start = true;
                j++;
            }
        }
        if (count == 0) {
            // We should have had a range started at this point. 
            assert (in_range);
            cur_range.end = popped;
            final_ranges.push_back(cur_range);
            in_range = false;
        } else if (!in_range) {
            // We have just started a new range.
            assert (count == 1);
            cur_range.start = popped;
            in_range = true;
        }
    }
    assert (count == 0);
    return final_ranges;
}

PhysicalIndexSet MergeUtils::Union(const IndexRangeList& ranges, const IndexList& list) {
    // Just eliminate the indexes that fall inside one of the ranges. Assumes both are sorted.
    if (ranges.size() == 0) {
        return {{}, list};
    }
    size_t cur_range_ix = 0;
    size_t list_ix = 0;
    IndexList pruned;
    pruned.reserve(list.size());
    while (list_ix < list.size()) {
        size_t index = list[list_ix];
        if (index < ranges[cur_range_ix].start) {
            pruned.push_back(index);
            list_ix++;
        }
        else if (index > ranges[cur_range_ix].end) {
            cur_range_ix++;
        } else {
            // The index is in a range.
            list_ix++;
        }
    }
    pruned.shrink_to_fit();
    return {ranges, pruned};
}


IndexList MergeUtils::Merge(const IndexRangeList& ranges, const IndexList& idxs) {
    if (ranges.empty() || idxs.empty()) {
        return {};
    }
    size_t cur_range_ix = 0;
    IndexList output;
    output.reserve(idxs.size());
    for (size_t ix : idxs) {
       while (ix >= ranges[cur_range_ix].end) {
           cur_range_ix++;
       }
       if (cur_range_ix > ranges.size()) {
           break;
       }
       if (ix < ranges[cur_range_ix].start) {
           continue;
       }
       output.push_back(ix);
    }
    output.shrink_to_fit();
    return output;
}

