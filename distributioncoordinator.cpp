#include "distributioncoordinator.hpp"

MPI_Datatype 		DistributionCoordinator::MPI_TYPE_EDGE;
MPI_Datatype 		DistributionCoordinator::MPI_TYPE_WEDGE_CNT;
const Edge 			DistributionCoordinator::END_STREAM(INVALID_VID, INVALID_VID);

unsigned short		DistributionCoordinator::lenBuf;

DistributionCoordinator::DistributionCoordinator(int &argc, char** &argv)//, bit(0), lenCQ(1), cqit(0)
{
	// Establish connection
	MPI_Init(&argc, &argv);

	// Get connection information
	MPI_Comm_size(MPI_COMM_WORLD, &szProc);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	// Initialize and Register struct EDGE, ELEMCNT information
	int          lenAttr[Edge::szAttr] = {1, 1};
	MPI_Datatype arrType[Edge::szAttr] = {MPI_UNSIGNED, MPI_UNSIGNED, MPI_SHORT, MPI_SHORT, MPI_CXX_BOOL};

	MPI_Aint     offsets[Edge::szAttr];
	offsets[0] = offsetof(Edge, src);
	offsets[1] = offsetof(Edge, dst);
	offsets[2] = offsetof(Edge, src_ptn);
	offsets[3] = offsetof(Edge, dst_ptn);
	offsets[4] = offsetof(Edge, add);
	MPI_Type_create_struct(Edge::szAttr, lenAttr, offsets, arrType, &MPI_TYPE_EDGE);
	MPI_Type_commit(&MPI_TYPE_EDGE);

	arrType[0] = MPI_UNSIGNED;
	arrType[1] = MPI_DOUBLE;
	offsets[0] = offsetof(WedgeCnt, first_vtx);
    offsets[1] = offsetof(WedgeCnt, third_vtx);
	offsets[2] = offsetof(WedgeCnt, cnt);
	MPI_Type_create_struct(WedgeCnt::szAttr, lenAttr, offsets, arrType, &MPI_TYPE_WEDGE_CNT);
	MPI_Type_commit(&MPI_TYPE_WEDGE_CNT);
}

// Initialize requests and buffers
void DistributionCoordinator::init(int lenBuf, int workerNum)
{
	commCostDistribute = 0;
    commCostGather = 0;
    ioCPUTime = 0;

    eBuf.clear();
    DistributionCoordinator::lenBuf = lenBuf;
    DistributionCoordinator::workerNum = workerNum;

    if (rank == MPI_MASTER)
    {
        eBuf.resize(workerNum);
        for (int i = 0; i < workerNum; i++)
        {
            eBuf[i].init(i);
        }
    }
    else
    {
        eBuf.resize(1);
        eBuf[0].init(getWorkerId());
    }
}


bool DistributionCoordinator::isMaster()
{
    return rank == MPI_MASTER;
}

NodeID DistributionCoordinator::getWorkerId()
{
    return (NodeID)(rank - 1);
}

long DistributionCoordinator::getCommCostDistribute()
{
    return commCostDistribute;
}

long DistributionCoordinator::getCommCostGather()
{
    return commCostGather;

}

void DistributionCoordinator::cleanup()
{
	MPI_Finalize();
}

bool DistributionCoordinator::bCastEdge(Edge &iEdge)
{
	Edge tmpEdge(iEdge);
	for (int mit = 0; mit < workerNum; mit++)
	{
		eBuf[mit].putNext(tmpEdge);
	}
	commCostDistribute += workerNum;

	return true;
}




bool DistributionCoordinator::bCastPartitionLimit(ui*& partition_limit, int& workernum){

    
    return true;
}

bool DistributionCoordinator::IrecvEdge(Edge *buf, MPI_Request &iReq)
{
	return (MPI_SUCCESS == MPI_Irecv(buf, lenBuf, MPI_TYPE_EDGE, MPI_MASTER, TAG_STREAM, MPI_COMM_WORLD, &iReq));
	//waitIOCompletion(iReq);
}

bool DistributionCoordinator::IsendEdge(Edge *buf, int mid, MPI_Request &iReq){
	MPI_Isend(buf, lenBuf, MPI_TYPE_EDGE, mid + 1, TAG_STREAM, MPI_COMM_WORLD, &iReq);
	return true;
}

