#ifndef DSC_DIST_COORDINATOR
#define DSC_DIST_COORDINATOR

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <stddef.h>
#include <ctime>
#include <sys/time.h>
#include <mpi.h>
#include "graph.h"
#include "types.h"

class DistributionCoordinator {

public:

    class EdgeContainer {
	    public:
            NodeID					    mid;
            unsigned int				bit;
            unsigned int				qit;
            bool						isEmpty;
            double						ioCPUTime; // cpu time used for MPI communication

            Edge						*buf[2];
            MPI_Request 				req[2];

            EdgeContainer(): bit(0), qit(0), isEmpty(true), ioCPUTime(0),
                    buf{nullptr, nullptr}, req{MPI_REQUEST_NULL, MPI_REQUEST_NULL}{}

            ~EdgeContainer()
            {
                if (buf[0] != nullptr)
                {
                    delete[] buf[0];
                    buf[0] = nullptr;
                }

                if (buf[1] != nullptr)
                {
                    delete[] buf[1];
                    buf[1] = nullptr;
                }

                waitIOCompletion(req[0]);
                waitIOCompletion(req[1]);
            }

            inline bool init(NodeID iMid)
            {
                bit = 0;
                qit = 0;
                req[0] = MPI_REQUEST_NULL;
                req[1] = MPI_REQUEST_NULL;
                mid = iMid;
                buf[0] = new Edge[lenBuf];
                buf[1] = new Edge[lenBuf];

                //std::cout << "Edge Container Initialization Done " <<std::endl;
                
                return true;
            }

            inline void getNext_blocking(Edge &oEdge, std::vector<Edge>& edges, ui& length){

                /*if (isEmpty)
                {
                    clock_t begin = clock();

                    DistributionCoordinator::IrecvEdge_blocking(buf[qit], edges);                  
                    bit = 0;

                    ioCPUTime += double(clock() - begin);

                    isEmpty = false;
                } else*/ 
                /*if (bit == lenBuf)
                {
                    clock_t begin = clock();

                    std::cout << "LenBuf : " << lenBuf << std::endl;                    
                    DistributionCoordinator::IrecvEdge_blocking(buf[qit], edges);
                    /*qit = (qit + 1) % 2;
                    waitIOCompletion(req[qit]);
                    bit = 0;

                    ioCPUTime += double(clock() - begin);
                }*/

                clock_t begin = clock();

                                    
                DistributionCoordinator::IrecvEdge_blocking(buf[qit], edges, length);
                /*qit = (qit + 1) % 2;
                waitIOCompletion(req[qit]);*/
                bit = 0;

                ioCPUTime += double(clock() - begin);

                if(edges.size() > 0){
                    oEdge = edges[edges.size() - 1];
                }                

                return;
            }

            // Methods for receiver
            inline void getNext(Edge &oEdge)
            {
                if (isEmpty)
                {
                    clock_t begin = clock();

                    DistributionCoordinator::IrecvEdge(buf[qit], req[qit]);
                    waitIOCompletion(req[qit]);
                    DistributionCoordinator::IrecvEdge(buf[(qit+1)%2], req[(qit+1)%2]);

                    ioCPUTime += double(clock() - begin);

                    isEmpty = false;
                }
                else if (bit == lenBuf)
                {
                    clock_t begin = clock();

                    bit = 0;
                    DistributionCoordinator::IrecvEdge(buf[qit], req[qit]);
                    qit = (qit + 1) % 2;
                    waitIOCompletion(req[qit]);

                    ioCPUTime += double(clock() - begin);
                }

                oEdge = buf[qit][bit++];

                return;
            }

            void cleanup(){
                int flag(0);
                MPI_Status st;
                for (int i = 0; i < 2; i++)
                {
                    if (req[i] != MPI_REQUEST_NULL)
                    {
                        MPI_Test(&req[i], &flag, &st);
                        if (flag == false)
                        {
                            MPI_Cancel(&req[i]);
                        }
                        req[i] = MPI_REQUEST_NULL;
                    }
                }
            }

            // Methods for sender
            inline bool putNext(const Edge &iEdge)
            {

                buf[qit][bit++] = iEdge;
                if (bit == lenBuf)
                {
                    clock_t begin = clock();
                
                    int length = bit;
                    bit = 0;
                    
                    //std::cout << "Isending to Rank : " << mid << std::endl;
                    DistributionCoordinator::IsendEdge(buf[qit], mid, length, req[qit]);
                    qit = (qit + 1) % 2;
                    waitIOCompletion(req[qit]);

                    ioCPUTime += double(clock() - begin);
                }

                return true;
            }

