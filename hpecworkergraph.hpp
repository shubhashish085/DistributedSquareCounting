
#ifndef HPEC_WORKER_GRAPH_H
#define HPEC_WORKER_GRAPH_H

#include <unordered_map>
#include <vector>
#include <map>
#include "types.h"

class HpecWorkerGraph{

public:

    std::vector<VertexID> vertices;
    std::vector<Edge> edges;
    ui vertices_count;
    ui edges_count;

    

    ui* degrees;

    ui* offsets;
    VertexID * neighbors;
    ui* neighbors_offset;

    std::map<VertexID, VertexID> vtx_map;
    std::map<std::pair<VertexID, VertexID>, ui> wedge_map;

    HpecWorkerGraph(){
        
        vertices_count = 0;
        edges_count = 0;

        offsets = NULL;
        neighbors = NULL;
    }

    HpecWorkerGraph(GraphMetaData& meta_data){

        vertices_count = meta_data.vtx_cnt;
        edges_count = meta_data.edge_cnt;
        edges.reserve(edges_count);


        degrees = new ui[vertices_count];
        offsets = new VertexID[vertices_count + 1];
        neighbors_offset = new ui[vertices_count];
        neighbors = new VertexID[edges_count];

        std::fill(neighbors_offset, neighbors_offset + vertices_count, 0);
        offsets[0] = 0;        
    }

    ~HpecWorkerGraph() {
        delete[] offsets;
        delete[] neighbors;
    }

    void fillData();

    bool insert_edge(Edge& edge){
        edges.push_back(edge);
    }   

    const ui* getOffsets() const {
        return offsets;
    }

    const ui* getNeighbors() const {
        return neighbors;
    }

    const ui getVerticesCount() const {
        return vertices_count;
    }

    const ui getEdgesCount() const {
        return edges_count;
    }

    const ui getVertexDegree(const VertexID id) const {
        return offsets[id + 1] - offsets[id];
    }

    ui * getVertexNeighbors(const VertexID id, ui& count) const {
        count = offsets[id + 1] - offsets[id]; // used for neighbor count
        return neighbors + offsets[id];
    }

    void getNeighborCount(const VertexID id, ui& count) const {
        count = offsets[id + 1] - offsets[id];
    }

    bool is_greater_in_order(VertexID u, VertexID v) {
        if(degrees[u] > degrees[v]){
            return true;
        }

        if(degrees[u] < degrees[v]){
            return false;
        }

        return u > v;
    }

    
    bool checkEdgeExistence(VertexID u, VertexID v) const {
        if (getVertexDegree(u) < getVertexDegree(v)) {
            std::swap(u, v);
        }
        ui count = 0;
        const VertexID* neighbors =  getVertexNeighbors(v, count);

        int begin = 0;
        int end = count - 1;
        while (begin <= end) {
            int mid = begin + ((end - begin) >> 1);
            if (neighbors[mid] == u) {
                return true;
            }
            else if (neighbors[mid] > u)
                end = mid - 1;
            else
                begin = mid + 1;
        }

        return false;
    }

};


#endif