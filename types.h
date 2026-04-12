#ifndef TYPES_H
#define TYPES_H

#include <cstdint>
#include <iostream>
#include <limits>
#include <stdlib.h>

typedef unsigned int ui;
typedef double decimal;

typedef uint32_t VertexID;
typedef short NodeID;
typedef ui LabelID;

const VertexID INVALID_VID(std::numeric_limits<VertexID>::max());
const NodeID INVALID_MID(std::numeric_limits<NodeID>::max());

const int MPI_MASTER = 0;
const int missingMId = -1;

const VertexID PCSR_INITIAL_SIZE = 100000;

const int TAG_BUFFER = 1;
const int TAG_BUFFER_SIZE = 2;
const int TAG_COUNT = 3;
const int BATCH_LENGTH = 150;
const ui GRAPH_CAPACITY = 20000;

struct GraphMetaData
{
	static const unsigned short szAttr = 2;
	ui vtx_cnt;
    ui edge_cnt;
	NodeID partition_id;

	GraphMetaData():vtx_cnt(INVALID_VID), edge_cnt(INVALID_VID), partition_id(INVALID_MID) {}

	inline void setValue(VertexID v_cnt, VertexID e_cnt, NodeID p_id)
	{
		vtx_cnt = v_cnt;
        edge_cnt = e_cnt;
		partition_id = p_id;
	}
};

struct Edge 
{
	static const unsigned short szAttr 	= 5;

	VertexID src;
	VertexID dst;
	NodeID src_ptn;
	NodeID dst_ptn;
	bool add;

	Edge(): src(INVALID_VID), dst(INVALID_VID) {}
	Edge(VertexID iSrc, VertexID iDst): src(iSrc), dst(iDst) {}
    Edge(const Edge &iEdge): src(iEdge.src), dst(iEdge.dst), src_ptn(iEdge.src_ptn), dst_ptn(iEdge.dst_ptn), add(iEdge.add) {}

	
    inline bool operator==(const Edge& iEdge) const 
	{
		return (src == iEdge.src) && (dst == iEdge.dst);
	}
    
	inline bool operator!=(const Edge& iEdge) const 
	{	
		return !operator==(iEdge);
	}

};

struct WedgeCnt
{
	static const unsigned short szAttr = 3;
	VertexID first_vtx;
    VertexID third_vtx;
	VertexID cnt;

	WedgeCnt():first_vtx(INVALID_VID), third_vtx(INVALID_VID) {}
	inline void setValue(VertexID first, VertexID third, VertexID iCnt)
	{
		first_vtx = first;
        third_vtx = third;
		cnt = iCnt;
	}
};




#endif