#include "countingalgo.h"
#include "distributioncoordinator.hpp"
#include <fstream>
#include <mpi.h>

std::pair<VertexID, VertexID> CountingAlgorithm::get_wedge_endpoint_pair(VertexID u, VertexID v)
{

    if (u > v)
    {
        VertexID tmp = v;
        v = u;
        u = tmp;
    }

    return std::make_pair(u, v);
}

long long combinations(int n, int r)
{
    if (r < 0 || r > n)
        return 0;
    if (r == 0 || r == n)
        return 1;
    if (r > n / 2)
        r = n - r;
    long long res = 1;
    for (int k = 1; k <= r; ++k)
    {
        res = res * (n - k + 1);
        res /= k;
    }
    return res;
}

void CountingAlgorithm::count_square(Graph *graph)
{

    VertexID *neighbors;
    ui nbr_count;
    std::pair<VertexID, VertexID> search_pair;

    for (ui i = 0; i < graph->getVerticesCount(); i++)
    {
        neighbors = graph->getVertexNeighbors(i, nbr_count);

        for (ui j = 0; j < nbr_count; j++)
        {
            for (ui k = j + 1; k < nbr_count; k++)
            {

                search_pair = get_wedge_endpoint_pair(neighbors[j], neighbors[k]);
                auto search = (graph->wedge_map).find(search_pair);

                if (search == (graph->wedge_map).end())
                {
                    graph->wedge_map[search_pair] = 1;
                }
                else
                {
                    graph->wedge_map[search_pair] = graph->wedge_map[search_pair] + 1;
                }
            }
        }
    }
}

void CountingAlgorithm::distributed_count_square_in_partitioned_graph(const std::string &vertex_partition_file_path, const std::string &file_path, int partition_no)
{

    VertexID *neighbors;
    ui nbr_count, wedge_count;
    std::pair<VertexID, VertexID> search_pair;

    VertexID wedge_begin, wedge_end;
    long long total_sq_count = 0;

    int world_size, world_rank, dest_rank = 0;
    int buffer_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    MPI_Status buffer_size_status, buffer_status;

    MPI_Datatype MPI_TYPE_WEDGE_CNT;

    int lenAttr[WedgeCnt::szAttr] = {1, 1, 1};
    MPI_Datatype types[WedgeCnt::szAttr] = {MPI_UNSIGNED, MPI_UNSIGNED, MPI_UNSIGNED};

    MPI_Aint offsets[WedgeCnt::szAttr];

    offsets[0] = offsetof(WedgeCnt, first_vtx);
    offsets[1] = offsetof(WedgeCnt, third_vtx);
    offsets[2] = offsetof(WedgeCnt, cnt);

    MPI_Type_create_struct(WedgeCnt::szAttr, lenAttr, offsets, types, &MPI_TYPE_WEDGE_CNT);
    MPI_Type_commit(&MPI_TYPE_WEDGE_CNT);

    if (world_rank == 0)
    {

        std::map<std::pair<VertexID, VertexID>, ui> wedge_map;

        for (int rank = 1; rank < world_size; ++rank)
        {
            MPI_Recv(&buffer_size, 1, MPI_INT, rank, TAG_BUFFER_SIZE, MPI_COMM_WORLD, &buffer_size_status);

            std::vector<WedgeCnt> recv_buffer(buffer_size);
            MPI_Recv(recv_buffer.data(), world_size, MPI_TYPE_WEDGE_CNT, rank, TAG_BUFFER, MPI_COMM_WORLD, &buffer_status);

            ui i = 0;

            while (i < recv_buffer.size())
            {

                search_pair = std::make_pair(recv_buffer[i].first_vtx, recv_buffer[i].third_vtx);

                if ((wedge_map).find(search_pair) != wedge_map.end())
                {
                    wedge_map[search_pair] = wedge_map[search_pair] + recv_buffer[i].cnt;
                }
                else
                {
                    wedge_map[search_pair] = recv_buffer[i].cnt;
                }

                i++;
            }
        }
    }
    else
    {
        Graph *graph = new Graph();
        graph->loadPartitionedGraphFromFile(vertex_partition_file_path, file_path, world_rank - 1);

        for (ui i = 0; i < (graph->vertices).size(); i++)
        {
            neighbors = graph->getVertexNeighbors(i, nbr_count);

            for (ui j = 0; j < nbr_count; j++){
                for (ui k = j + 1; k < nbr_count; k++){
                    search_pair = get_wedge_endpoint_pair(neighbors[j], neighbors[k]);
                    auto search_result = (graph->wedge_map).find(search_pair);

                    if (search_result == (graph->wedge_map).end())
                    {
                        graph->wedge_map[search_pair] = 1;
                    }
                    else
                    {
                        graph->wedge_map[search_pair] = graph->wedge_map[search_pair] + 1;
                    }
                }
            }
        }

        ui buffer_size = (graph->wedge_map).size();
        std::vector<WedgeCnt> buffer(buffer_size);

        WedgeCnt wedge_cnt;
        for (const auto &[key, value] : graph->wedge_map)
        {
            wedge_cnt.first_vtx = key.first;
            wedge_cnt.third_vtx = key.second;
            wedge_cnt.cnt = value;
            buffer.push_back(wedge_cnt);
        }

        MPI_Send(&buffer_size, 1, MPI_INT, MPI_MASTER, TAG_BUFFER_SIZE, MPI_COMM_WORLD);
        MPI_Send(buffer.data(), buffer_size, MPI_TYPE_WEDGE_CNT, MPI_MASTER, TAG_BUFFER, MPI_COMM_WORLD);
    }

    MPI_Type_free(&MPI_TYPE_WEDGE_CNT);
    MPI_Finalize();
}

