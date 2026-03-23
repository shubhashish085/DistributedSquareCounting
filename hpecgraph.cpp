#include "hpecgraph.hpp"
#include <fstream>
#include <cmath>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>

    
void HpecGraph::loadGraphFromFile(const std::string& file_path){

    std::cout << "############# Loading Graph With Edges ###############" << std::endl;

    std::ifstream infile(file_path);

    if (!infile.is_open()) {
        std::cout << "Can not open the graph file " << file_path << " ." << std::endl;
        exit(-1);
    }

    char type;
    std::string input_line;
    ui label = 0;

    std::cout << "Reading File............ " << std::endl;

    ui line_count = 0, count = 0, comment_line_count = 4;

    while (std::getline(infile, input_line)) {

        if (input_line.rfind("#", 0) == 0) {

            line_count++;

            if (input_line.rfind("# Nodes", 0) == 0) {
                
                std::stringstream ss(input_line);
                std::string token;
                int count = 0;
                
                while (!ss.eof()) {
                    
                    std::getline(ss, token, ' ');
                    
                    if (!(token.rfind("#", 0) == 0 || token.rfind("Nodes:", 0) == 0 || token.rfind("Edges:", 0) == 0)) {
                        
                        if (count == 0) {
                            
                            vertices_count = stoi(token);
                            std::cout << "Vertex Count : " << vertices_count << std::endl;
                            degrees = new ui[vertices_count];
                            std::fill(degrees, degrees + vertices_count, 0);
                            count = 1;
                        } else {
                            edges_count = stoi(token);
                            count = 0;
                        }
                        std::cout << "Vertices Count : " << vertices_count << " Edges Count : " << edges_count
                                  << std::endl;
                    }
                }
            }
        }

        if(line_count >= comment_line_count){
            break;
        }
    }

    vertex_partition_map = new NodeID[vertices_count];
    std::fill(vertex_partition_map, vertex_partition_map + vertices_count, -1);

    VertexID begin, end;


    while(infile >> begin) {

        infile >> end;

        if (begin != end && begin < vertices_count && end < vertices_count) {
            degrees[begin] += 1;
            degrees[end] += 1;
        }
    }

    infile.close();

    std::ifstream input_file(file_path);

    offsets = new ui[vertices_count +  1];
    offsets[0] = 0;

    neighbors = new VertexID[edges_count * 2];
    max_degree = 0;
    
    std::vector<ui> neighbors_offset(vertices_count, 0);// used for adjust neighbors with offset

    for(ui id = 0; id < vertices_count; id++){
        offsets[id + 1] = offsets[id] + degrees[id];

        if (degrees[id] > max_degree) {
            max_degree = degrees[id];
        }
    }

    line_count = 0;

    while (std::getline(input_file, input_line)) {
        line_count++;
        if(line_count >= comment_line_count){
            break;
        }
    }

    while(input_file >> begin){

        input_file >> end;

        line_count++;
        if(begin >= vertices_count || end >= vertices_count || begin == end){
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
    

    for (ui i = 0; i < vertices_count; ++i) {
        std::sort(neighbors + offsets[i], neighbors + offsets[i + 1]);
    } 

}

void HpecGraph::transformToDirectedGraph(){

    ui offset;

    directed_degrees = new ui[vertices_count];
    directed_offsets = new ui[vertices_count + 1];
    directed_nbrs = new VertexID[edges_count];
    directed_nbrs_offset = new ui[vertices_count];

    std::fill(directed_nbrs_offset, directed_nbrs_offset + vertices_count, 0);

    for(VertexID i = 0; i < vertices_count; i++){
        for(ui j = offsets[i]; j < offsets[i + 1]; j++){
            if(!is_greater_in_order(i, neighbors[j])){
                directed_degrees[i]++;
            }
        }
    }

    directed_offsets[0] = 0;
    for(ui i = 1; i < vertices_count + 1; i++){
        directed_offsets[i] = directed_offsets[i - 1] + directed_degrees[i - 1];
    }

    for(VertexID i = 0; i < vertices_count; i++){
        for(ui j = offsets[i]; j < offsets[i + 1]; j++){
            if(!is_greater_in_order(i, neighbors[j])){
                offset = directed_offsets[i] + directed_nbrs_offset[i];
                directed_nbrs[offset] = neighbors[j];
                directed_nbrs_offset[i]++;
            }
        }
    }
}
