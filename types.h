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

struct Edge 
{
	static const unsigned short szAttr 	= 3;

	VertexID src;
	VertexID dst;
	bool add;

	Edge(): src(INVALID_VID), dst(INVALID_VID) {}
	Edge(VertexID iSrc, VertexID iDst): src(iSrc), dst(iDst) {}
    Edge(const Edge &iEdge): src(iEdge.src), dst(iEdge.dst) {}
	
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
	inline void setValue(VertexID first, VertexID third, VertexID iCnt)
	{
		first_vtx = first;
        third_vtx = third;
		cnt = iCnt;
	}
};


#endif