void CountingAlgorithm::dist_opt_count_square_in_partitioned_graph(const std::string &file_path, const std::string &vertex_partition_file_path){

    VertexID *neighbors;
    ui nbr_count, wedge_count;
    std::pair<VertexID, VertexID> search_pair;

    VertexID wedge_begin, wedge_end;
    long long total_sq_count = 0, recv_sq_count;

    int world_size, world_rank, dest_rank = 0;
    int buffer_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    MPI_Status buffer_size_status, buffer_status, local_cnt_status;

    MPI_Datatype MPI_TYPE_WEDGE_CNT;

    int lenAttr[WedgeCnt::szAttr] = {1, 1, 1};
    MPI_Datatype types[WedgeCnt::szAttr] = {MPI_UNSIGNED, MPI_UNSIGNED, MPI_UNSIGNED};

    MPI_Aint offsets[WedgeCnt::szAttr];

    offsets[0] = offsetof(WedgeCnt, first_vtx);
    offsets[1] = offsetof(WedgeCnt, third_vtx);
    offsets[2] = offsetof(WedgeCnt, cnt);

    MPI_Type_create_struct(WedgeCnt::szAttr, lenAttr, offsets, types, &MPI_TYPE_WEDGE_CNT);
    MPI_Type_commit(&MPI_TYPE_WEDGE_CNT);

    if (world_rank == 0)
    {

        std::map<std::pair<VertexID, VertexID>, ui> wedge_map;

        clock_t begin_time = clock_t();

        for (int rank = 1; rank < world_size; ++rank)
        {
            MPI_Recv(&buffer_size, 1, MPI_INT, rank, TAG_BUFFER_SIZE, MPI_COMM_WORLD, &buffer_size_status);

            std::vector<WedgeCnt> recv_buffer(buffer_size);
            MPI_Recv(recv_buffer.data(), world_size, MPI_TYPE_WEDGE_CNT, rank, TAG_BUFFER, MPI_COMM_WORLD, &buffer_status);

            ui i = 0;

            while (i < recv_buffer.size())
            {

                search_pair = std::make_pair(recv_buffer[i].first_vtx, recv_buffer[i].third_vtx);

                if ((wedge_map).find(search_pair) != wedge_map.end())
                {
                    wedge_map[search_pair] = wedge_map[search_pair] + recv_buffer[i].cnt;
                }
                else
                {
                    wedge_map[search_pair] = recv_buffer[i].cnt;
                }
            }

            MPI_Recv(&recv_sq_count, 1, MPI_LONG_LONG, rank, TAG_COUNT, MPI_COMM_WORLD, &local_cnt_status);
            total_sq_count += recv_sq_count;
        }

        for (const auto &[key, value] : wedge_map)
        {
            total_sq_count += (value * (value - 1) / 2);
        }

        total_sq_count = total_sq_count / 2;

        clock_t duration = (clock_t() - begin_time) / CLOCKS_PER_SEC;

        std::cout << " Coordinator Processor : " << world_rank << " Elapsed Time : " << duration << std::endl;

        std::cout << " Total Square Count : " << total_sq_count << std::endl;
    }
    else
    {
        Graph *graph = new Graph();
        NodeID ptn_idx;
        graph->loadPartitionedGraphFromFile(vertex_partition_file_path, file_path, world_rank - 1);

        clock_t begin_time = clock_t();

        for (ui i = 0; i < (graph->vertices).size(); i++)
        {
            neighbors = graph->getVertexNeighbors(i, nbr_count);

            for (ui j = 0; j < nbr_count; j++)
            {
                for (ui k = j + 1; k < nbr_count; k++)
                {

                    search_pair = get_wedge_endpoint_pair(neighbors[j], neighbors[k]);
                    ptn_idx = graph->partition[neighbors[j]];

                    if (ptn_idx == graph->partition[neighbors[k]] && ptn_idx == (world_rank - 1)){
                        auto search_result = (graph->wedge_map).find(search_pair);
                        if (search_result == (graph->wedge_map).end())
                        {
                            graph->wedge_map[search_pair] = 1;
                        }
                        else
                        {
                            graph->wedge_map[search_pair] = graph->wedge_map[search_pair] + 1;
                        }
                    } else {
                        auto search_result = (graph->wedge_map_comm).find(search_pair);
                        if (search_result == (graph->wedge_map_comm).end()){
                            graph->wedge_map_comm[search_pair] = 1;
                        } else {
                            graph->wedge_map_comm[search_pair] = graph->wedge_map_comm[search_pair] + 1;
                        }
                    }
                }
            }
        }

        for (ui i = 0; i < graph->ghost_vertices_count; i++)
        {
            for (ui j = graph->g_offsets[i]; j < graph->g_offsets[i + 1]; j++)
            {
                for (ui k = j + 1; k < graph->g_offsets[i + 1]; k++)
                {
                    search_pair = get_wedge_endpoint_pair(graph->g_neighbors[j], graph->g_neighbors[k]);

                    auto search_result = (graph->wedge_map).find(search_pair);
                    if (search_result == (graph->wedge_map).end()){
                        graph->wedge_map[search_pair] = 1;
                    } else {
                        graph->wedge_map[search_pair] = graph->wedge_map[search_pair] + 1;
                    }
                }
            }
        }

        long long local_sq_count = 0;

        for (const auto &[key, value] : graph->wedge_map)
        {
            local_sq_count += (value * (value - 1) / 2);
        }

        ui buffer_size = (graph->wedge_map).size();
        std::vector<WedgeCnt> buffer(buffer_size);

        WedgeCnt wedge_cnt;
        for (const auto &[key, value] : graph->wedge_map_comm)
        {
            wedge_cnt.first_vtx = key.first;
            wedge_cnt.third_vtx = key.second;
            wedge_cnt.cnt = value;
            buffer.push_back(wedge_cnt);
        }

        MPI_Send(&buffer_size, 1, MPI_INT, MPI_MASTER, TAG_BUFFER_SIZE, MPI_COMM_WORLD);
        MPI_Send(buffer.data(), buffer_size, MPI_TYPE_WEDGE_CNT, MPI_MASTER, TAG_BUFFER, MPI_COMM_WORLD);

        MPI_Send(&local_sq_count, 1, MPI_LONG_LONG, MPI_MASTER, TAG_COUNT, MPI_COMM_WORLD);

        clock_t duration = (clock_t() - begin_time) / CLOCKS_PER_SEC;

        std::cout << "Rank " << world_rank << " Elapsed Time : " << duration << std::endl;
    }

    MPI_Type_free(&MPI_TYPE_WEDGE_CNT);
    MPI_Finalize();
}

