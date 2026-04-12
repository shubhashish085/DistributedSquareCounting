#include <fstream>
#include "partition.hpp"
#include "masterdynamicgraph.hpp"
#include "countingalgo.h"
#include "distributedrun.hpp"
#include "distributioncoordinator.hpp"

double run_mpi_for_static_network(const char *filename, DistributionCoordinator &hIO, int workerNum, int lenBuf, double &srcCompCost, double &workerCompCostMax, double &workerCompCostSum)
{
    clock_t begin = clock();

    hIO.init(lenBuf, workerNum);

    ui *partition_limit = new ui[workerNum];
    ui n_partition = workerNum;

    int rank = hIO.getRank();
    Graph *master_graph = new Graph();

    if (hIO.isMaster())
    {
        master_graph->loadGraphFromFile(filename);
        GraphPartitioning::even_degree_partition(master_graph, n_partition, partition_limit);
    }

    MPI_Bcast(partition_limit, workerNum, MPI_INT, MPI_MASTER, MPI_COMM_WORLD);

    if (hIO.isMaster())
    {

        long long globalCnt = 0;

        for (ui i = 0; i < workerNum; i++)
        {
            hIO.recvWedgeCnt(master_graph->wedge_map);
        }

        CountingAlgorithm::aggregate_square_count(master_graph->wedge_map, globalCnt);

        hIO.recvTime(workerCompCostMax, workerCompCostSum);

        srcCompCost = (double(clock() - begin) - hIO.getIOCPUTime()) / CLOCKS_PER_SEC;

        return globalCnt;
    }
    else
    {

        Graph *worker_graph = new Graph();
        VertexID minVertexID, maxVertexID;

        if (rank == 1)
        {
            minVertexID = 0;
            maxVertexID = partition_limit[rank - 1];
        }
        else
        {
            minVertexID = partition_limit[rank - 2];
            maxVertexID = partition_limit[rank - 1];
        }

        worker_graph->loadDBPartitionedGraphFromFile(filename, minVertexID, maxVertexID);
        CountingAlgorithm::count_square(worker_graph);

        hIO.sendWedgeCnt(worker_graph->wedge_map);

        double workerCompCost = (double(clock() - begin) - hIO.getIOCPUTime()) / CLOCKS_PER_SEC;

        hIO.sendTime(workerCompCost);
        return 0;
    }
}

void run_mpi_for_dynamic_network(std::string filename, DistributionCoordinator &hIO, int workerNum, int lenBuf, double &srcCompCost, double &workerCompCostMax, double &workerCompCostSum)
{

    clock_t begin = clock();

    hIO.init(lenBuf, workerNum);

    MasterGraph *master_graph;

    // Sourcce init
    if (hIO.isMaster())
    {
        master_graph = new MasterGraph(workerNum, GRAPH_CAPACITY, GRAPH_CAPACITY);

        Edge edge;
        std::ifstream infile(filename);

        std::cout << "In Master - Rank : " << hIO.getRank() << std::endl;

        if (!infile.is_open())
        {
            std::cout << "Can not open the graph file " << filename << " ." << std::endl;
            exit(-1);
        }

        VertexID begin, end;
        std::string addition;
        NodeID u_partition, v_partition;


        ui line_count = 0, comment_line_count = 4;
        std::string input_line;


        while (std::getline(infile, input_line)) {
            line_count++;
            if(line_count >= comment_line_count){
                break;
            }
        }


        while (infile >> begin) // Stream edges
        {
            infile >> end;
            //infile >> addition;

            if(begin > (GRAPH_CAPACITY - 2) || end >  (GRAPH_CAPACITY - 2) || (begin == end)){
                continue;
            }

            edge.src = begin;
            edge.dst = end;
            edge.add = true;

            /*if (addition == "-")
            {
                edge.add = false;
            }
            else
            {
                edge.add = true;
            }*/

            //std::cout << "Partitioning Started" << hIO.getRank() << std::endl;
            GraphPartitioning::hash_dyn_partition(master_graph, edge, u_partition, v_partition);
            //GraphPartitioning::ldg_dyn_partition(master_graph, edge, u_partition, v_partition);
            // GraphPartitioning::fennel_dyn_partition(master_graph, edge, u_partition, v_partition);


            //std::cout << "U partition : " << edge.src_ptn << " V partition : " << edge.dst_ptn << std::endl;

            if (u_partition == v_partition)
            {
                hIO.sendEdge(edge, u_partition);
            }
            else
            {
                hIO.sendEdge(edge, u_partition);
                hIO.sendEdge(edge, v_partition);
            }
        }

        hIO.sendEndSignal_blocking();

        long long globalCnt = 0;

        /*for (ui i = 0; i < workerNum; i++)
        {
            hIO.recvWedgeCnt(master_graph->wedge_map);
        }

        CountingAlgorithm::aggregate_square_count(master_graph->wedge_map, globalCnt);

        hIO.recvTime(workerCompCostMax, workerCompCostSum);

        srcCompCost = (double(clock() - begin) - hIO.getIOCPUTime()) / CLOCKS_PER_SEC;*/
    }
    else
    {
        PCSR *graph = new PCSR(PCSR_INITIAL_SIZE);
        Edge edge;
        std::vector<Edge> batched_edges;

        ui edge_counter = 0;
        long long comm_cost = 0, prev_wedge_map_size = 0;

        while (true)
        {
            /*for(ui idx = 0; idx < hIO.eBuf[0].bit; idx++){
                batched_edges.push_back(hIO.eBuf[0].buf[]);
            }*/
            bool last = false;
            ui length = 0;

            hIO.recvEdge_blocking(edge,batched_edges, length);
            

            /*for(ui i = 0; i < lenBuf; i++){
                if(batched_edges[i].src == INVALID_VID && batched_edges[i].dst == INVALID_VID){
                   last = true; 
                }
            }*/

            //std::cout << "Edge : " << batched_edges[0].src << " ------ "<< batched_edges[0].dst << std::endl; 

            /*if(batched_edges[length - 1].src == INVALID_VID || batched_edges[length - 1].dst == INVALID_VID){
                //batched_edges.pop_back();
                //comm_cost += CountingAlgorithm::dist_comm_cost_analysis(graph, batched_edges, hIO.getRank());
                break;
            }*/

            edge_counter += batched_edges.size();


            comm_cost += CountingAlgorithm::dist_comm_cost_analysis(graph, batched_edges, hIO.getRank());
            //std::cout << "Communication Cost : " << comm_cost << std::endl;
            

            //batched_edges.clear();             
            // if (edge.add)
            // {
            //     batched_edges.push_back(edge);
            // }
            /*else
            {
                graph->delete_edge(edge.src, edge.dst);
            }*/

           if(length < lenBuf){

                std::cout << "Last Set Done ! Edge Count From Partition " << hIO.getRank() << " : " << edge_counter << std::endl;
                break;
           }

           length = 0;

        }

        std::cout << "Partition : " << hIO.getRank() << " : Communication Cost - " << comm_cost << std::endl;

        std::cout << "======================================================================================" << std::endl;

        // send counts to master
        /*CountingAlgorithm::distributed_dynamic_count_square(graph);

        hIO.sendWedgeCnt(graph->wedge_map);

        double workerCompCost = (double(clock() - begin) - hIO.getIOCPUTime()) / CLOCKS_PER_SEC;

        hIO.sendTime(workerCompCost);*/
        
    }

    //MPI_Type_free(&MPI_TYPE_EDGE);
    MPI_Finalize();
}

