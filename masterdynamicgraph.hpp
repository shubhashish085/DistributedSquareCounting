
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