long long CountingAlgorithm::naive_comm_cost_analysis(Graph *graph, const std::string &vertex_partition_file_path, int partition_no)
{

    long long comm_cost = 0;

    std::ifstream vertex_partition_file(vertex_partition_file_path);

    VertexID vertex_id;
    NodeID partition_id;

    graph->partition = new NodeID[graph->vertices_count];

    while (vertex_partition_file >> vertex_id)
    {
        vertex_partition_file >> partition_id;
        graph->partition[vertex_id] = partition_id;
    }

    ui nbr_count;
    VertexID *neighbors;
    std::pair<VertexID, VertexID> search_pair;

    for (ui partition_idx = 0; partition_idx < partition_no; partition_idx++)
    {
        for (VertexID i = 0; i < (graph->vertices).size(); i++)
        {
            if (graph->partition[i] == partition_idx)
            {
                neighbors = graph->getVertexNeighbors(i, nbr_count);

                for (ui j = 0; j < nbr_count; j++)
                {
                    for (ui k = j + 1; k < nbr_count; k++)
                    {
                        search_pair = get_wedge_endpoint_pair(neighbors[j], neighbors[k]);

                        auto search = (graph->wedge_map).find(search_pair);
                        if (search == (graph->wedge_map).end())
                        {
                            graph->wedge_map[search_pair] = 1;
                        }
                        else
                        {
                            graph->wedge_map[search_pair] = graph->wedge_map[search_pair] + 1;
                        }
                    }
                }
            }
        }

        comm_cost += (graph->wedge_map).size();
        (graph->wedge_map).clear();
    }

    return comm_cost;
}

