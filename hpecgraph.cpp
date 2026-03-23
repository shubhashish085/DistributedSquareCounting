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

}
