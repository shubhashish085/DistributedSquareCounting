#include <fstream>
#include "partition.hpp"
#include "hpecgraph.hpp"
#include "hpecworkergraph.hpp"
#include "distributedrun.hpp"
#include "distributioncoordinator.hpp"

double run_mpi_hpec_graph(const char *filename, DistributionCoordinator &hIO, int workerNum, int lenBuf, double &srcCompCost, double &workerCompCostMax, double &workerCompCostSum)
{
    clock_t begin = clock();

    hIO.init(lenBuf, workerNum);

    ui *partition_limit = new ui[workerNum];
    ui n_partition = workerNum;

    int rank = hIO.getRank();
    HpecGraph *hpec_graph = new HpecGraph();

    if (hIO.isMaster())
    {
        hpec_graph->loadGraphFromFile(filename);
        hpec_graph->transformToDirectedGraph();
        GraphPartitioning::hpec_even_degree_partition(hpec_graph, n_partition);

        Edge edge;
        std::ifstream infile(filename);

        VertexID begin, end;
        NodeID dst_partition;

        for(VertexID i = 0; i < hpec_graph->vertices_count; i++){
            for(ui j = hpec_graph->directed_offsets[i]; j < hpec_graph->directed_offsets[i + 1]; j++){
                edge.src = i;
                edge.dst = hpec_graph->directed_nbrs[j];
                hIO.sendEdge(edge, hpec_graph->vertex_partition_map[i]);
            }
        }

        hIO.sendEndSignal();
    }

    if (hIO.isMaster())
    {

        long long globalCnt = 0;


        return globalCnt;
    }
    else
    {

        HpecWorkerGraph *worker_graph = new HpecWorkerGraph();
        Edge edge;


        while (hIO.recvEdge(edge))
        {
            if (edge.add)
            {
                worker_graph->insert_edge(edge);
            }
        }

        //worker_graph->fillData();


        double workerCompCost = (double(clock() - begin) - hIO.getIOCPUTime()) / CLOCKS_PER_SEC;

        hIO.sendTime(workerCompCost);
        return 0;
    }
}


void run_exp_hpec_graph(const char *input, const char *outPath, DistributionCoordinator &hIO, int workerNum, int bufLen)
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

        long long globalCnt = 0;
        run_mpi_for_dynamic_network(input, hIO, workerNum, bufLen, srcCompCost, workerCompCostMax, workerCompCostSum);

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
        run_mpi_for_dynamic_network(input, hIO, workerNum, bufLen, srcCompCost, workerCompCostMax, workerCompCostSum);
    }
}
