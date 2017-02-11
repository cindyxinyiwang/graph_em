#pragma once
#include <iostream>
#include <assert.h>
#include <string>
#include "cmsparse\isramatrix.h" 
#include "IIntDictionary.h"

using namespace std;

typedef ISRAMatrix::ColIterator* GraphIteratorID;

class GraphLoader;

class Graph
{
	friend class GraphLoader;

public:
	enum EdgeType
	{
		NONE = 0,
		REGULAR = 1,
		REVERSE = -1,
		UNDIRECTED = 3
	};


private:
	SRAMatrix<int> m_GraphMatrix;
	int* m_NodeDegreesCount;
	int m_NodeCount;
	int m_UndirectedEdgeCount;
	IIntDictionary m_NodeIDHash;
	bool m_bIsSymetric;
protected:
	inline void _SetEdge(int SrcNodeId, int DstNodeId, int Value);
	inline int _GetEdge(int SrcNodeId, int DstNodeId);
	inline void InitRawMatrix(int MaxNodeCount, int MaxEdgeCount, bool bIsSymetric=true);
public:
	Graph(void);
	void AddEdge(int SrcNodeId, int DstNodeId, EdgeType Type);
	explicit Graph(Graph& rhs);
	Graph &operator=(Graph& rhs);
	bool Load(string filename);
	int GetNodeCount(){return m_NodeCount;};
	int GetUndirEdgeCount(){return m_UndirectedEdgeCount;};
	//void GetNeighborList(int NodeId){return;};
	inline int GetNodeDegree(int Node);
	inline GraphIteratorID GetNeighborListIterator(int NodeId);
	//int ReStartNeighborListIterator(int NodeId, iter){return 0;};
	inline int ClearNeighborListIterator(GraphIteratorID& Iterator);
	inline int RemoveEdge(GraphIteratorID& Iterator);
	inline int GetNextNeighbor(GraphIteratorID Iterator);
	inline int GetNextNeighbor(GraphIteratorID Iterator, Graph::EdgeType &edgeType);
	inline int GetCurrNeighbor(GraphIteratorID Iterator, Graph::EdgeType &edgeType);

	// Checks if A is a neighbor of B TODO : If not symmetric, check if this works
	inline int CheckIfNeigbors(int NodeAId, int NodeBId){return m_GraphMatrix.const_element(NodeAId, NodeBId);/* TODO : CHANGED Returns that value of the element != 0;*/};
	
public:
	~Graph(void);
};

class GraphLoader
{
protected:
	inline int* NodeDegrees(Graph& graph);
	inline void InitRawMatrix(Graph& graph, int MaxNodeCount, int MaxEdgeCount, bool bIsSymmetric=true);
	inline void SetEdge(Graph& graph, int SrcNodeId, int DstNodeId, int Value);
	inline int  GetEdge(Graph& graph, int SrcNodeId, int DstNodeId);
	inline void SetUndirectedEdgeCount(Graph& graph, int UndirectedEdgeCount);
public:
	virtual bool LoadGraph(Graph& graph, IDictionary* Dictionary, bool bIsDirected, bool bHasWeights) = 0;
};

////////////////////////////
// Inline functions
////////////////////////////

// GRAPH LOADER
// ------------
inline void GraphLoader::SetUndirectedEdgeCount(Graph& graph, int UndirectedEdgeCount)
{
	graph.m_UndirectedEdgeCount = UndirectedEdgeCount;
}

inline int* GraphLoader::NodeDegrees(Graph& graph) 
{
	return graph.m_NodeDegreesCount;
}

inline void GraphLoader::InitRawMatrix(Graph& graph, int MaxNodeCount, int MaxEdgeCount, bool bIsSymmetric)
{
	return graph.InitRawMatrix(MaxNodeCount, MaxEdgeCount, bIsSymmetric);
}

inline void GraphLoader::SetEdge(Graph& graph, int SrcNodeId, int DstNodeId, int Value)
{
	return graph._SetEdge(SrcNodeId, DstNodeId ,Value);
}

inline int GraphLoader::GetEdge(Graph& graph, int SrcNodeId, int DstNodeId)
{
	return graph._GetEdge(SrcNodeId, DstNodeId);
}

// Graph
// -----

inline int Graph::GetNodeDegree(int Node)
{
	if ((Node > m_NodeCount) ||  (Node == 0) || (m_NodeDegreesCount == NULL))
		return 0;

	return m_NodeDegreesCount[Node-1];
}

