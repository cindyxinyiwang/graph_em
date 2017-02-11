/** 
 * Copyright 2010 Dror Marcus. All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 * 
 *    1. Redistributions of source code must retain the above copyright notice, this list of
 *       conditions and the following disclaimer.
 * 
 *    2. Redistributions in binary form must reproduce the above copyright notice, this list
 *       of conditions and the following disclaimer in the documentation and/or other materials
 *       provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY DROR MARCUS ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DROR MARCUS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those of the
 * authors and should not be interpreted as representing official policies, either expressed
 * or implied, of Dror Marcus.
 */

#include <iostream>
#include <fstream>
#include <string>
#include "Graph.h"
using namespace std;

Graph::Graph(void)
:m_GraphMatrix(0) // Set the matrix default value to 0
{
	// No nodes untill file load
	m_NodeCount = 0;
	m_bIsSymetric = true;
	m_NodeDegreesCount = NULL;
	m_UndirectedEdgeCount = 0;
}

Graph::Graph(Graph& rhs)
{
	// No nodes untill file load
	m_NodeCount = 0;
	m_bIsSymetric = true;
	m_NodeDegreesCount = NULL;
	m_UndirectedEdgeCount = 0;

	operator=(rhs);
}
Graph::~Graph(void)
{
	if (m_NodeDegreesCount != NULL)
		delete m_NodeDegreesCount;
}

Graph& Graph::operator=(Graph& rhs)
{
	if (this ==&rhs)
		return *this;
	m_bIsSymetric = rhs.m_bIsSymetric;
	if (m_NodeDegreesCount != NULL && m_NodeCount != rhs.m_NodeCount)
	{
		delete [] m_NodeDegreesCount;
		m_NodeDegreesCount = NULL;
	}

	m_NodeCount = rhs.m_NodeCount;
	m_UndirectedEdgeCount = rhs.m_UndirectedEdgeCount;

	if (m_NodeDegreesCount == NULL)
	{
		m_NodeDegreesCount = new int[m_NodeCount];
	}
	memcpy(m_NodeDegreesCount, rhs.m_NodeDegreesCount, sizeof(int) * m_NodeCount);
	m_GraphMatrix = rhs.m_GraphMatrix;
	return *this;
}

void Graph::AddEdge(int SrcNodeId, int DstNodeId, EdgeType Type)
{
	if (m_GraphMatrix.element(SrcNodeId, DstNodeId) != 0)
	{
		m_NodeDegreesCount[SrcNodeId]++;
		m_NodeDegreesCount[DstNodeId]++;
		m_UndirectedEdgeCount++;
	}

	m_GraphMatrix.element(SrcNodeId, DstNodeId) = Type;

	// TODO : Move to a makro\inline funtion
	if (m_bIsSymetric || Type == UNDIRECTED)
	{
		m_GraphMatrix.element(DstNodeId, SrcNodeId) = Type;
	}
	else if (Type == REGULAR)
	{
		m_GraphMatrix.element(DstNodeId, SrcNodeId) = REVERSE;
	}
	else if (Type == REVERSE)
	{
		m_GraphMatrix.element(DstNodeId, SrcNodeId) = REGULAR;
	}
	else
	{
		// TODO WARNING?
		m_GraphMatrix.element(DstNodeId, SrcNodeId) = Type;
	}
}

bool Graph::Load(string filename)
{
	ifstream adjGraphFile;
	adjGraphFile.open(filename.c_str());
	if (!adjGraphFile.is_open())
	{
		
		return false;
	}

	string CurrLine;
	int NodeA;
	int NodeB;

	// TODO : get the maximum elelment value here and the row count and set the max element then
	m_NodeIDHash.Init(); // For now, the parameters are not important
	
	// First Loop - obtain all node id's and node count plus the edge count for the matrix
	int MaxEdgeCount = 0;
	int NodeCount=0;
	int tmp = 0;
	while (!adjGraphFile.eof())
	{
		NodeA = -1;
		NodeB = -1;
		adjGraphFile >> NodeA >> NodeB;
		
		// If only one node was read (something is bad here... set error value)
		if (NodeB == -1)
		{
			NodeA = -1;
		}
		else
		{
			MaxEdgeCount++;
			if (m_NodeIDHash.GetKey(NodeA, tmp) == IDictionary::DICTIONARY_NEW_ELEMENT)
			{
				NodeCount++;
			}
			if (m_NodeIDHash.GetKey(NodeB, tmp) == IDictionary::DICTIONARY_NEW_ELEMENT)
			{
				NodeCount++;
			}
		}
	}

	// Sec Loop - Load all edges to the matrix
	InitRawMatrix(NodeCount, MaxEdgeCount);

	adjGraphFile.clear();
	adjGraphFile.close();
	adjGraphFile.open(filename.c_str());
	if (!adjGraphFile.is_open())
	{
		return false;
	}

	while (!adjGraphFile.eof())
	{
		NodeA = -1;
		NodeB = -1;
		adjGraphFile >> NodeA >> NodeB;
		
		// If only one node was read (something is bad here... set error value)
		if (NodeB == -1)
		{
			NodeA = -1;
		}
		else
		{
			m_NodeIDHash.GetKey(NodeA, NodeA);
			m_NodeIDHash.GetKey(NodeB, NodeB);
			
			_SetEdge(NodeA, NodeB, 1);
		}
	}

	// Calculate the Node Degree
	GraphIteratorID Iter;
	for (int Node = 1; Node <= NodeCount; Node++)
	{
		Iter = this->GetNeighborListIterator(Node);
		
		m_NodeDegreesCount[Node-1] = 0;

		for (int NodeNbr = this->GetNextNeighbor(Iter); 
			 NodeNbr != -1; 
			 NodeNbr = this->GetNextNeighbor(Iter))
		{
			if (Node!=NodeNbr) m_NodeDegreesCount[Node-1]++;
		}
	}
	return true;
}