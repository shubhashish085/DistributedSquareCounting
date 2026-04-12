

#include "graph.h"
#include <fstream>
#include <cmath>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <sstream>

void Graph::loadGraphFromFile(const std::string &file_path)
{

    std::cout << "############# Loading Graph With Edges ###############" << std::endl;

    std::ifstream infile(file_path);

    if (!infile.is_open())
    {
        std::cout << "Can not open the graph file " << file_path << " ." << std::endl;
        exit(-1);
    }

    char type;
    std::string input_line;
    ui label = 0;

    std::cout << "Reading File............ " << std::endl;

    ui line_count = 0, count = 0, comment_line_count = 4;

    while (std::getline(infile, input_line))
    {

        if (input_line.rfind("#", 0) == 0)
        {

            line_count++;

            if (input_line.rfind("# Nodes", 0) == 0)
            {

                std::stringstream ss(input_line);
                std::string token;
                int count = 0;

                while (!ss.eof())
                {

                    std::getline(ss, token, ' ');

                    if (!(token.rfind("#", 0) == 0 || token.rfind("Nodes:", 0) == 0 || token.rfind("Edges:", 0) == 0))
                    {

                        if (count == 0)
                        {

                            vertices_count = stoi(token);
                            std::cout << "Vertex Count : " << vertices_count << std::endl;
                            degrees = new ui[vertices_count];
                            std::fill(degrees, degrees + vertices_count, 0);
                            count = 1;
                        }
                        else
                        {
                            edges_count = stoi(token);
                            count = 0;
                        }
                        std::cout << "Vertices Count : " << vertices_count << " Edges Count : " << edges_count
                                  << std::endl;
                    }
                }
            }
        }

        if (line_count >= comment_line_count)
        {
            break;
        }
    }

    VertexID begin, end;

    while (infile >> begin)
    {

        infile >> end;

        if (begin != end && begin < vertices_count && end < vertices_count)
        {
            degrees[begin] += 1;
            degrees[end] += 1;
        }
    }

    infile.close();

    std::ifstream input_file(file_path);

    offsets = new ui[vertices_count + 1];
    offsets[0] = 0;

    neighbors = new VertexID[edges_count * 2];
    max_degree = 0;

    std::cout << "Initialization Finished" << std::endl;

    LabelID max_label_id = 0, begin_vtx_label, end_vtx_label;
    std::vector<ui> neighbors_offset(vertices_count, 0); // used for adjust neighbors with offset

    for (ui id = 0; id < vertices_count; id++)
    {
        offsets[id + 1] = offsets[id] + degrees[id];

        if (degrees[id] > max_degree)
        {
            max_degree = degrees[id];
        }
    }

    line_count = 0;

    while (std::getline(input_file, input_line))
    {
        line_count++;
        if (line_count >= comment_line_count)
        {
            break;
        }
    }

    while (input_file >> begin)
    {

        input_file >> end;

        line_count++;
        if (begin >= vertices_count || end >= vertices_count || begin == end)
        {
            continue;
        }

        ui offset = offsets[begin] + neighbors_offset[begin];
        neighbors[offset] = end;

        offset = offsets[end] + neighbors_offset[end];
        neighbors[offset] = begin;

        neighbors_offset[begin] += 1;
        neighbors_offset[end] += 1;
    }

    input_file.close();

    for (ui i = 0; i < vertices_count; ++i)
    {
        std::sort(neighbors + offsets[i], neighbors + offsets[i + 1]);
    }
}