long long CountingAlgorithm::communication_cost_analysis(const std::string &vertex_partition_file_path, const std::string &file_path, int partition_no)
{

    long long comm_cost = 0;

    VertexID *neighbors;
    ui nbr_count;
    std::pair<VertexID, VertexID> search_pair;

    for (ui partition_idx = 0; partition_idx < partition_no; partition_idx++)
    {
        Graph *graph = new Graph();
        graph->loadPartitionedGraphFromFile(vertex_partition_file_path, file_path, partition_idx);

        for (ui i = 0; i < (graph->vertices).size(); i++)
        {
            neighbors = graph->getVertexNeighbors_partitioned(graph->vertices[i], nbr_count);

            for (ui j = 0; j < nbr_count; j++)
            {
                for (ui k = j + 1; k < nbr_count; k++)
                {
                    search_pair = get_wedge_endpoint_pair(neighbors[j], neighbors[k]);
                    if (graph->partition[neighbors[j]] != graph->partition[neighbors[k]])
                    {
                        auto search = (graph->wedge_map_comm).find(search_pair);
                        if (search == (graph->wedge_map_comm).end())
                        {
                            graph->wedge_map_comm[search_pair] = 1;
                        }
                        else
                        {
                            graph->wedge_map_comm[search_pair] = graph->wedge_map_comm[search_pair] + 1;
                        }
                    }
                }
            }
        }
        comm_cost += (graph->wedge_map_comm).size();
        (graph->wedge_map_comm).clear();
    }

    return comm_cost;
}

long long CountingAlgorithm::dist_comm_cost_analysis(PCSR *graph, std::vector<Edge> batched_edges, int rank)
{
    long long comm_cost = 0;

    VertexID *neighbors;
    ui nbr_count;
    std::pair<VertexID, VertexID> search_pair;

    for(ui i = 0; i < batched_edges.size(); i++){

        graph->vertex_ptn_map[batched_edges[i].src] = batched_edges[i].src_ptn;
        graph->vertex_ptn_map[batched_edges[i].dst] = batched_edges[i].dst_ptn;

        //std::cout << "In partition - " << batched_edges[i].src_ptn << std::endl;

        if(graph->vertex_ptn_map[batched_edges[i].src] == rank){

            graph->insert_edge(batched_edges[i].src, batched_edges[i].dst);

            std::vector<VertexID> nbrs = graph->edges(batched_edges[i].src);

            for(ui j = 0; j < nbrs.size(); j++){

                search_pair = get_wedge_endpoint_pair(batched_edges[i].dst, nbrs[j]);
                if (graph->vertex_ptn_map[batched_edges[i].dst] != graph->vertex_ptn_map[nbrs[j]])
                {
                    auto search = (graph->wedge_map_comm).find(search_pair);
                    if (search == (graph->wedge_map_comm).end())
                    {
                        graph->wedge_map_comm[search_pair] = 1;
                    }
                    else
                    {
                        graph->wedge_map_comm[search_pair] = graph->wedge_map_comm[search_pair] + 1;
                    }
                }

            }
        }

        if(graph->vertex_ptn_map[batched_edges[i].src] == rank){

            graph->insert_edge(batched_edges[i].dst, batched_edges[i].src);

            std::vector<VertexID> nbrs = graph->edges(batched_edges[i].dst);

            for(ui j = 0; j < nbrs.size(); j++){

                search_pair = get_wedge_endpoint_pair(batched_edges[i].src, nbrs[j]);
                if (graph->vertex_ptn_map[batched_edges[i].src] != graph->vertex_ptn_map[nbrs[j]])
                {
                    auto search = (graph->wedge_map_comm).find(search_pair);
                    if (search == (graph->wedge_map_comm).end())
                    {
                        graph->wedge_map_comm[search_pair] = 1;
                    }
                    else
                    {
                        graph->wedge_map_comm[search_pair] = graph->wedge_map_comm[search_pair] + 1;
                    }
                }
            }
        }       

    }

    batched_edges.clear();

    comm_cost += (graph->wedge_map_comm).size();
    (graph->wedge_map_comm).clear();

    return comm_cost;
}

