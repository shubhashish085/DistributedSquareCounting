
#ifndef DYNAMIC_GRAPH_H
#define DYNAMIC_GRAPH_H

#include <unordered_map>
#include <vector>
#include <map>
#include "types.h"

class MasterGraph{

public:

    ui vertices_count;
    ui edges_count;
    ui max_degree;

    ui* degrees;
    NodeID* vertex_node_map;

    ui size_of_graph;
    ui capacity;
    ui numberOfPartitions;
    double slack;
    double gamma;
    double alpha;
    ui* partitionSizes;
    ui** partition_wise_neighbors;

    long long numberOfEdges = 0;
    long long numberOfEdgecut = 0;

    std::map<std::pair<VertexID, VertexID>, ui> wedge_map;

    MasterGraph(){
        
        vertices_count = 0;
        edges_count = 0;
        max_degree = 0;

    }

    MasterGraph(int worker_num, ui i_capacity, ui i_size_of_graph){
        
        vertices_count = 0;
        edges_count = 0;
        max_degree = 0;
        capacity = i_capacity;
        size_of_graph = i_size_of_graph;

        vertex_node_map = new NodeID[capacity];

        for(ui i = 0; i < capacity; i++){
            vertex_node_map[i] = -1;
        }

        numberOfPartitions = worker_num;
        partitionSizes = new ui[worker_num];

        for(ui i = 0; i < numberOfPartitions; i++){
            partitionSizes[i] = 0;
        }
        
        partition_wise_neighbors = new ui*[size_of_graph];
        for(ui i = 0; i < size_of_graph; i++){
            partition_wise_neighbors[i] = new ui[worker_num];
            for(ui j = 0; j < worker_num;j++){
                partition_wise_neighbors[i][j] = 0;
            }
        }

        std::cout << "Initialization Done " << std::endl;
    }

    ~MasterGraph() {
        
    }

    
    const ui getVerticesCount() const {
        return vertices_count;
    }

    const ui getEdgesCount() const {
        return edges_count;
    }

    ui* get_partition_wise_neighbors(VertexID& v) {
        return partition_wise_neighbors[v];
    }


    const ui getGraphMaxDegree() const {
        return max_degree;
    }

};


#endif