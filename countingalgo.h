#ifndef DISTRIBUTEDSQUARECOUNTING_ALGORITHM_H
#define DISTRIBUTEDSQUARECOUNTING_ALGORITHM_H

#include <map>
#include "graph.h"
#include "types.h"

class CountingAlgorithm {

public:

    static std::pair<VertexID, VertexID> get_wedge_endpoint_pair(VertexID u, VertexID v);
    //static long long count_triangle(Graph* graph);
    static long long count_square(Graph* graph);
    static long long distributed_count_square(Graph* graph);
};


#endif
