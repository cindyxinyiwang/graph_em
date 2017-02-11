#include <iostream>
#include <fstream>
#include <string>
#include "CaidaASNGraphLoader.h"

using namespace std;

bool CaidaASNGraphLoader::LoadGraph(Graph& graph, IDictionary* Dictionary, bool bIsDirected, bool bHasWeights)
{
	if (m_filename == "" || Dictionary == NULL)
		return false;

	ifstream adjGraphFile;
	adjGraphFile.open(m_filename.c_str());
	if (!adjGraphFile.is_open())
	{
		return false;
	}

	string CurrLine;
	int NodeA;
	int NodeB;
	int Classification;

	int tmpignorWeight;
	// TODO : get the maximum elelment value here and the row count and set the max element then
	Dictionary->Init(); // For now, the parameters are not important
	
	// First Loop - obtain all node id's and node count plus the edge count for the matrix
	int MaxEdgeCount = 0;
	int NodeCount=0;
	int tmp = 0;

	while (!adjGraphFile.eof())
	{
		NodeA = -1;
		NodeB = -1;

		// If each edge row has weights also
		if(bHasWeights)
		{
			// for now we ignore the weights
			adjGraphFile >> NodeA >> NodeB >> tmpignorWeight >> Classification;
		}
		else
		{
			adjGraphFile >> NodeA >> NodeB >> Classification;
		}
		
		// If only one node was read (something is bad here... set error value)
		if (NodeB == -1)
		{
			NodeA = -1;
		}
		// AVOID SELF LOOPS
		else if (NodeA != NodeB)
		{
			MaxEdgeCount++;
			MaxEdgeCount++;
			if (Dictionary->GetKey(NodeA, tmp) == IDictionary::DICTIONARY_NEW_ELEMENT)
			{
				NodeCount++;
			}
			if (Dictionary->GetKey(NodeB, tmp) == IDictionary::DICTIONARY_NEW_ELEMENT)
			{
				NodeCount++;
			}
		}
	}

	// Sec Loop - Load all edges to the matrix
	InitRawMatrix(graph, NodeCount, MaxEdgeCount, false);

	adjGraphFile.clear();
	adjGraphFile.close();
	adjGraphFile.open(m_filename.c_str());
	if (!adjGraphFile.is_open())
	{
		return false;
	}

	while (!adjGraphFile.eof())
	{
		NodeA = -1;
		NodeB = -1;
		// If each edge row has weights also
		if(bHasWeights)
		{
			// for now we ignore the weights
			adjGraphFile >> NodeA >> NodeB >> tmpignorWeight >> Classification;
		}
		else
		{
			adjGraphFile >> NodeA >> NodeB >> Classification;
		}
				
		// If only one node was read (something is bad here... set error value)
		if (NodeB == -1)
		{
			NodeA = -1;
		}
		else if (NodeA != NodeB)
		{
			Dictionary->GetKey(NodeA, NodeA);
			Dictionary->GetKey(NodeB, NodeB);

			if (Classification == EEdgeTypes::CustomerProvider)
			{
				SetEdge(graph, NodeA, NodeB, Graph::EdgeType::REVERSE);
				SetEdge(graph, NodeB, NodeA, Graph::EdgeType::REGULAR);
			}
			else if (Classification == EEdgeTypes::ProviderCustomer)
			{
				SetEdge(graph, NodeA, NodeB, Graph::EdgeType::REGULAR);
				SetEdge(graph, NodeB, NodeA, Graph::EdgeType::REVERSE);
			}
			else if (Classification == EEdgeTypes::P2P)
			{
				SetEdge(graph, NodeA, NodeB, Graph::UNDIRECTED);
				SetEdge(graph, NodeB, NodeA, Graph::UNDIRECTED);
			}
			else if (Classification == EEdgeTypes::S2S)
			{
				// TODO:!!!!!!!!!!!!!!!!
				// SetEdge(graph, NodeA, NodeB, Graph::UNDIRECTED);
			}
			else if (Classification == EEdgeTypes::Unknown)
			{
				// TODO: ............
			}
			else
			{
				// TODO: ............
			}
		}
	}


	// Calculate the Node Degree
	GraphIteratorID Iter;
	int tmpr=0;
	for (int Node = 1; Node <= NodeCount; Node++)
	{
		Iter = graph.GetNeighborListIterator(Node);
		
		NodeDegrees(graph)[Node-1] = 0;

		for (int NodeNbr = graph.GetNextNeighbor(Iter); 
			 NodeNbr != -1; 
			 NodeNbr = graph.GetNextNeighbor(Iter))
		{
			if (Node!=NodeNbr) 
			{
				NodeDegrees(graph)[Node-1]++;
		
			}
			if (Node < NodeNbr)
			{
						tmpr++;
			}
		}
		graph.ClearNeighborListIterator(Iter);
	}
	SetUndirectedEdgeCount(graph, tmpr);

	return true;

}
