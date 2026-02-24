#ifndef DISTRIBUTEDSQUARECOUNTING_PCSR_H
#define DISTRIBUTEDSQUARECOUNTING_PCSR_H

#include <climits>
#include <cmath>
#include <iostream>
#include <unordered_set>
#include "pma.h"


class PCSR{

    public:
        PCSR(uint32_t size);
        uint32_t size();
        
        void insert_edge(uint32_t from, uint32_t to);
        void delete_edge(uint32_t from, uint32_t to);
        bool query_edge(uint32_t from, uint32_t to);
        
        std::vector<uint32_t> edges(uint32_t from);
        std::vector<std::pair<uint32_t, std::vector<uint32_t>>> adjacency_lists();
    
    private:
        static uint64_t make_edge_tuple(uint32_t from, uint32_t to);
        static std::pair<uint32_t, uint32_t> get_edge_tuple(uint64_t edge);
        PMA pma;
        const uint32_t TO_ONES = 0xFFFFFFFF;
};

PCSR::PCSR(uint32_t size) : pma(size) {}

uint64_t PCSR::make_edge_tuple(uint32_t from, uint32_t to) { return (((uint64_t) from) << 32) | to; }
std::pair<uint32_t, uint32_t> PCSR::get_edge_tuple(uint64_t edge) { return std::make_pair((uint32_t)(edge >> 32), (uint32_t)(edge << 32)); } 

void PCSR::insert_edge(uint32_t from, uint32_t to) {
    uint64_t edge = make_edge_tuple(from, to);
    pma.insert(edge);
}

void PCSR::delete_edge(uint32_t from, uint32_t to) {
    uint64_t edge = make_edge_tuple(from, to);
    pma.delete_edge(edge);
}

bool PCSR::query_edge(uint32_t from, uint32_t to) {
    uint64_t edge = make_edge_tuple(from, to);
    return pma.query(edge);
}

std::vector<uint32_t> PCSR::edges(uint32_t from) {
    uint64_t from_padded = ((uint64_t) from) << 32;
    std::vector<uint32_t> edge_list;
    auto edge_populater = PMA::range_func([&edge_list](uint64_t v) { 
        edge_list.push_back((uint32_t) v); 
    });
    pma.range(from_padded, from_padded | TO_ONES, edge_populater);
    return edge_list;
}


std::vector<std::pair<uint32_t, std::vector<uint32_t>>> PCSR::adjacency_lists() {
    
    std::vector<std::pair<uint32_t, std::vector<uint32_t>>> adjacencies;
    uint32_t curr_source = UINT32_MAX;
    uint32_t* curr_source_ptr = &curr_source;
    std::vector<uint32_t> adjacency;

    auto f = PMA::range_func([&adjacencies, &adjacency, curr_source_ptr](uint64_t v) {
        uint32_t source, dest;
        source = v >> 32;
        dest = v;
        
        if (source != *curr_source_ptr) {
            if (*curr_source_ptr != UINT32_MAX) {
                adjacencies.push_back(
                    std::make_pair(*curr_source_ptr, std::move(adjacency))
                );
                adjacency.clear();
            }
            *curr_source_ptr = source;
            adjacency.push_back(dest);
        }
        else {
            adjacency.push_back(dest);
        }
    });
    pma.range(0, UINT64_MAX - 1, f);
    
    if (curr_source != UINT32_MAX) {
        adjacencies.push_back(
            std::make_pair(curr_source, std::move(adjacency))
        );
    }

    return adjacencies;
}



#endif