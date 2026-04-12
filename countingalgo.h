#ifndef DSC_ALGORITHM_H
#define DSC_ALGORITHM_H

#include <map>
#include "graph.h"
#include "pcsr.h"
#include "hpecworkergraph.hpp"
#include "types.h"

class CountingAlgorithm {

public:

    static std::pair<VertexID, VertexID> get_wedge_endpoint_pair(VertexID u, VertexID v);
    //static long long count_triangle(Graph* graph);
    static void count_square(Graph* graph);

    static void distributed_count_square(Graph* graph);
    static void distributed_count_square_in_partitioned_graph(const std::string& vertex_partition_file_path, const std::string& file_path, int partition_no);
    static void dist_opt_count_square_in_partitioned_graph(const std::string& file_path, const std::string& vertex_partition_file_path);

    static long long aggregate_square_count(std::map<std::pair<VertexID, VertexID>, ui>& wedge_map, long long&  global_cnt);

    static void distributed_dynamic_count_square(PCSR* graph);

    static long long communication_cost_analysis(const std::string& vertex_partition_file_path, const std::string& file_path, int partition_no);
    static long long naive_comm_cost_analysis(Graph* graph, const std::string& vertex_partition_file_path, int partition_no);
    static long long dist_comm_cost_analysis(PCSR* graph, std::vector<Edge> batched_edges, int rank);
};


#endif