bool DistributionCoordinator::sendEdge(const Edge &iEdge, NodeID dst)
{
	Edge tmpEdge = iEdge;
	commCostDistribute++;
	eBuf[dst].putNext(tmpEdge);

	return true;
}

bool DistributionCoordinator::recvEdge(Edge &oEdge)
{
	eBuf[0].getNext(oEdge);
	if (oEdge == END_STREAM)
	{
		eBuf[0].cleanup();
	}
	return (oEdge != END_STREAM);
}

bool DistributionCoordinator::sendEndSignal()
{
	Edge signal(END_STREAM);
	for (int mit = 0; mit < workerNum; mit++)
	{
		eBuf[mit].putNext(signal);
		eBuf[mit].flushSend();
	}
	return true;
}


bool DistributionCoordinator::sendWedgeCnt(std::map<std::pair<VertexID, VertexID>, ui>& send_wedge_map)
{
    clock_t begin = clock();
    
    ioCPUTime += double(clock() - begin);

    ui buffer_size = send_wedge_map.size();
    std::vector<WedgeCnt> buffer(buffer_size);
    

    WedgeCnt wedge_cnt;
    for (const auto& [key, value] : send_wedge_map){
        wedge_cnt.first_vtx = key.first;
        wedge_cnt.third_vtx = key.second;
        wedge_cnt.cnt = value;
        buffer.push_back(wedge_cnt);
    }

    begin = clock();
    MPI_Send(&buffer_size, 1, MPI_INT, MPI_MASTER, TAG_BUFFER_SIZE, MPI_COMM_WORLD);
    MPI_Send(buffer.data(), buffer_size, MPI_TYPE_WEDGE_CNT, MPI_MASTER, TAG_BUFFER, MPI_COMM_WORLD);
    ioCPUTime += double(clock() - begin);

    return true;
}

bool DistributionCoordinator::recvWedgeCnt(std::map<std::pair<VertexID, VertexID>, ui>& recv_wedge_map)
{

    MPI_Status buffer_size_status, buffer_status;
	double empty = 0;
    clock_t begin = clock();

    ui buffer_size;
	
    ioCPUTime += double(clock() - begin);

    begin = clock();

    MPI_Recv(&buffer_size, 1, MPI_INT, rank, TAG_BUFFER_SIZE, MPI_COMM_WORLD, &buffer_size_status);

    std::vector<WedgeCnt> recv_buffer(buffer_size);

    MPI_Recv(recv_buffer.data(), buffer_size, MPI_TYPE_WEDGE_CNT, MPI_ANY_SOURCE, TAG_BUFFER, MPI_COMM_WORLD, &buffer_status);
    
    ui i = 0;
    std::pair<VertexID, VertexID> key_pair;

    while (i < recv_buffer.size()){
        key_pair = std::make_pair(recv_buffer[i].first_vtx, recv_buffer[i].third_vtx);
        if(recv_wedge_map.find(key_pair) != recv_wedge_map.end()){
            recv_wedge_map[key_pair] += recv_buffer[i].cnt;
        }else{
            recv_wedge_map[key_pair] += 0;
        }
        
        i++;
    }   

    ioCPUTime += double(clock() - begin);

	return true;
}

double DistributionCoordinator::getIOCPUTime()
{
	if (rank == MPI_MASTER)
	{
		double totalIOCPUTime = ioCPUTime;
		for (int i = 0; i < workerNum; i++)
		{
			totalIOCPUTime += eBuf[i].ioCPUTime;
		}
		return totalIOCPUTime;
	}
	else
	{
		return eBuf[0].ioCPUTime;
	}
}

bool DistributionCoordinator::sendTime(double compTime)
{
    MPI_Reduce(&compTime, nullptr, 1, MPI_DOUBLE, MPI_MAX, MPI_MASTER, MPI_COMM_WORLD);
    MPI_Reduce(&compTime, nullptr, 1, MPI_DOUBLE, MPI_SUM, MPI_MASTER, MPI_COMM_WORLD);
    return true;
}

bool DistributionCoordinator::recvTime(double &compTimeMax, double &compTimeSum)
{
	double empty = 0;
	MPI_Reduce(&empty, &compTimeMax, 1, MPI_DOUBLE, MPI_MAX, MPI_MASTER, MPI_COMM_WORLD);
    MPI_Reduce(&empty, &compTimeSum, 1, MPI_DOUBLE, MPI_SUM, MPI_MASTER, MPI_COMM_WORLD);
	return true;
}