inline void  Graph::InitRawMatrix(int MaxNodeCount, int MaxEdgeCount, bool bIsSymmetric)
{
	m_NodeCount = MaxNodeCount;
	m_GraphMatrix.Init(MaxEdgeCount*2+m_NodeCount*2, false);
	m_NodeDegreesCount = new int[m_NodeCount];
	m_bIsSymetric = bIsSymmetric;
}

void Graph::_SetEdge(int SrcNodeId, int DstNodeId, int Value)
{
	m_GraphMatrix.element(SrcNodeId, DstNodeId) = Value;

	if (m_bIsSymetric)
		m_GraphMatrix.element(DstNodeId, SrcNodeId) = Value;
}

int Graph::_GetEdge(int SrcNodeId, int DstNodeId)
{
	return m_GraphMatrix.element(SrcNodeId, DstNodeId);
}

GraphIteratorID Graph::GetNeighborListIterator(int NodeId) 
{
	ISRAMatrix::ColIterator* iter = new ISRAMatrix::ColIterator();
	m_GraphMatrix.restartColIterator(*iter, NodeId);
	return (GraphIteratorID)iter;
}

int Graph::ClearNeighborListIterator(GraphIteratorID& Iterator)
{
	if (Iterator != 0)
		delete (ISRAMatrix::ColIterator*)Iterator;
	Iterator = 0;
	return 0;
}

int Graph::RemoveEdge(GraphIteratorID& Iterator)
{
	ISRAMatrix::ColIterator* iter = (ISRAMatrix::ColIterator*)Iterator;
	
	if (iter == 0)
		return -2;
	
	int rRow,rCol;
	m_GraphMatrix.getCurrentInds(*iter,rRow, rCol);
	
	assert(m_NodeDegreesCount[ rRow - 1] > 0);
	assert(m_NodeDegreesCount[ rCol - 1] > 0);
	assert(m_NodeDegreesCount[m_UndirectedEdgeCount > 0]);
	m_NodeDegreesCount[ rRow - 1]--;
	m_NodeDegreesCount[ rCol - 1]--;	
	m_UndirectedEdgeCount--;
	

	m_GraphMatrix.removeCurrent(*iter);
	
	// TODO: Remove the other side of edge (the edge marking from other node connected), for now only mark "0"
	m_GraphMatrix.element(rCol, rRow) = 0;

	return 0;
}

int Graph::GetNextNeighbor(GraphIteratorID Iterator)
{
	ISRAMatrix::ColIterator* iter = (ISRAMatrix::ColIterator*)Iterator;
	
	if (Iterator == 0)
		return -2;
	
	int NextNbr = -1;
	int row = 0;
	int col = 0;
	while (!m_GraphMatrix.atEnd(*iter) && NextNbr == -1)
	{
		m_GraphMatrix.getCurrentInds(*iter,row,col);
		int* pVal = m_GraphMatrix.getCurrent(*iter);
		if (pVal != 0)
			NextNbr = row;
		m_GraphMatrix.advanceColIterator(*iter);
	}

	return NextNbr;
};

int Graph::GetNextNeighbor(GraphIteratorID Iterator, Graph::EdgeType &edgeType)
{
	ISRAMatrix::ColIterator* iter = (ISRAMatrix::ColIterator*)Iterator;
	
	if (Iterator == 0)
		return -2;
	
	int NextNbr = -1;
	int row = 0;
	int col = 0;
	while (!m_GraphMatrix.atEnd(*iter) && NextNbr == -1)
	{
		m_GraphMatrix.getCurrentInds(*iter,row,col);
		int* pVal = m_GraphMatrix.getCurrent(*iter);
		if (pVal != 0)
		{
			edgeType = (EdgeType)(*pVal);
			NextNbr = row;
		}
		else
		{
			edgeType = (EdgeType)0;
		}
		m_GraphMatrix.advanceColIterator(*iter);
	}

	return NextNbr;
};

int Graph::GetCurrNeighbor(GraphIteratorID Iterator, Graph::EdgeType &edgeType)
{
	ISRAMatrix::ColIterator* iter = (ISRAMatrix::ColIterator*)Iterator;
	
	if (Iterator == 0)
		return -2;
	
	int NextNbr = -1;
	int row = 0;
	int col = 0;
	while (!m_GraphMatrix.atEnd(*iter) && NextNbr == -1)
	{
		m_GraphMatrix.getCurrentInds(*iter,row,col);
		int* pVal = m_GraphMatrix.getCurrent(*iter);
		if (pVal != 0)
		{
			edgeType = (EdgeType)*pVal;
			NextNbr = row;
		}
		else
		{
			edgeType = (EdgeType)0;
		}
	}

	return NextNbr;
};
