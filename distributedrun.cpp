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

