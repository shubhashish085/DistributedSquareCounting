#include "hpecworkergraph.hpp"

void HpecWorkerGraph::fillData(){

    ui offset;

    for(ui i = 0; i < edges.size(); i++){
        
    }

    for(ui i = 0; i < edges.size(); i++){
        VertexID src = edges[i].src;
        VertexID dst = edges[i].dst;
        offset = offsets[src] + neighbors_offset[src];
        neighbors[offset] = dst; 
        neighbors_offset[src]++;
    }


}