void Graph::loadDBPartitionedGraphFromFile(const std::string &file_path, VertexID minVertexID, VertexID maxVertexID)
{

    std::ifstream infile(file_path);

    if (!infile.is_open())
    {
        std::cout << "Can not open the graph file " << file_path << " ." << std::endl;
        exit(-1);
    }

    char type;
    std::string input_line;
    ui label = 0;

    std::cout << "Reading File............ " << std::endl;

    ui line_count = 0, count = 0, comment_line_count = 4;

    vertices_count = maxVertexID - minVertexID + 1;
    edges_count = 0;
    degrees = new ui[vertices_count];
    std::fill(degrees, degrees + vertices_count, 0);

    while (std::getline(infile, input_line))
    {
        line_count++;
        if (line_count >= comment_line_count)
        {
            break;
        }
    }

    VertexID begin, end;

    while (infile >> begin)
    {

        infile >> end;

        if (begin >= minVertexID || end <= maxVertexID)
        {

            if (begin != end && begin >= minVertexID && end <= maxVertexID)
            {
                begin -= minVertexID;
                end -= minVertexID;
                degrees[begin] += 1;
                degrees[end] += 1;
                edges_count += 2;
            }
            else if (begin != end && begin >= minVertexID)
            {
                begin -= minVertexID;
                degrees[begin] += 1;
                edges_count += 1;
            }
            else if (begin != end && end <= maxVertexID)
            {
                end -= minVertexID;
                degrees[end] += 1;
                edges_count += 1;
            }
        }
    }

    infile.close();

    std::ifstream input_file(file_path);

    offsets = new ui[vertices_count + 1];
    offsets[0] = 0;

    neighbors = new VertexID[edges_count];
    max_degree = 0;

    LabelID max_label_id = 0, begin_vtx_label, end_vtx_label;
    std::vector<ui> neighbors_offset(vertices_count, 0); // used for adjust neighbors with offset

    for (ui id = 0; id < vertices_count; id++)
    {
        offsets[id + 1] = offsets[id] + degrees[id];

        if (degrees[id] > max_degree)
        {
            max_degree = degrees[id];
        }
    }

    line_count = 0;

    while (std::getline(input_file, input_line))
    {
        line_count++;
        if (line_count >= comment_line_count)
        {
            break;
        }
    }

    ui offset;

    while (input_file >> begin)
    {

        input_file >> end;

        line_count++;
        if ((begin > maxVertexID && end > maxVertexID) || (begin < minVertexID && end < minVertexID) || begin == end)
        {
            continue;
        }

        if (begin >= minVertexID && end <= maxVertexID)
        {

            begin -= minVertexID;
            end -= minVertexID;

            offset = offsets[begin] + neighbors_offset[begin];
            neighbors[offset] = end;

            offset = offsets[end] + neighbors_offset[end];
            neighbors[offset] = begin;

            neighbors_offset[begin] += 1;
            neighbors_offset[end] += 1;
        }
        else if (begin >= minVertexID)
        {

            begin -= minVertexID;

            offset = offsets[begin] + neighbors_offset[begin];
            neighbors[offset] = end;
            neighbors_offset[begin] += 1;
        }
        else if (end <= maxVertexID)
        {

            end -= minVertexID;

            offset = offsets[end] + neighbors_offset[end];
            neighbors[offset] = begin;

            neighbors_offset[end] += 1;
        }
    }

    input_file.close();

    for (ui i = 0; i < vertices_count; ++i)
    {
        std::sort(neighbors + offsets[i], neighbors + offsets[i + 1]);
    }
}

void transformToAugmentedGraph(Graph *data_graph, Graph *augmented_graph)
{

    augmented_graph->vertices_count = data_graph->vertices_count;
    augmented_graph->edges_count = data_graph->edges_count;

    augmented_graph->degrees = new ui[augmented_graph->vertices_count];
    std::fill(augmented_graph->degrees, augmented_graph->degrees + augmented_graph->vertices_count, 0);
}

