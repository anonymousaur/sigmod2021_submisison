/**
 * @file
 * @author Guillaume Leclerc
 * @brief some generic utilitary functions
 */

#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

template <typename K>
std::vector<K> load_binary_file(const std::string &filename) {
    std::vector<K> result;
    std::ifstream file;
    // Start the file at the end to know the size easily
    file.open(filename, std::ios::in | std::ios::binary | std::ios::ate);
    size_t size = file.tellg();
    result.resize(size / sizeof(K));
    // Jump to the beginning of the file now that we know its size
    file.seekg(0, std::ios::beg);
    file.read((char*) result.data(), size);
    file.close();
    return result;
}

template <size_t D>
std::vector<Query<D>> load_query_file(const std::string& filename) {
    std::vector<Query<D>> result;
    std::ifstream file(filename);

    std::string sep;
    std::string line;
    std::vector<Query<D>> q_list;
    while (std::getline(file, line)) {
        if (line.empty()) {
            break;
        }
        assert (line.front() == '=');
        Query<D> q;
        for (size_t i = 0; i < D; i++) {
            std::getline(file, line);
            assert (!line.empty());
            std::istringstream iss(line);
            std::string type;
            iss >> type;
            Scalar val;
            QueryFilter sf;
            // By default don't assume there's a filter.
            sf.present = false;
            if (type == "values") {
                sf.is_range = false;
                std::vector<Scalar> vals;
                while (iss >> val) {
                    vals.push_back(val);
                }
                sf.values = vals;
                sf.present = true;
            } else if (type == "ranges") {
                sf.is_range = true;
                ScalarRange range;
                iss >> range.first >> range.second;    
                sf.ranges = { range };
                sf.present = true;
            } else {
                assert (type == "none");
            }
            q.filters[i] = sf;
        }
        q_list.push_back(q);
    }
    return q_list;
}

std::string ArrayToString(const std::vector<Scalar>& vals) {
    std::stringbuf buffer;
    std::ostream os(&buffer);
    for (Scalar v : vals) {
        os << v << " ";
    }
    return buffer.str();
}

template <size_t D>
std::string PointsToString(typename std::vector<Point<D>>::iterator start,
        typename std::vector<Point<D>>::iterator end) {
    std::stringbuf buffer;
    std::ostream os(&buffer);
    for (auto it = start; it != end; it++) {
        os << "{";
        for (Scalar p : *it) {
            os << p << ", ";
        }
        os << "}" << std::endl;
    }
    return buffer.str();
}

std::string RangesToString(const std::vector<PhysicalIndexRange>& ranges) {
    std::stringbuf buffer;
    std::ostream os(&buffer);
    for (const PhysicalIndexRange& pr : ranges) {
        os << "[" << pr.start << ", " << pr.end << "]" << std::endl;
    }
    return buffer.str();
}

template <typename T>
std::string VectorToString(const std::vector<T>& v) {
    std::stringbuf buffer;
    std::ostream os(&buffer);
    os << "{";
    for (auto t : v) {
        os << t << " ";
    }
    os << "}" << std::flush;
    return buffer.str();
}


std::string QueryFilterToString(const QueryFilter& qf) {
    std::stringbuf buffer;
    std::ostream os(&buffer);
    os << "{present = " << qf.present;
    if (qf.present) {
        os << ", is_range = " << qf.is_range;
        if (qf.is_range) {
            os << ", ranges = " << VectorToString(qf.ranges);
        } else {
            os << ", values = " << VectorToString(qf.values);
        }
    }
    os << "}" << std::endl;
    return buffer.str();
}

