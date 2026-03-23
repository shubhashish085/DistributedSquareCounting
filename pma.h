#ifndef DISTRIBUTEDSQUARECOUNTING_PMA_H
#define DISTRIBUTEDSQUARECOUNTING_PMA_H


#include <climits>
#include <cmath>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <functional>
#include <cstring>
#include <cassert>
#include <chrono>
#include "types.h"


class PMA{

    public:
        typedef std::function<void(uint64_t)> range_func;
        static inline const uint64_t INT_NULL = UINT64_MAX;
        static inline const VertexID INVALID_IDX = UINT32_MAX;

        ui vertices_count;

        std::vector<uint64_t> data;
        VertexID length;
        double leaf_max = 0.75;
        VertexID num_elements;
        bool resize_allowed;
        VertexID max_index;
        std::vector<uint64_t> temp;

        PMA(VertexID size);
        PMA(VertexID size, double leaf_max);
        PMA(VertexID size, double leaf_max, bool resize_allowed);
        
        PMA() = default;

        std::vector<uint64_t> get_min_range(VertexID left, VertexID right);
        void swap_data(std::vector<uint64_t>& tmp);
        void range(uint64_t left, uint64_t right, range_func& op);
        
        VertexID logN();
        VertexID loglogN();
        VertexID leaf_index(VertexID index);
        VertexID next_leaf(VertexID index);
        VertexID leaf_number(VertexID index);
        VertexID leaf_position(VertexID leaf_num);
        VertexID num_leaves();
        VertexID depth();
        VertexID count_nonempty(VertexID index, VertexID len);

        VertexID size();
        void insert(uint64_t i);
        void delete_edge(uint64_t i);
        bool query(uint64_t i); 
        VertexID search(uint64_t i);
        void redistribute(VertexID index, VertexID len, VertexID density_count);
        void resize();
        void slide_left(VertexID index);
        void slide_right(VertexID index);
        void print_pma(std::ostream& stream = std::cout);
        
};

#endif