void Graph::loadPartitionedGraphFromFile(const std::string &vertex_partition_file_path, const std::string &file_path, int partition_no)
{

    VertexID vertex_id, begin, end;
    int partition_id;
    bool contains_in_partition = false;

    std::ifstream vertex_partition_file(vertex_partition_file_path);
    std::ifstream vtx_ptn_file(vertex_partition_file_path);
    std::ifstream infile(file_path);

    if (!vertex_partition_file.is_open() || !infile.is_open())
    {
        std::cout << "Can not open the graph file " << vertex_partition_file_path << " or " << file_path << "." << std::endl;
        exit(-1);
    }

    ui total_vertices_count = 0;

    while (vertex_partition_file >> vertex_id)
    {

        vertex_partition_file >> partition_id;

        if (partition_id == partition_no)
        {
            vertices.push_back(vertex_id);
            vertex_idx_map[vertex_id] = vertices_count;
            vertices_count++;
        }

        total_vertices_count++;
    }

    vertex_partition_file.close();

    partition = new NodeID[total_vertices_count];

    while (vtx_ptn_file >> vertex_id)
    {
        vtx_ptn_file >> partition_id;
        partition[vertex_id] = partition_id;
    }

    vtx_ptn_file.close();

    degrees = new ui[vertices_count];
    ghost_vertices_count = 0;
    std::fill(degrees, degrees + vertices_count, 0);

    char type;
    std::string input_line;
    ui label = 0, offset;

    std::cout << "Reading File............ " << std::endl;

    ui line_count = 0, count = 0, comment_line_count = 4;

    while (std::getline(infile, input_line))
    {

        if (input_line.rfind("#", 0) == 0)
        {
            line_count++;
        }

        if (line_count >= comment_line_count)
        {
            break;
        }
    }

    ui ghost_edges_count = 0;

    while (infile >> begin)
    {

        infile >> end;

        if (begin != end && begin < total_vertices_count && end < total_vertices_count)
        {
            if (partition[begin] != partition_no && partition[end] == partition_no){

                ghost_edges_count++;

                if (ghost_vertex_map.find(begin) != ghost_vertex_map.end()) {
                    ghost_vertex_map[begin] += 1;
                } else {
                    ghost_vertex_map[begin] = 1;
                    ghost_vertices_count += 1;
                }
            } else if (partition[begin] == partition_no && partition[end] != partition_no){

                ghost_edges_count++;

                if (ghost_vertex_map.find(end) != ghost_vertex_map.end()) {
                    ghost_vertex_map[end] += 1;
                }
                else{
                    ghost_vertex_map[end] = 1;
                    ghost_vertices_count += 1;
                }
            }

            if (vertex_idx_map.find(begin) != vertex_idx_map.end())
            {
                degrees[vertex_idx_map[begin]] += 1;
                contains_in_partition = true;
            }

            if (vertex_idx_map.find(end) != vertex_idx_map.end())
            {
                degrees[vertex_idx_map[end]] += 1;
                contains_in_partition = true;
            }

            if (contains_in_partition)
            {
                edges_count++;
            }

            contains_in_partition = false;
        }
    }

    infile.close();

    std::ifstream input_file(file_path);

    offsets = new ui[vertices_count + 1];
    g_offsets = new ui[ghost_vertices_count + 1];
    offsets[0] = 0;
    g_offsets[0] = 0;

    neighbors = new VertexID[edges_count * 2];
    g_neighbors = new VertexID[ghost_edges_count];
    max_degree = 0;

    std::vector<ui> neighbors_offset(vertices_count, 0); // used for adjust neighbors with offset
    std::vector<ui> g_neighbors_offset(ghost_vertices_count, 0);
    VertexID id;

    for (VertexID i = 0; i < vertices_count; i++){
        /*id = vertices[i];
        offsets[id + 1] = offsets[id] + degrees[id];*/
        offsets[i + 1] = offsets[i] + degrees[i];
    }

    VertexID i = 0;
    for (const auto &[key, value] : ghost_vertex_map)
    {
        ghost_vertex_idx_map[key] = i;
        g_offsets[i + 1] = g_offsets[i] + value;
        i++;
    }

    line_count = 0;

    while (std::getline(input_file, input_line))
    {
        line_count++;
        if (line_count >= comment_line_count)
        {
            break;
        }
    }

    VertexID ghost_idx;
    ui g_offset;
    ui begin_idx, end_idx;

    while (input_file >> begin)
    {
        input_file >> end;

        if((begin == end) || (begin >= total_vertices_count) || (end >= total_vertices_count)){
            continue;
        }


        if (vertex_idx_map.find(begin) != vertex_idx_map.end()){
            begin_idx = vertex_idx_map[begin];
            offset = offsets[begin_idx] + neighbors_offset[begin_idx];
            neighbors[offset] = end;
            neighbors_offset[begin] += 1;
        }

        if (vertex_idx_map.find(end) != vertex_idx_map.end()){
            end_idx = vertex_idx_map[end];
            offset = offsets[end_idx] + neighbors_offset[end_idx];
            neighbors[offset] = begin;
            neighbors_offset[end] += 1;
        }

        if (partition[begin] != partition_no && partition[end] == partition_no){
            ghost_idx = ghost_vertex_idx_map[begin];
            g_offset = g_offsets[ghost_idx] + g_neighbors_offset[ghost_idx];
            g_neighbors[g_offset] = end;
            g_neighbors_offset[ghost_idx] += 1;
        }

        if (partition[begin] == partition_no && partition[end] != partition_no){
            ghost_idx = ghost_vertex_idx_map[end];
            g_offset = g_offsets[ghost_idx] + g_neighbors_offset[ghost_idx];
            g_neighbors[g_offset] = begin;
            g_neighbors_offset[ghost_idx] += 1;
        }
    }

    input_file.close();

    for (ui i = 0; i < vertices_count; ++i)
    {
        std::sort(neighbors + offsets[i], neighbors + offsets[i + 1]);
    }

    for (ui i = 0; i < ghost_vertices_count; i++)
    {
        std::sort(g_neighbors + g_offsets[i], g_neighbors + g_offsets[i + 1]);
    }
}

