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
#include <string>

#include "matchingcommand.h"
#include "pcsr.h"
#include "types.h"
#include "countingalgo.h"
#include "distributedrun.hpp"

/*int main(int argc, char** argv){

    std::string input_data_graph_file = argv[1];
    std::string vertex_partition_file = argv[2];
    std::string partition_no = argv[3];

    int n_partition = std::stoi(partition_no);

    Graph* graph = new Graph();
    graph->loadGraphFromFile(input_data_graph_file);

    long long comm_cost = CountingAlgorithm::naive_comm_cost_analysis(graph, vertex_partition_file, n_partition);

    std::cout << "Input Graph File : " << input_data_graph_file << std::endl;
    std::cout << "Vertex Partition File : " << vertex_partition_file << std::endl;
    std::cout << "Partition No : " << n_partition << std::endl;
    std::cout << "Naive Communication Cost : " << comm_cost << std::endl;

}*/


/*int main(int argc, char** argv){

    std::string input_data_graph_file = argv[1];
    std::string output_data_graph_file = "output.txt";
    std::string partition_no = argv[2];

    int n_partition = std::stoi(partition_no);
    DistributionCoordinator hIO(argc, argv);
    //hIO.init(BATCH_LENGTH, n_partition);

    std::cout << "Input Graph File : " << input_data_graph_file << std::endl;
    std::cout << "Partition No : " << n_partition << std::endl;

    run_exp_dynamic_network(input_data_graph_file, output_data_graph_file, hIO, n_partition, BATCH_LENGTH);
    
    //std::cout << "Naive Communication Cost : " << comm_cost << std::endl;
}*/

/*int main(int argc, char** argv){

    long long wedge_count = 0;
    long long square_count = 0;

    std::string input_data_graph_file = argv[1];
    std::cout << "Input Graph File : " << input_data_graph_file << std::endl;

    Graph* graph = new Graph();
    graph->loadGraphFromFile(input_data_graph_file);

    for(ui i = 0; i < graph->getVerticesCount(); i++){
        wedge_count += (graph->degrees[i] * (graph->degrees[i] - 1)) / 2;
    }

    std::cout << "Total Wedge Count : " << wedge_count << std::endl;

    square_count = graph->count_exact_square();

    std::cout << "Total Square Count : " << square_count << std::endl;

    std::cout << "==================================================" << std::endl;

}*/




int main(int argc, char** argv){

    std::string input_data_graph_file = argv[1];
    std::string vertex_partition_file = argv[2];
    std::cout << "Input Graph File : " << input_data_graph_file << std::endl;
    std::cout << "Vertex Partition File : " << vertex_partition_file << std::endl;

    CountingAlgorithm::dist_opt_count_square_in_partitioned_graph(input_data_graph_file, vertex_partition_file);   

    std::cout << "==================================================" << std::endl;

}



/*int main(int argc, char** argv) {
    
    
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
    

    /*PCSR pcsr(1 << 4);

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
}*/
