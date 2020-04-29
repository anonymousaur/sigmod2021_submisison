/**
 * @file
 * @author Guillaume Leclerc
 * @brief some generic utilitary functions
 */

#pragma once

#include <iostream>
#include <fstream>

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
        assert (line.front() == '=');
        Query<D> q;
        for (size_t i = 0; i < D; i++) {
            std::getline(file, line);
            assert (!line.empty());
            std::istringstream iss(line);
            Scalar val;
            q.filters[i] = std::vector<Scalar>();
            while (iss >> val) {
                q.filters[i].push_back(val);
            }
        }
        q_list.push_back(q);
    }
    return q_list;
}