void Graph::printGraphMetaData()
{
    std::cout << "|V|: " << vertices_count << ", |E|: " << edges_count << std::endl;
    std::cout << "Max Degree: " << max_degree << ", Max Label Frequency: " << std::endl;
}

long long Graph::count_exact_square()
{
    long long exact_count = 0;

    std::set<VertexID> intersection_set;

    for (VertexID u = 0; u < vertices_count; u++)
    {
        ui u_nbr_cnt;
        VertexID *u_nbrs = getVertexNeighbors(u, u_nbr_cnt);

        for (VertexID i = 0; i < u_nbr_cnt; i++)
        {
            VertexID v = u_nbrs[i];
            if (u < v)
            {
                ui v_nbr_cnt;
                VertexID *v_nbrs = getVertexNeighbors(v, v_nbr_cnt);
                std::set<VertexID> v_nbr_set(v_nbrs, v_nbrs + v_nbr_cnt);

                for (VertexID j = 0; j < u_nbr_cnt; j++)
                {
                    VertexID x = u_nbrs[j];
                    int x_valid_idx = 0;
                    if (v < x)
                    {
                        ui x_nbr_cnt;
                        VertexID *x_nbrs = getVertexNeighbors(x, x_nbr_cnt);
                        for (VertexID k = 0; k < x_nbr_cnt; k++)
                        {
                            if (x_nbrs[k] < u)
                            {
                                x_valid_idx++;
                            }
                            else
                            {
                                break;
                            }
                        }

                        if (x_valid_idx == 0)
                        {
                            break;
                        }

                        std::set<VertexID> x_nbr_set(x_nbrs, x_nbrs + x_valid_idx);

                        intersection_set.clear();

                        std::set_intersection(v_nbr_set.begin(), v_nbr_set.end(), x_nbr_set.begin(), x_nbr_set.end(),
                                              std::inserter(intersection_set, intersection_set.begin()));

                        exact_count += intersection_set.size();
                    }
                }
            }
        }
    }

    return exact_count;
}

long long Graph::count_exact_square_parallel()
{

    long long exact_count = 0;

#pragma omp parallel reduction(+ : exact_count)
    {
        std::set<VertexID> intersection_set;

#pragma omp for schedule(dynamic)
        for (VertexID u = 0; u < vertices_count; u++)
        {

            ui u_nbr_cnt;
            VertexID *u_nbrs = getVertexNeighbors(u, u_nbr_cnt);

            for (VertexID i = 0; i < u_nbr_cnt; i++)
            {
                VertexID v = u_nbrs[i];
                if (u < v)
                {
                    ui v_nbr_cnt;
                    VertexID *v_nbrs = getVertexNeighbors(v, v_nbr_cnt);
                    std::set<VertexID> v_nbr_set(v_nbrs, v_nbrs + v_nbr_cnt);

                    for (VertexID j = 0; j < u_nbr_cnt; j++)
                    {
                        VertexID x = u_nbrs[j];
                        int x_valid_idx = 0;
                        if (v < x)
                        {
                            ui x_nbr_cnt;
                            VertexID *x_nbrs = getVertexNeighbors(x, x_nbr_cnt);
                            for (VertexID k = 0; k < x_nbr_cnt; k++)
                            {
                                if (x_nbrs[k] < u)
                                {
                                    x_valid_idx++;
                                }
                                else
                                {
                                    break;
                                }
                            }

                            if (x_valid_idx == 0)
                            {
                                break;
                            }

                            std::set<VertexID> x_nbr_set(x_nbrs, x_nbrs + x_valid_idx);

                            intersection_set.clear();

                            std::set_intersection(v_nbr_set.begin(), v_nbr_set.end(), x_nbr_set.begin(), x_nbr_set.end(),
                                                  std::inserter(intersection_set, intersection_set.begin()));

                            exact_count += intersection_set.size();
                        }
                    }
                }
            }
        }
    }

    return exact_count;
}