void run_exp(const char *input, const char *outPath, DistributionCoordinator &hIO, int workerNum, int bufLen)
{

    int seed = 0;

    struct timeval diff, startTV, endTV;

    if (hIO.isMaster())
    {
        struct stat sb;
        if (stat(outPath, &sb) == 0)
        {
            if (S_ISDIR(sb.st_mode)) // TODO. directory is exists
                ;
            else if (S_ISREG(sb.st_mode)) // TODO. No directory but a regular file with same name
                ;
            else // TODO. handle undefined cases.
                ;
        }
        else
        {
            mkdir(outPath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        }
    }

    if (hIO.isMaster())
    {

        gettimeofday(&startTV, NULL);

        std::vector<float> nodeToCnt;

        double srcCompCost = 0;
        double workerCompCostMax = 0;
        double workerCompCostSum = 0;

        long long globalCnt = run_mpi_for_static_network(input, hIO, workerNum, bufLen, srcCompCost, workerCompCostMax, workerCompCostSum);

        gettimeofday(&endTV, NULL);

        timersub(&endTV, &startTV, &diff);

        double elapsedTime = diff.tv_sec * 1000 + diff.tv_usec / 1000;
    }
    else
    {

        double srcCompCost = 0;
        double workerCompCostMax = 0;
        double workerCompCostSum = 0;
        std::vector<float> nodeToCnt;
        run_mpi_for_static_network(input, hIO, workerNum, bufLen, srcCompCost, workerCompCostMax, workerCompCostSum);
    }
}

void run_exp_dynamic_network(std::string input, std::string outPath, DistributionCoordinator &hIO, int workerNum, int bufLen)
{

    int seed = 0;

    struct timeval diff, startTV, endTV;

    
    //if (hIO.isMaster())
    //{
    gettimeofday(&startTV, NULL);

    std::vector<float> nodeToCnt;

    double srcCompCost = 0;
    double workerCompCostMax = 0;
    double workerCompCostSum = 0;

    std::cout << "Before running dynamic network" << std::endl;

    run_mpi_for_dynamic_network(input, hIO, workerNum, bufLen, srcCompCost, workerCompCostMax, workerCompCostSum);

    gettimeofday(&endTV, NULL);

    timersub(&endTV, &startTV, &diff);

    double elapsedTime = diff.tv_sec * 1000 + diff.tv_usec / 1000;
    //}
    /*else
    {
        double srcCompCost = 0;
        double workerCompCostMax = 0;
        double workerCompCostSum = 0;
        std::vector<float> nodeToCnt;
        run_mpi_for_dynamic_network(input, hIO, workerNum, bufLen, srcCompCost, workerCompCostMax, workerCompCostSum);
    }*/
}
