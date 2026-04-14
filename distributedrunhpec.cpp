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
