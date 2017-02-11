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
#include "DimesClassificationGraphLoader.h"

using namespace std;

bool DimesClassificationGraphLoader::LoadGraph(Graph& graph, IDictionary* Dictionary, bool bIsDirected, bool bHasWeights)
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
	int DimesClassification;
	int CaidaClassification;

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
			adjGraphFile >> NodeA >> NodeB >> tmpignorWeight >> DimesClassification >> CaidaClassification;
		}
		else
		{
			adjGraphFile >> NodeA >> NodeB >> DimesClassification >> CaidaClassification;
		}
		
		// If only one node was read (something is bad here... set error value)
		if (NodeB == -1)
		{
			NodeA = -1;
		}
		else if (NodeA != NodeB)
		{
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
	/////////////// DEBUG ////////////// CREATE LEDA GRAP
/*
	ofstream LedaFile;
	LedaFile.open("SMALL_TEST.gw");
    LedaFile << "LEDA.GRAPH" << endl << "string"<< endl << "short" << endl << NodeCount << endl;
	for (int i = 1; i<= NodeCount; i++)
		LedaFile << "|{" << Dictionary->_GetStringValue(i) << "}|" << endl;
*/
	/////////////////////////////////////////////////////

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
			adjGraphFile >> NodeA >> NodeB >> tmpignorWeight >> DimesClassification >> CaidaClassification;
		}
		else
		{
			adjGraphFile >> NodeA >> NodeB >> DimesClassification >> CaidaClassification;
		}
				
		// If only one node was read (something is bad here... set error value)
		if (NodeB == -1)
		{
			NodeA = -1;
		}
		else
		{
			Dictionary->GetKey(NodeA, NodeA);
			Dictionary->GetKey(NodeB, NodeB);

			// Incase unknown use the other classification
			if (DimesClassification == EEdgeTypes::Unknown)
			{
				DimesClassification = CaidaClassification;
			}

			if (DimesClassification == EEdgeTypes::CustomerProvider)
			{
				SetEdge(graph, NodeA, NodeB, Graph::EdgeType::REVERSE);
			}
			else if (DimesClassification == EEdgeTypes::ProviderCustomer)
			{
				SetEdge(graph, NodeA, NodeB, Graph::EdgeType::REGULAR);
			}
			else if (DimesClassification == EEdgeTypes::P2P)
			{
				SetEdge(graph, NodeA, NodeB, Graph::UNDIRECTED);
				SetEdge(graph, NodeB, NodeA, Graph::UNDIRECTED);
			}
			else if (DimesClassification == EEdgeTypes::S2S)
			{
				// TODO:!!!!!!!!!!!!!!!!
				SetEdge(graph, NodeA, NodeB, Graph::UNDIRECTED);
			}
			else if (DimesClassification == EEdgeTypes::Unknown)
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
	// ================================================
	// ================================================
/*	/////////////// DEBUG ////////////// CREATE LEDA GRAP
	LedaFile << tmpr << endl;

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
			/// ---------
			LedaFile << Node << " " << NodeNbr << " 0 |{1}|" << endl;
			/// ---------

			}
		}
	}

	LedaFile.close();

	// ================================================
*/	// ================================================
	return true;

}
