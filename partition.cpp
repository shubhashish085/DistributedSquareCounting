#include "partition.hpp"
#include <cstring>
#include <cmath>
#include <vector>
#include <random>
#include <algorithm>


void GraphPartitioning::even_degree_partition(Graph* data_graph, VertexID*& vtx_partition_array, ui& n_partition) {


    ui* cand_degree_offset = new ui[data_graph->getVerticesCount() + 1];
    cand_degree_offset[0] = 0;
    ui* candidate_limit = new ui[n_partition];

    for(ui j = 0; j < data_graph->getVerticesCount() ; j++){
        cand_degree_offset[j + 1] = cand_degree_offset[j] + data_graph->getVertexDegree(j);
    }

    ui total_degree = cand_degree_offset[data_graph->getVerticesCount()];
    ui avg_degree = total_degree / n_partition;

    ui last_node_degree_offset = 0;
    ui node_idx = 0;

    for(ui j = 0; j < data_graph->getVerticesCount(); j++){
        if(node_idx == n_partition - 1){
            candidate_limit[node_idx++] = data_graph->getVerticesCount() - 1;
            last_node_degree_offset = cand_degree_offset[data_graph->getVerticesCount()];
            break;
        }else if(j == data_graph->getVerticesCount() - 1){
            candidate_limit[node_idx++] = j;
            last_node_degree_offset = cand_degree_offset[j + 1];
        }else if(cand_degree_offset[j + 1] - last_node_degree_offset >= avg_degree){
            candidate_limit[node_idx++] = j;
            last_node_degree_offset = cand_degree_offset[j + 1];
        }
    }

    ui start_point = 0, k  = 0;
    while (k < n_partition){

        for(ui i = start_point; i <= candidate_limit[k]; i++){
            vtx_partition_array[i] = k;
        }

        start_point = candidate_limit[k];
        k++;
    }
        
}


void GraphPartitioning::hpec_even_degree_partition(HpecGraph* data_graph, ui& n_partition) {


    ui* cand_degree_offset = new ui[data_graph->getVerticesCount() + 1];
    cand_degree_offset[0] = 0;
    ui* candidate_limit = new ui[n_partition];

    for(ui j = 0; j < data_graph->getVerticesCount() ; j++){
        cand_degree_offset[j + 1] = cand_degree_offset[j] + data_graph->getVertexDegree(j);
    }

    ui total_degree = cand_degree_offset[data_graph->getVerticesCount()];
    ui avg_degree = total_degree / n_partition;

    ui last_node_degree_offset = 0;
    ui node_idx = 0;

    for(ui j = 0; j < data_graph->getVerticesCount(); j++){
        if(node_idx == n_partition - 1){
            candidate_limit[node_idx++] = data_graph->getVerticesCount() - 1;
            last_node_degree_offset = cand_degree_offset[data_graph->getVerticesCount()];
            break;
        }else if(j == data_graph->getVerticesCount() - 1){
            candidate_limit[node_idx++] = j;
            last_node_degree_offset = cand_degree_offset[j + 1];
        }else if(cand_degree_offset[j + 1] - last_node_degree_offset >= avg_degree){
            candidate_limit[node_idx++] = j;
            last_node_degree_offset = cand_degree_offset[j + 1];
        }
    }

    ui start_point = 0, k  = 0;
    while (k < n_partition){

        for(ui i = start_point; i <= candidate_limit[k]; i++){
            data_graph->vertex_partition_map[i] = k;
        }

        start_point = candidate_limit[k];
        k++;
    }
        
}



void GraphPartitioning::even_degree_partition(Graph* data_graph, ui& n_partition, ui*& partition_limit) {


    ui* cand_degree_offset = new ui[data_graph->getVerticesCount() + 1];
    cand_degree_offset[0] = 0;

    for(ui j = 0; j < data_graph->getVerticesCount() ; j++){
        cand_degree_offset[j + 1] = cand_degree_offset[j] + data_graph->getVertexDegree(j);
    }

    ui total_degree = cand_degree_offset[data_graph->getVerticesCount()];
    ui avg_degree = total_degree / n_partition;

    ui last_node_degree_offset = 0;
    ui node_idx = 0;

    for(ui j = 0; j < data_graph->getVerticesCount(); j++){
        if(node_idx == n_partition - 1){
            partition_limit[node_idx++] = data_graph->getVerticesCount() - 1;
            last_node_degree_offset = cand_degree_offset[data_graph->getVerticesCount()];
            break;
        }else if(j == data_graph->getVerticesCount() - 1){
            partition_limit[node_idx++] = j;
            last_node_degree_offset = cand_degree_offset[j + 1];
        }else if(cand_degree_offset[j + 1] - last_node_degree_offset >= avg_degree){
            partition_limit[node_idx++] = j;
            last_node_degree_offset = cand_degree_offset[j + 1];
        }
    }        
}

