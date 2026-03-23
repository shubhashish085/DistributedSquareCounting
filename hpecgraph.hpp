
#ifndef HPEC_GRAPH_H
#define HPEC_GRAPH_H

#include <unordered_map>
#include <vector>
#include <map>
#include "types.h"

class HpecGraph{

public:

    std::vector<VertexID> vertices;
    ui vertices_count;
    ui edges_count;
    ui max_degree;

    ui* degrees;

    ui* offsets;
    VertexID * neighbors;
    VertexID * ordered_nbrs;

    NodeID* vertex_partition_map;

    HpecGraph(){
        
        vertices_count = 0;
        edges_count = 0;
        max_degree = 0;

        offsets = NULL;
        neighbors = NULL;
    }

    ~HpecGraph() {
        delete[] offsets;
        delete[] neighbors;
    }

    void loadGraphFromFile(const std::string& file_path);
    

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


    const ui getGraphMaxDegree() const {
        return max_degree;
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

    void order_vertices(VertexID& u, VertexID& v, NodeID& dst){

        VertexID temp;

        if(is_greater_in_order(u, v)){
            temp = v;
            v = u;
            u = temp;
        }

        dst = vertex_partition_map[u];
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