            inline bool putNext_blocking(Edge &iEdge)
            {

                buf[qit][bit++] = iEdge;
                if (bit == lenBuf)
                {
                    clock_t begin = clock();
                
                    int length = bit;
                    
                    
                    //std::cout << "Isending to Rank : " << mid << std::endl;
                    DistributionCoordinator::IsendEdge_blocking(buf[qit], mid, length);
                    //qit = (qit + 1) % 2;
                    bit = 0;
                    //waitIOCompletion(req[qit]);

                    ioCPUTime += double(clock() - begin);
                }

                return true;
            }

            void flushSend()
            {

                clock_t begin = clock();

                if (bit != 0)
                {
                    int length = bit;
                    DistributionCoordinator::IsendEdge(buf[qit], mid, length, req[qit]);
                }

                bit = 0;

                for (int i = 0; i < 2; i++)
                {
                    waitIOCompletion(req[i]);
                }

                ioCPUTime += double(clock() - begin);
            }

            void flushSend_blocking()
            {

                clock_t begin = clock();

                if (bit != 0)
                {
                    int length = bit;
                    DistributionCoordinator::IsendEdge_blocking(buf[qit], mid, length);
                }

                bit = 0;

                /*for (int i = 0; i < 2; i++)
                {
                    waitIOCompletion(req[i]);
                }*/

                ioCPUTime += double(clock() - begin);
            }
	};


    static const int 		TAG_STREAM = 0;
	static const int 		TAG_RET	= 1;
	static MPI_Datatype 	MPI_TYPE_EDGE;
	static MPI_Datatype 	MPI_TYPE_WEDGE_CNT;
	static const Edge 		END_STREAM;
	static unsigned short	lenBuf;

    static bool IrecvEdge(Edge *buf, MPI_Request &iReq);
	static bool IsendEdge(Edge *buf, int dst, int length, MPI_Request &iReq);
    static bool IrecvEdge_blocking(Edge *buf, std::vector<Edge>& edges, ui& length);
	static bool IsendEdge_blocking(Edge *& buf, int dst, int length);
	
	MPI_Request				req;
	int 					rank;
	int 					szProc;
	int 					workerNum; // number of actual workers
	std::vector<EdgeContainer>	eBuf;
	long					commCostDistribute;
    long					commCostGather;
	long					ioCPUTime;

    DistributionCoordinator(int &argc, char** &argv);
	~DistributionCoordinator(){}    

    inline static void waitIOCompletion(MPI_Request &iReq)
	{
		MPI_Status 	st;
		(MPI_REQUEST_NULL != iReq) && MPI_Wait(&iReq, &st);
		iReq = MPI_REQUEST_NULL;
		return;
	}


	// initialize
	void init(int buffersize, int workerNum);
	void cleanup();

	// whether this thread is a master or not
	bool isMaster();


	// worker Id
	NodeID getWorkerId();

	// communication cost (logical)
	long getCommCostDistribute();
	long getCommCostGather();

	// Edge
	bool sendEdge(Edge &iEdge, NodeID dst);
	bool bCastEdge(Edge& iEdge);
    bool bCastPartitionLimit(ui*& partition_limit, int& workernum);
	bool recvEdge(Edge &oEdge);
    bool recvEdge_blocking(Edge &oEdge, std::vector<Edge>& edges, ui& length);

	// Receiving data
	bool sendWedgeCnt(std::map<std::pair<VertexID, VertexID>, ui>& wedge_map);
    bool recvWedgeCnt(std::map<std::pair<VertexID, VertexID>, ui>& wedge_map);

    bool sendCnt(long long & global_cnt);
    bool recvCnt(long long & global_cnt);

	// CPU Time used for MPI communication
	double getIOCPUTime();

	// send computational time info
	bool sendTime(double compTime);
	bool recvTime(double &compTimeMax, double &compTimeSum);

	// Control flow
	bool sendEndSignal();
    bool sendEndSignal_blocking();

	// Get variable
	int getRank(){
        return rank;
    }
	
    int getSzProc(){ 
        return szProc;
    }	
};


#endif