void CountingAlgorithm::distributed_count_square(Graph *graph)
{

    VertexID *neighbors;
    ui nbr_count, wedge_count;
    std::pair<VertexID, VertexID> search_pair;

    VertexID wedge_begin, wedge_end;
    std::vector<VertexID> buffer;

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

            while (i < recv_buffer.size())
            {
                wedge_begin = recv_buffer[i];
                wedge_end = recv_buffer[i + 1];
                wedge_count = recv_buffer[i + 2];

                search_pair = std::make_pair(wedge_begin, wedge_end);

                if ((graph->wedge_map).find(search_pair) != (graph->wedge_map).end())
                {
                    graph->wedge_map[search_pair] = graph->wedge_map[search_pair] + wedge_count;
                }
                else
                {
                    graph->wedge_map[search_pair] = wedge_count;
                }

                i = i + 3;
            }
        }
    }
    else
    {

        for (ui i = 0; i < graph->getVerticesCount(); i++)
        {
            neighbors = graph->getVertexNeighbors(i, nbr_count);

            for (ui j = 0; j < nbr_count; j++)
            {
                for (ui k = j + 1; k < nbr_count; k++)
                {

                    search_pair = get_wedge_endpoint_pair(neighbors[j], neighbors[k]);
                    auto search = (graph->wedge_map).find(search_pair);

                    if (search == (graph->wedge_map).end())
                    {
                        graph->wedge_map[search_pair] = 1;
                    }
                    else
                    {
                        graph->wedge_map[search_pair] = graph->wedge_map[search_pair] + 1;
                    }
                }
            }
        }

        for (const auto &[key, value] : graph->wedge_map)
        {
            buffer.push_back(key.first);
            buffer.push_back(key.second);
            buffer.push_back(value);
        }

        int buffer_size = buffer.size();
        MPI_Send(&buffer_size, 1, MPI_INT, dest_rank, TAG_BUFFER_SIZE, MPI_COMM_WORLD);

        MPI_Send(buffer.data(), buffer_size, MPI_INT, dest_rank, TAG_BUFFER, MPI_COMM_WORLD);
    }
}

void CountingAlgorithm::distributed_dynamic_count_square(PCSR *graph)
{

    std::vector<VertexID> neighbors;
    ui nbr_count;
    std::pair<VertexID, VertexID> search_pair;

    for (ui i = 0; i < graph->get_vertices_count(); i++)
    {
        neighbors = graph->edges(i);
        nbr_count = neighbors.size();

        for (ui j = 0; j < nbr_count; j++)
        {
            for (ui k = j + 1; k < nbr_count; k++)
            {
                search_pair = get_wedge_endpoint_pair(neighbors[j], neighbors[k]);
                auto search = (graph->wedge_map).find(search_pair);

                if (search == (graph->wedge_map).end())
                {
                    graph->wedge_map[search_pair] = 1;
                }
                else
                {
                    graph->wedge_map[search_pair] = graph->wedge_map[search_pair] + 1;
                }
            }
        }
    }
}

long long CountingAlgorithm::aggregate_square_count(std::map<std::pair<VertexID, VertexID>, ui> &wedge_map, long long &global_cnt)
{

    for (const auto &[key, value] : wedge_map)
    {
        global_cnt += combinations(value, 2);
    }

    return global_cnt / 2;
}
