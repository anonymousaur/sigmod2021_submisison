#include "merge_utils.h"


IndexRange MergeUtils::Intersect(const IndexRange& first, const IndexRange& second) {
    // Assumes both ranges are already sorted.
    IndexRange final_ranges;
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

IndexRange MergeUtils::Union(const IndexRange& first, const IndexRange& second) {
    // Assumes both ranges are already sorted.
    IndexRange final_ranges;
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

IndexRange MergeUtils::Merge(const IndexRange& ranges, const IndexList& idxs, size_t gap_threshold) {
    IndexRange output;
    if (ranges.empty() || idxs.empty()) {
        return output;
    }
    size_t cur_range_ix = 0;
    PhysicalIndexRange running = {0, 0};
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
       if (running.start >= running.end) {
           running.start = ix;
           running.end = ix+1;
       } else {
           if (running.end + gap_threshold > ix) {
               running.end = ix+1;
           } else {
               output.push_back(running);
               running = {ix, ix+1};
           }
       }
    }
    output.push_back(running);
    return output;
}

