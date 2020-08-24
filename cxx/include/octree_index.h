#pragma once

#include <vector>
#include <memory>

#include "types.h"
#include "utils.h"
#include "primary_indexer.h"

/**
 * Splits space into eighths until each leaf node contains less than the page size number of points
 */
template <size_t D>
class OctreeIndex : public PrimaryIndexer<D> {
    public:
        //    Children follow a predictable pattern to make accesses simple.
        //    Here, - means less than 'origin' in that dimension, + means greater than.
        //    child:	0 1 2 3 4 5 6 7
        //    x:        - - - - + + + +
        //    y:        - - + + - - + +
        //    z:        - + - + - + - +
        struct Node {
            std::vector<std::unique_ptr<Node>> children;
            std::vector<Scalar> mins;
            std::vector<Scalar> maxs;  // inclusive
            PhysicalIndex start_offset;
            PhysicalIndex end_offset;
        };

        explicit OctreeIndex(std::vector<size_t>& index_dims);
        OctreeIndex(std::vector<size_t>& index_dims, size_t page_size);

        void Init(PointIterator<D> start, PointIterator<D> end) override;
        PhysicalIndexSet Ranges(const Query<D>&) const override;
        size_t Size() const override;

        std::unordered_set<size_t> GetColumns() const override {
            return std::unordered_set<size_t>(index_dims_.cbegin(), index_dims_.cend());
        }

    private:
        std::vector<size_t> index_dims_;
        size_t page_size_;
        std::unique_ptr<Node> root_node;
        bool sort_leaf_;
        size_t sort_dim_;
        size_t data_size_;

        std::vector<Scalar> mins_;
        std::vector<Scalar> maxs_;

        int get_octant_containing_point(Point<D>& point, std::vector<Scalar>& center) const;
        bool divide_node(Node* node, PointIterator<D> start, PointIterator<D> end, int depth);
        bool should_keep_dividing(Node* node, int depth) const;
        bool is_relevant_node(Node* node, const Query<D>& query) const;
        size_t num_partitions_;

        static const size_t DEFAULT_PAGE_SIZE = 10000;
        static const int DEFAULT_MAX_DEPTH = 100;
        std::ofstream sorted_data_points_;
        std::ofstream sorted_data_buckets_; 
};

#include "../src/octree_index.hpp"
