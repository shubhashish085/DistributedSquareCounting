#ifndef DSC_ALGORITHM_H
#define DSC_ALGORITHM_H

#include <map>
#include "graph.h"
#include "pcsr.h"
#include "types.h"

class CountingAlgorithm {

public:

    static std::pair<VertexID, VertexID> get_wedge_endpoint_pair(VertexID u, VertexID v);
    //static long long count_triangle(Graph* graph);
    static void count_square(Graph* graph);

    static void distributed_count_square(Graph* graph);
    static long long aggregate_square_count(std::map<std::pair<VertexID, VertexID>, ui>& wedge_map, long long&  global_cnt);

    static void distributed_dynamic_count_square(PCSR* graph);
};


#endif