void GraphPartitioning::partition_with_degree_refinement(Graph* data_graph, NodeID*& metis_part, ui& n_partition, NodeID*& final_part){

    long long* load = new long long[n_partition];
    std::fill(load, load + n_partition, 0);

    ui* cand_degree_offset = new ui[data_graph->getVerticesCount() + 1];
    cand_degree_offset[0] = 0;

    for(ui j = 0; j < data_graph->getVerticesCount() ; j++){
        cand_degree_offset[j + 1] = cand_degree_offset[j] + data_graph->getVertexDegree(j);
    }

    ui total_degree = cand_degree_offset[data_graph->getVerticesCount()];
    ui target_degree = total_degree / n_partition;

    for(ui i = 0; i < data_graph->getVerticesCount(); i++){
        load[metis_part[i]] += data_graph->degrees[i];
    }

    std::vector<ui> order(data_graph->getVerticesCount());
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(), [&](int a, int b) {
        return data_graph->degrees[a] > data_graph->degrees[b];
    });

    for (int v : order) {
        int bestPart = metis_part[v];
        int minLoad = load[bestPart];

        for (int p = 0; p < n_partition; p++) {
            if (load[p] < minLoad) {
                bestPart = p;
                minLoad = load[p];
            }
        }

        if (bestPart != metis_part[v]) {
            load[metis_part[v]] -= data_graph->degrees[v];
            //metis_part[v] = bestPart;
            load[bestPart] += data_graph->degrees[v];
        }

        final_part[v] = bestPart;        
    }    
}

void GraphPartitioning::partition_with_target_degree_refinement(Graph* data_graph, NodeID*& metis_part, ui& n_partition, NodeID*& final_part, double threshold){

    long long* load = new long long[n_partition];
    std::fill(load, load + n_partition, 0);

    std::memcpy(final_part, metis_part, sizeof(NodeID));

    std::vector<NodeID> highly_loaded_partitions;
    std::vector<NodeID> less_loaded_partitions;

    ui* cand_degree_offset = new ui[data_graph->getVerticesCount() + 1];
    cand_degree_offset[0] = 0;

    for(ui j = 0; j < data_graph->getVerticesCount() ; j++){
        cand_degree_offset[j + 1] = cand_degree_offset[j] + data_graph->getVertexDegree(j);
    }

    ui total_degree = cand_degree_offset[data_graph->getVerticesCount()];
    ui target_degree = total_degree / n_partition;

    for(ui i = 0; i < data_graph->getVerticesCount(); i++){
        load[metis_part[i]] += data_graph->degrees[i];
    }

    for(ui i = 0; i < n_partition; i++){
        if(load[i] > threshold * target_degree){
            highly_loaded_partitions.push_back(i);
        }else if(load[i] < target_degree){
            less_loaded_partitions.push_back(i);
        }
    }

    std::vector<VertexID> order(data_graph->getVerticesCount());
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(), [&](VertexID a, VertexID b) {
        return data_graph->degrees[a] > data_graph->degrees[b];
    });

    std::sort(less_loaded_partitions.begin(), less_loaded_partitions.end(), [&](int a, int b) {
        return load[a] < load[b];
    });

    int bestPart;
    std::vector<VertexID> ptn_vertices;
    ui loaded_ptn_idx;  

    for(ui i = 0; i < highly_loaded_partitions.size(); i++){
        loaded_ptn_idx = highly_loaded_partitions[i];
        for(ui j = 0; j < order.size(); j++){
            if(metis_part[order[j]] == loaded_ptn_idx){
                ptn_vertices.push_back(order[j]);
            }            
        }

        for(ui j = 0; j < ptn_vertices.size(); j++){
            if(less_loaded_partitions.size() > 0 && load[less_loaded_partitions[0]] < target_degree){
                final_part[ptn_vertices[j]] = less_loaded_partitions[0];
                load[less_loaded_partitions[0]] += data_graph->degrees[ptn_vertices[j]];
                load[loaded_ptn_idx] -= data_graph->degrees[ptn_vertices[j]];

            }else if (less_loaded_partitions.size() > 0){
                less_loaded_partitions.erase(less_loaded_partitions.begin());
            }

            if(load[loaded_ptn_idx] <= threshold * target_degree){
                break;
            }
        }

        ptn_vertices.clear();
    }   
}



