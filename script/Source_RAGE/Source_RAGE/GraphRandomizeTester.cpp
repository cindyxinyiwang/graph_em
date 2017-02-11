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

#include "GraphRandomizeTester.h"
#include <time.h>
#include <algorithm>
using namespace std;

GraphRandomizeTester::GraphRandomizeTester(void)
{
	unsigned int tmp = time(NULL);
	srand(tmp);
}

GraphRandomizeTester::~GraphRandomizeTester(void)
{
}

void GraphRandomizeTester::RandomizeGraph(Graph& CurrentGraph, 
										  //std::vector<int>& m_arrNodesTickets,
											int TotalTickets,
											int NoiseIterations)
{
	int ticket;
	int nodeindex;
	int currNodeDeg;
	std::vector<int>::iterator it; 

	// Count the edge types removed for the add edge statistics 
	// We first remove edges, and used the removed edge types for statistics gathering on the fly, 
	// to determin later on, for each edge what type it should be (the probabilty for it to be undirected\from high degree node to low and visa versa)...
	int EdgeTypesCount[4] = {1,1,1,1};
//	int HighDegToLowDegEdge = 1;
//	int LowDegToHighDegEdge = 1;
//	int DoubleSideEdge = 1;
	GraphIteratorID edgeit;

	int oppNode;
	Graph::EdgeType edgeType;

	// Edge removal iterations
	for (int i = 0; i < NoiseIterations; i++)
	{
		// Select a node to remove
		ticket = (int)(((double)rand() * rand() / (double)(RAND_MAX*RAND_MAX)) * (double)(TotalTickets - 1));
		it = upper_bound(m_arrNodesTickets.begin(), m_arrNodesTickets.end(), ticket);
		nodeindex = it - m_arrNodesTickets.begin();
		nodeindex++;
		
		// Select the edge to remove
		currNodeDeg = CurrentGraph.GetNodeDegree(nodeindex);
		if (currNodeDeg == 0)
			continue;
		ticket = (int)(((double)rand() * rand() / (double)(RAND_MAX*RAND_MAX)) * (double)(currNodeDeg - 1));
		edgeit = CurrentGraph.GetNeighborListIterator(nodeindex);
		edgeType = (Graph::EdgeType)0;
		for (int j = 0 ; j < ticket; j++)
		{
			oppNode = CurrentGraph.GetNextNeighbor(edgeit, edgeType);
		}
		oppNode = CurrentGraph.GetCurrNeighbor(edgeit, edgeType);
		if (currNodeDeg > CurrentGraph.GetNodeDegree(oppNode))
		{
			if (edgeType == Graph::REGULAR)
			{
				edgeType = Graph::REVERSE;
			}
			else if (edgeType == Graph::REVERSE)
			{
				edgeType = Graph::REGULAR;
			}
		}

		switch (edgeType)
		{
		case Graph::REGULAR:
			{
				EdgeTypesCount[0]++;
				break;
			}
		case Graph::REVERSE:
			{
				EdgeTypesCount[1]++;
				break;
			}
		case Graph::UNDIRECTED:
			{
				EdgeTypesCount[2]++;
				break;
			}
		default:
			break;
		}

		CurrentGraph.RemoveEdge(edgeit);
		CurrentGraph.ClearNeighborListIterator(edgeit);
	}

	int destnodeindex;
	int TotalTypeCount = EdgeTypesCount[0]+EdgeTypesCount[1]+EdgeTypesCount[2];
	EdgeTypesCount[1] += EdgeTypesCount[0];
	EdgeTypesCount[2] += EdgeTypesCount[1];

	// Edge Addition Itertation
	for (int i = 0; i < NoiseIterations; i++)
	{
		// Select a node to add an edge from
		ticket = (int)(((double)rand() * rand() / (double)(RAND_MAX*RAND_MAX)) * (double)(TotalTickets - 1));
		it = upper_bound(m_arrNodesTickets.begin(), m_arrNodesTickets.end(), ticket);
		nodeindex = it - m_arrNodesTickets.begin();
		nodeindex++;
		
		// Select a node to add the edge to
		ticket = (int)(((double)rand() * rand() / (double)(RAND_MAX*RAND_MAX)) * (double)(TotalTickets - 1));
		it = upper_bound(m_arrNodesTickets.begin(), m_arrNodesTickets.end(), ticket);
		destnodeindex = it - m_arrNodesTickets.begin();
		destnodeindex++;

		if (destnodeindex == nodeindex)
			continue;

		// Set the src node to be the lower degree node
		if (CurrentGraph.GetNodeDegree(nodeindex) > CurrentGraph.GetNodeDegree(destnodeindex))
		{
			oppNode = nodeindex;
			nodeindex = destnodeindex;
			destnodeindex = oppNode;
		}

		// Determin the edge type.
		ticket = (int)(((double)rand() / RAND_MAX) * (double)(TotalTypeCount - 1));
		if (ticket < EdgeTypesCount[0])
		{
			edgeType = Graph::REGULAR;
		}
		else if (ticket < EdgeTypesCount[1])
		{
			edgeType = Graph::REVERSE;
		}
		else
		{
			edgeType= Graph::UNDIRECTED;
		}

		CurrentGraph.AddEdge(nodeindex, destnodeindex, edgeType);
	}
}

void GraphRandomizeTester::RunTest(Graph& InputGraph, 
				DirectedMotifCount& CountAlg,					  
				int RandomGraphCount, 
				float NoiseAmount, 
				string StorePath,
				bool StoreRandGraphs, 
				bool StorePerGraphResults,
				DirectedMotifResults* NormalizedResutls)
{
	m_OriginalGraph = &InputGraph;
	m_GraphRootFolder = StorePath;
	m_NRandomGraphCount= RandomGraphCount;

	m_arrNodesTickets.assign(m_OriginalGraph->GetNodeCount() + 1, 0);

	// Sanity check, TODO ASSERT?
	if (NormalizedResutls == NULL)
		return;

	m_GraphResultsArray = NormalizedResutls;
	
	// Init Lotto array 
	int TotalTickets = 0;
	for (int i = 1; i < m_OriginalGraph->GetNodeCount() + 1; i++)
	{
		TotalTickets += m_OriginalGraph->GetNodeDegree(i);
		m_arrNodesTickets[ i - 1] = TotalTickets;
	}

	m_arrNodesTickets[m_OriginalGraph->GetNodeCount()] = TotalTickets + 1;

	if (TotalTickets == 0 || NoiseAmount > 1)
		return;

	// Create graphs
	Graph CurrentGraph;
	int NoiseIterations = NoiseAmount * (TotalTickets / 2);

	if (NoiseIterations == 0 )
		NoiseIterations++;

	for (int i = 0 ; i < m_NRandomGraphCount; i++)
	{
		CurrentGraph = *m_OriginalGraph;

		// Add some noise to the graph
		RandomizeGraph(CurrentGraph, 
						//m_arrNodesTickets,
						TotalTickets,
						NoiseIterations);

		if (StoreRandGraphs)
		{
			// TODO : STORE NEW GRAPH
		}

		CountAlg.MotifCount(CurrentGraph, &NormalizedResutls[i], false);

		if (StorePerGraphResults)
		{
			// Store results
		}
	}

//	delete [] m_arrNodesTickets;
}

