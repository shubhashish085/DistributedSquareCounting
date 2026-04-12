
#ifndef GRAPH_H
#define GRAPH_H

#include <unordered_map>
#include <vector>
#include <map>
#include "types.h"

class Graph{

public:

    std::vector<VertexID> vertices;
    std::map<VertexID, VertexID> ghost_vertex_map;
    std::map<VertexID, VertexID> ghost_vertex_idx_map;
    ui ghost_vertices_count;
    ui vertices_count;
    ui edges_count;
    ui max_degree;

    ui* degrees;

    ui* offsets;
    VertexID * neighbors;
    NodeID* partition;

    ui* g_offsets;
    VertexID * g_neighbors;
    
    std::unordered_map<VertexID, VertexID> vertex_idx_map;
    std::map<std::pair<VertexID, VertexID>, ui> wedge_map;
    std::map<std::pair<VertexID, VertexID>, ui> wedge_map_comm; 

    Graph(){
        
        vertices_count = 0;
        edges_count = 0;
        max_degree = 0;

        offsets = NULL;
        neighbors = NULL;
        wedge_map.clear();
    }

    ~Graph() {
        delete[] offsets;
        delete[] neighbors;
    }

public:
    void loadGraphFromFile(const std::string& file_path);
    void loadPartitionedGraphFromFile(const std::string& vertex_partition_file_path, const std::string& file_path, int partition_no);
    void loadDBPartitionedGraphFromFile(const std::string& file_path, VertexID minVertexID, VertexID maxVertexID);
    void transformToAugmentedGraph(Graph* data_graph, Graph* augmented_graph);
    void printGraphMetaData();
    long long count_exact_square();
    long long count_exact_square_parallel();

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

    ui * getVertexNeighbors_partitioned(const VertexID vid, ui& count) const {
        
        VertexID id = (vertex_idx_map.find(vid))->second;
        count = offsets[id + 1] - offsets[id]; // used for neighbor count
        return neighbors + offsets[id];
    }

    void getNeighborCount(const VertexID id, ui& count) const {
        count = offsets[id + 1] - offsets[id];
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