#ifndef DSC_DIST_RUN
#define DSC_DIST_RUN

#include <unordered_map>
#include <unordered_set>
#include "graph.h"
#include "types.h"
#include <dirent.h>
#include <sys/stat.h>
#include "distributioncoordinator.hpp"


double run_mpi_for_static_network(const char* filename, DistributionCoordinator &coordinator, int workerNum, int lenBuf, 
    double &srcCompCost, double &workerCompCostMax, double &workerCompCostSum);

void run_mpi_for_dynamic_network(std::string filename, DistributionCoordinator &coordinator, int workerNum, int lenBuf, double &srcCompCost, double &workerCompCostMax, double &workerCompCostSum);

double run_mpi_for_hpec_graph(const char* filename, DistributionCoordinator &coordinator, int workerNum, int lenBuf, 
    double &srcCompCost, double &workerCompCostMax, double &workerCompCostSum);

void run_exp (const char* input, const char* outPath, DistributionCoordinator &coordinator, int workerNum, int bufLen=1000);

void run_exp_dynamic_network (std::string input, std::string outPath, DistributionCoordinator &coordinator, int workerNum, int bufLen=100);

void run_exp_hpec_graph(const char* input, const char* outPath, DistributionCoordinator &coordinator, int workerNum, int bufLen=1000);


#endif