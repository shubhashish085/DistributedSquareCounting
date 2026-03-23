#include "pcsr.h"

PCSR::PCSR(VertexID size) : pma(size) {}

uint64_t PCSR::make_edge_tuple(VertexID from, VertexID to) { return (((uint64_t) from) << 32) | to; }
std::pair<VertexID, VertexID> PCSR::get_edge_tuple(uint64_t edge) { return std::make_pair((VertexID)(edge >> 32), (VertexID)(edge << 32)); } 

VertexID PCSR::size(){
    return pma.size();
}

ui PCSR::get_vertices_count(){
    return pma.vertices_count;
}

void PCSR::insert_edge(VertexID from, VertexID to) {
    uint64_t edge = make_edge_tuple(from, to);
    pma.insert(edge);
}

void PCSR::delete_edge(VertexID from, VertexID to) {
    uint64_t edge = make_edge_tuple(from, to);
    pma.delete_edge(edge);
}

bool PCSR::query_edge(VertexID from, VertexID to) {
    uint64_t edge = make_edge_tuple(from, to);
    return pma.query(edge);
}

std::vector<VertexID> PCSR::edges(VertexID from) {
    uint64_t from_padded = ((uint64_t) from) << 32;
    std::vector<VertexID> edge_list;
    auto edge_populater = PMA::range_func([&edge_list](uint64_t v) { 
        edge_list.push_back((VertexID) v); 
    });
    pma.range(from_padded, from_padded | TO_ONES, edge_populater);
    return edge_list;
}


std::vector<std::pair<VertexID, std::vector<VertexID>>> PCSR::adjacency_lists() {
    
    std::vector<std::pair<VertexID, std::vector<VertexID>>> adjacencies;
    VertexID curr_source = UINT32_MAX;
    VertexID* curr_source_ptr = &curr_source;
    std::vector<VertexID> adjacency;

    auto f = PMA::range_func([&adjacencies, &adjacency, curr_source_ptr](uint64_t v) {
        VertexID source, dest;
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
