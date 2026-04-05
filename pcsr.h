#ifndef DISTRIBUTEDSQUARECOUNTING_PCSR_H
#define DISTRIBUTEDSQUARECOUNTING_PCSR_H

#include <climits>
#include <cmath>
#include <iostream>
#include <map>
#include <unordered_set>
#include "pma.h"


class PCSR{

    public:
        PCSR(VertexID size);
        VertexID size();
        ui get_vertices_count();
        
        void insert_edge(VertexID from, VertexID to);
        void delete_edge(VertexID from, VertexID to);
        bool query_edge(VertexID from, VertexID to);
        
        std::vector<VertexID> edges(VertexID from);
        std::vector<std::pair<VertexID, std::vector<VertexID>>> adjacency_lists();

        std::unordered_map<VertexID, VertexID> vertex_idx_map;
        std::unordered_map<VertexID, NodeID> vertex_ptn_map;
        std::map<std::pair<VertexID, VertexID>, ui> wedge_map;
        std::map<std::pair<VertexID, VertexID>, ui> wedge_map_comm; 
    
        static uint64_t make_edge_tuple(VertexID from, VertexID to);
        static std::pair<VertexID, VertexID> get_edge_tuple(uint64_t edge);
        PMA pma;
        const VertexID TO_ONES = 0xFFFFFFFF;
};



#endif