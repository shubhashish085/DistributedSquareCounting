#include "partition.hpp"
#include <cstring>
#include <cmath>
#include <vector>
#include <random>


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


void GraphPartitioning::hash_dyn_partition(MasterGraph* master_graph, Edge& edge, NodeID& u_partition, NodeID& v_partition){

    VertexID u = edge.src;
    VertexID v = edge.dst;

    ui* u_numberOfNeighbours = master_graph->get_partition_wise_neighbors(u);

    int argmax = -1;    

    argmax = hash_vertex(u, master_graph->numberOfPartitions);

    for(int i = 0 ; i < master_graph->numberOfPartitions ; i++) {
        master_graph->numberOfEdges += u_numberOfNeighbours[i];
        if(i != argmax) {
            master_graph->numberOfEdgecut += u_numberOfNeighbours[i];
        }
    }

    u_partition = argmax;

    ui* v_numberOfNeighbours = master_graph->get_partition_wise_neighbors(v);

    argmax = -1;    

    argmax = hash_vertex(v, master_graph->numberOfPartitions);

    for(int i = 0 ; i < master_graph->numberOfPartitions ; i++) {
        master_graph->numberOfEdges += v_numberOfNeighbours[i];
        if(i != argmax) {
            master_graph->numberOfEdgecut += v_numberOfNeighbours[i];
        }
    }

    v_partition = argmax;

    u_numberOfNeighbours[v_partition] += 1;
    v_numberOfNeighbours[u_partition] += 1;
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

    if(u_partition == -1){

        u_numberOfNeighbours = master_graph->get_partition_wise_neighbors(u);
        
        for(int i = 0; i < master_graph->numberOfPartitions; i++){
            
            next = (1 - (master_graph->partitionSizes[i] / master_graph->capacity)) * u_numberOfNeighbours[i];
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

    if(v_partition == -1){

        v_numberOfNeighbours = master_graph->get_partition_wise_neighbors(v);

        for(int i = 0; i < master_graph->numberOfPartitions; i++){
            
            next = (1 - (master_graph->partitionSizes[i] / master_graph->capacity)) * v_numberOfNeighbours[i];
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