long long GraphPartitioning::hash_vertex(VertexID& v, ui& numberOfPartitions) {
    uint32_t x = v;

    // 32-bit mix (MurmurHash3 finalizer style)
    x ^= x >> 16;
    x *= 0x85ebca6bU;
    x ^= x >> 13;
    x *= 0xc2b2ae35U;
    x ^= x >> 16;

    return (static_cast<long long>(x % (numberOfPartitions - 1)) + 1);
}



void GraphPartitioning::hash_dyn_partition(MasterGraph* master_graph, Edge& edge, NodeID& u_partition, NodeID& v_partition){

    VertexID u = edge.src;
    VertexID v = edge.dst;

    //std::cout << "Partitioning Started" << std::endl;

    ui* u_numberOfNeighbours = master_graph->get_partition_wise_neighbors(u);

    //std::cout << "--------------- " << u_numberOfNeighbours[0];

    int argmax = -1;    

    argmax = hash_vertex(u, master_graph->numberOfPartitions);

    //std::cout << "Hash Done" << std::endl;

    for(int i = 1 ; i < master_graph->numberOfPartitions ; i++) {
        master_graph->numberOfEdges += u_numberOfNeighbours[i];
        if(i != argmax) {
            master_graph->numberOfEdgecut += u_numberOfNeighbours[i];
        }
    }

    u_partition = argmax;

    //std::cout << "U partition : " << u_partition << std::endl;

    ui* v_numberOfNeighbours = master_graph->get_partition_wise_neighbors(v);

    argmax = -1;    

    argmax = hash_vertex(v, master_graph->numberOfPartitions);

    //std::cout << "Hash Done" << std::endl;

    for(int i = 1 ; i < master_graph->numberOfPartitions ; i++) {
        master_graph->numberOfEdges += v_numberOfNeighbours[i];
        if(i != argmax) {
            master_graph->numberOfEdgecut += v_numberOfNeighbours[i];
        }
    }

    v_partition = argmax;

    //std::cout << "U partition : " << u_partition << std::endl;

    u_numberOfNeighbours[v_partition] += 1;
    v_numberOfNeighbours[u_partition] += 1;

    //std::cout << "Partition Ended" << std::endl;

    edge.src_ptn = u_partition;
    edge.dst_ptn = v_partition;
}

