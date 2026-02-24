#include <chrono>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <list>
#include <numeric>
#include <set>
#include <vector>
#include <fstream> 
#include <sstream>
#include <iostream>

#include "matchingcommand.h"
#include "pcsr.h"
#include "types.h"


int main(int argc, char** argv) {
    
    
    MatchingCommand command(argc, argv);
    
    std::string input_data_graph_file = command.getDataGraphFilePath();
    std::string output_file = command.getOutputFilePath();

    std::ifstream infile;
    std::ofstream outputfile;
    outputfile.open(output_file, std::ios::app);

    if (!infile.is_open()) {
        std::cout << "Can not open the graph file " << input_data_graph_file << " ." << std::endl;
        exit(-1);
    }

    std::string input_line, addition;

    ui line_count = 0, comment_line_count = 4;
    

    PCSR pcsr(1 << 4);

    while (std::getline(infile, input_line)) {

        line_count++;

        if(line_count >= comment_line_count){
            break;
        }
    }

    VertexID from, to;

    while(infile >> from) {

        infile >> to;
        infile >> addition;

        if(addition == "-"){
            pcsr.delete_edge(from, to);
        }else{
            pcsr.insert_edge(from, to);
        }

    }

    infile.close();

    return 0;
}
