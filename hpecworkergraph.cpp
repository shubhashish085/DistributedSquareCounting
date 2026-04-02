#include "hpecworkergraph.hpp"

void HpecWorkerGraph::fillData(){

    ui offset, idx = 0;
    ui prev_vtx = INVALID_VID;

    for(ui i = 0; i < edges.size(); i++){
        if(edges[i].src != prev_vtx){
            vtx_map[edges[i].src] = idx;
            prev_vtx = edges[i].src;
            idx++;
        }
    }

    for(ui i = 0; i < edges.size(); i++){
        VertexID src = vtx_map[edges[i].src];
        VertexID dst = edges[i].dst;
        offset = offsets[src] + neighbors_offset[src];
        neighbors[offset] = dst; 
        neighbors_offset[src]++;
    }
}