void GraphPartitioning::ldg_dyn_partition(MasterGraph* master_graph, Edge& edge, NodeID& u_partition, NodeID& v_partition){

    VertexID u = edge.src;
    VertexID v = edge.dst;

    double result = -1, next;
    int argmax = -1;

    ui* u_numberOfNeighbours;
    ui* v_numberOfNeighbours;

    std::vector<int> tie_breaker(master_graph->numberOfPartitions);

    std::random_device rd; 
    std::mt19937 gen(rd()); 

    u_partition = master_graph->vertex_node_map[u];
    v_partition = master_graph->vertex_node_map[v];

    //std::cout << "Finding u partition" << std::endl;

    if(u_partition == -1){

        u_numberOfNeighbours = master_graph->get_partition_wise_neighbors(u);
        
        for(int i = 1; i < master_graph->numberOfPartitions; i++){
            
            next = (1.0 - (double)(master_graph->partitionSizes[i] / master_graph->capacity)) * u_numberOfNeighbours[i];
            if(next > result){
                if(master_graph->partitionSizes[i] < master_graph->capacity){
                    result = next;
                    argmax = i;
                    tie_breaker.clear();
                    tie_breaker.push_back(argmax);
                }
            }else if(next == result){
                tie_breaker.push_back(i);
            }
        }

        std::uniform_int_distribution<> u_distrib(0, tie_breaker.size() - 1);

        int index = u_distrib(gen);
        argmax = tie_breaker[index];
        u_partition = argmax;

        master_graph->partitionSizes[u_partition] += 1;
    }

    tie_breaker.clear();

    //std::cout << "Finished u partition : " << u_partition << std::endl;

    result = -1;

    //std::cout << "Finding v partition : " << v << std::endl;

    if(v_partition == -1){

        v_numberOfNeighbours = master_graph->get_partition_wise_neighbors(v);

        for(int i = 1; i < master_graph->numberOfPartitions; i++){
            
            next = (1.0 - (double)(master_graph->partitionSizes[i] / master_graph->capacity)) * v_numberOfNeighbours[i];
            if(next > result){
                if(master_graph->partitionSizes[i] < master_graph->capacity){
                    result = next;
                    argmax = i;
                    tie_breaker.clear();
                    tie_breaker.push_back(argmax);
                }
            }else if(next == result){
                tie_breaker.push_back(i);
            }
        }

        std::uniform_int_distribution<> v_distrib(0, tie_breaker.size() - 1);

        int index = v_distrib(gen);
        argmax = tie_breaker[index];
        v_partition = argmax;

        master_graph->partitionSizes[v_partition] += 1;
    }

    //std::cout << "Finished v partition : " << v_partition << std::endl;

    if(u_partition != v_partition){
        master_graph->numberOfEdgecut += 1;
    }    

    u_numberOfNeighbours[v_partition] += 1;
    v_numberOfNeighbours[u_partition] += 1;

    edge.src_ptn = u_partition;
    edge.dst_ptn = v_partition;
}


void GraphPartitioning::fennel_dyn_partition(MasterGraph* master_graph, Edge& edge, NodeID& u_partition, NodeID& v_partition){

    VertexID u = edge.src;
    VertexID v = edge.dst;

    double result = -1, next;
    int argmax = -1;

    std::vector<int> tie_breaker(master_graph->numberOfPartitions);
    std::random_device rd; 
    std::mt19937 gen(rd());

    u_partition = master_graph->vertex_node_map[u];
    v_partition = master_graph->vertex_node_map[v];

    ui* u_numberOfNeighbours = master_graph->get_partition_wise_neighbors(u);

    if(u_partition == -1){

        for(int i = 0; i < master_graph->numberOfPartitions; i++){
        
            next = u_numberOfNeighbours[i] - (master_graph->gamma * master_graph->alpha * std::pow((double)(master_graph->partitionSizes[i]), (master_graph->gamma - 1.0)));
            if(next > result){
                if(master_graph->partitionSizes[i] < master_graph->capacity){
                    result = next;
                    argmax = i;
                    tie_breaker.clear();
                    tie_breaker.push_back(argmax);
                }
            }else if(next == result){
                tie_breaker.push_back(i);
            }
        }

    
        std::uniform_int_distribution<> u_distrib(0, tie_breaker.size() - 1);

        int index = u_distrib(gen);
        argmax = tie_breaker[index];
        u_partition = argmax;

        master_graph->partitionSizes[u_partition] += 1;
    }    

    tie_breaker.clear();

    ui* v_numberOfNeighbours = master_graph->get_partition_wise_neighbors(v);

    if(v_partition == -1){

        for(int i = 0; i < master_graph->numberOfPartitions; i++){
        
            next = v_numberOfNeighbours[i] - (master_graph->gamma * master_graph->alpha * std::pow((double)(master_graph->partitionSizes[i]), (master_graph->gamma - 1.0)));;
            if(next > result){
                if(master_graph->partitionSizes[i] < master_graph->capacity){
                    result = next;
                    argmax = i;
                    tie_breaker.clear();
                    tie_breaker.push_back(argmax);
                }
            }else if(next == result){
                tie_breaker.push_back(i);
            }
        }

        std::uniform_int_distribution<> v_distrib(0, tie_breaker.size() - 1);

        int index = v_distrib(gen);
        argmax = tie_breaker[index];
        v_partition = argmax;

        master_graph->partitionSizes[v_partition] += 1;        
    }

    if(u_partition != v_partition){
        master_graph->numberOfEdgecut += 1;
    }   

    u_numberOfNeighbours[v_partition] += 1;
    v_numberOfNeighbours[u_partition] += 1;

}
