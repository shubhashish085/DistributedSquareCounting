#include "countingalgo.h"
#include <mpi.h>


std::pair<VertexID, VertexID> CountingAlgorithm::get_wedge_endpoint_pair(VertexID u, VertexID v){

    if(u > v){
        VertexID tmp = v;
        v = u;
        u = tmp;
    }

    return std::make_pair(u, v);
}

long long combinations(int n, int r) {
    if (r < 0 || r > n) return 0;
    if (r == 0 || r == n) return 1;
    if (r > n / 2) r = n - r;
    long long res = 1;
    for (int k = 1; k <= r; ++k) {
        res = res * (n - k + 1);
        res /= k;
    }
    return res;
}


void CountingAlgorithm::count_square(Graph* graph){

    VertexID* neighbors;
    ui nbr_count;
    std::pair<VertexID, VertexID> search_pair;

    for(ui i = 0; i < graph->getVerticesCount(); i++){
        neighbors = graph->getVertexNeighbors(i, nbr_count);

        for(ui j = 0; j < nbr_count; j++){
            for(ui k = j+1; k < nbr_count; k++){

                search_pair = get_wedge_endpoint_pair(neighbors[j], neighbors[k]);                                 
                auto search = (graph->wedge_map).find(search_pair);

                if(search == (graph->wedge_map).end()){
                    graph->wedge_map[search_pair] = 1;
                }else{
                    graph->wedge_map[search_pair] = graph->wedge_map[search_pair] + 1;
                }
            }
        }
    }
}


void CountingAlgorithm::distributed_count_square(Graph* graph){

    VertexID* neighbors;
    ui nbr_count, wedge_count;
    std::pair<VertexID, VertexID> search_pair;

    VertexID wedge_begin, wedge_end;

    std::vector<VertexID> buffer;
    std::pair<VertexID, VertexID> search_pair;

    long long total_sq_count = 0;    

    int world_size, world_rank, dest_rank = 0;
    int buffer_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    MPI_Status buffer_size_status, buffer_status;

    
    if (world_rank == 0)
    {

        for (int rank = 1; rank < world_size; ++rank)
        {
            MPI_Recv(&buffer_size, 1, MPI_INT, rank, TAG_BUFFER_SIZE, MPI_COMM_WORLD, &buffer_size_status);

            std::vector<VertexID> recv_buffer(buffer_size);
            MPI_Recv(recv_buffer.data(), world_size, MPI_UNSIGNED, rank, TAG_BUFFER, MPI_COMM_WORLD, &buffer_status);

            ui i = 0;

            while(i < recv_buffer.size()){
                wedge_begin = recv_buffer[i];
                wedge_end = recv_buffer[i + 1];
                wedge_count = recv_buffer[i + 2];

                search_pair = std::make_pair(wedge_begin, wedge_end);

                if((graph->wedge_map).find(search_pair) != (graph->wedge_map).end()){
                    graph->wedge_map[search_pair] = graph->wedge_map[search_pair] + wedge_count;
                }else{
                    graph->wedge_map[search_pair] = wedge_count;
                }               
                
                i = i + 3;
            }
        }

    }
    else
    {

        for(ui i = 0; i < graph->getVerticesCount(); i++){
            neighbors = graph->getVertexNeighbors(i, nbr_count);

            for(ui j = 0; j < nbr_count; j++){
                for(ui k = j+1; k < nbr_count; k++){

                    search_pair = get_wedge_endpoint_pair(neighbors[j], neighbors[k]);                                 
                    auto search = (graph->wedge_map).find(search_pair);

                    if(search == (graph->wedge_map).end()){
                        graph->wedge_map[search_pair] = 1;
                    }else{
                        graph->wedge_map[search_pair] = graph->wedge_map[search_pair] + 1;
                    }
                }
            }

        }

        for (const auto& [key, value] : graph->wedge_map){
            buffer.push_back(key.first);
            buffer.push_back(key.second);
            buffer.push_back(value);
        }

        int buffer_size = buffer.size();
        MPI_Send(&buffer_size, 1, MPI_INT, dest_rank, TAG_BUFFER_SIZE, MPI_COMM_WORLD);

        MPI_Send(buffer.data(), buffer_size, MPI_INT, dest_rank, TAG_BUFFER, MPI_COMM_WORLD);
    }

}


void CountingAlgorithm::distributed_dynamic_count_square(PCSR* graph){

    std::vector<VertexID> neighbors;
    ui nbr_count;
    std::pair<VertexID, VertexID> search_pair;

    for(ui i = 0; i < graph->get_vertices_count(); i++){
        neighbors = graph->edges(i);
        nbr_count = neighbors.size();

        for(ui j = 0; j < nbr_count; j++){
            for(ui k = j+1; k < nbr_count; k++){
                search_pair = get_wedge_endpoint_pair(neighbors[j], neighbors[k]);                                 
                auto search = (graph->wedge_map).find(search_pair);

                if(search == (graph->wedge_map).end()){
                    graph->wedge_map[search_pair] = 1;
                }else{
                    graph->wedge_map[search_pair] = graph->wedge_map[search_pair] + 1;
                }
            }
        }
    }
}





long long CountingAlgorithm::aggregate_square_count(std::map<std::pair<VertexID, VertexID>, ui>& wedge_map, long long&  global_cnt){

    for (const auto& [key, value] : wedge_map){
        global_cnt += combinations(value, 2);
    }

    return global_cnt;
}
