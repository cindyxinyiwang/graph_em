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
 *
 * Algorithm implemntation of <SET PERMISSIONS HERE!>
 */

#include "BIUDirectedMotifCount.h"
#include "Math.h"
#include <time.h>

#define PROGRESS_INFO 1
#include <stdlib.h>
#include <stdio.h>

#define NODE_IN_MERGED_LIST 3
#define NODE_IN_MAIN_LIST 1
#define NODE_IN_NBR_LIST 2

#define DEBUG_ASSERT
#ifdef DEBUG_ASSERT
#include "assert.h"
#define DBG_ASSERT(X) assert(X)
#else
#define DBG_ASSERT(X)
#endif
const int BIUDirectedMotifCount::MARKED_NODE = -1;
void BIUDirectedMotifCount::UpdateCount(DirectedMotifResults* pResults, NodeMotifRolesFinder& MotifRoles ,int Connections, Graph& graph, int FirstNode, int SecNode, int ThirdNode, int FourthNode)
{

	int NodeIds[4] = {FirstNode, SecNode, ThirdNode, FourthNode};
	unsigned char ConnectionMat[4][4] = {0};
	int EdgeType;
	for (int i = 0; i < 4; i++)
	{
		for (int j = i+1; j < 4; j++)
		{
			EdgeType = graph.CheckIfNeigbors(NodeIds[i], NodeIds[j]);
			if (EdgeType == Graph::REGULAR)
			{
				ConnectionMat[i][j] = 1;
			}
			else if (EdgeType == Graph::REVERSE)
			{
				ConnectionMat[j][i] = 1;
			}
			else if (EdgeType == Graph::UNDIRECTED)
			{
				ConnectionMat[i][j] = 1;
				ConnectionMat[j][i] = 1;
			}
		}
	}

	int Mat = 0;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			Mat = Mat << 1;
			if (ConnectionMat[3-i][3-j] == 1)
				Mat |= 1;
		}
	}


	const HashElement& role = MotifRoles.GetRoleHash().GetElement(Mat);
	for (int i = 0; i < 4; i++)
	{
		(*(pResults->Result(NodeIds[i], MotifRoles.GetRoleHash().ParseID(role.motifID), role.Roles[i])))++;
	}
}

void BIUDirectedMotifCount::CountSecondPattern(DirectedMotifResults* pResults,
											   NodeMotifRolesFinder& MotifRoles,
											   Graph& graph, 
												int MainNode, 
												int SecondNode, 
												int ThirdNode, 
												SortedLinkedList<int>& secondNodeNeghbors, 
												SortedLinkedList<int>& thirdNodeNeghbors, 
												int* NodeMarkArray)
{
	SortedLinkedList<int>::Iterator SecNbrNodeNeghborsIter(&secondNodeNeghbors);
	SortedLinkedList<int>::Iterator ThirdNbrNodeNeghborsIter(&thirdNodeNeghbors);

	int FourthNode = 0;
	// While there are still possible equal elements
	while(!SecNbrNodeNeghborsIter.EndOfList() && !ThirdNbrNodeNeghborsIter.EndOfList())
	{
		if (SecNbrNodeNeghborsIter.Value() < ThirdNbrNodeNeghborsIter.Value())
		{
			// Make sure this is not the nbr node
			FourthNode = SecNbrNodeNeghborsIter.Value();
			SecNbrNodeNeghborsIter.AdvanceIterator();
		}
		else if (SecNbrNodeNeghborsIter.Value() > ThirdNbrNodeNeghborsIter.Value())
		{
			FourthNode = ThirdNbrNodeNeghborsIter.Value();
			ThirdNbrNodeNeghborsIter.AdvanceIterator();
		}
		else // they are equal
		{
			FourthNode = ThirdNbrNodeNeghborsIter.Value();
			SecNbrNodeNeghborsIter.AdvanceIterator();
			ThirdNbrNodeNeghborsIter.AdvanceIterator();
		}
		UpdateCount(pResults, MotifRoles, 0, graph, MainNode, SecondNode, ThirdNode, FourthNode);
	}
	while(!SecNbrNodeNeghborsIter.EndOfList())
	{
		UpdateCount(pResults, MotifRoles, 0, graph, MainNode, SecondNode, ThirdNode, SecNbrNodeNeghborsIter.GetNextValue());
	}
	while(!ThirdNbrNodeNeghborsIter.EndOfList())
	{
		UpdateCount(pResults, MotifRoles, 0, graph, MainNode, SecondNode, ThirdNode, ThirdNbrNodeNeghborsIter.GetNextValue());
	}
}


void BIUDirectedMotifCount::BuildNeighborList(int Level, SortedLinkedList<int>& list, Graph& graph, int NodeId, int MainNode, int* NodeMarkArray, bool bUpdateNodeArray)
{
		int NbrNode;
		GraphIteratorID GraphIter = 0;
		GraphIter = graph.GetNeighborListIterator(NodeId);
		NbrNode = graph.GetNextNeighbor(GraphIter);

		// Get the main node neighbors in a list (will be used in list merge later)
		while (NbrNode > 0)
		{
			// Ignor nodes we gone over before... 
			if (NbrNode > MainNode && NodeMarkArray[NbrNode] >= Level)
			{
				list.AddToList(NbrNode);

				if (bUpdateNodeArray)
					NodeMarkArray[NbrNode] = Level;
			}
			NbrNode = graph.GetNextNeighbor(GraphIter);
		}
		graph.ClearNeighborListIterator(GraphIter);
}

void BIUDirectedMotifCount::MotifCount(Graph& SearchGraph,
						   DirectedMotifResults* pResults, 
					       bool bIsGraphSymetric)
{
///////////////////////
	///////////////////
	/////////////////////
	/////////////////
	
	NodeMotifRolesFinder* pNodeRoles = new NodeMotifRolesFinder();
	pNodeRoles->LoadRoles();

	SortedLinkedList<int> MainNodeNeighbors;
	SortedLinkedList<int> secondNodeNeghbors;
	SortedLinkedList<int> thirdNodeNeghbors;

	SortedLinkedList<int>::Iterator LstSecoundMainNodeNeighborsIter(&MainNodeNeighbors);
	SortedLinkedList<int>::Iterator LstThirdNodeNeighborsIter(&MainNodeNeighbors);
	SortedLinkedList<int>::Iterator LstFourthNodeNeighborsIter(&MainNodeNeighbors);
	SortedLinkedList<int>::Iterator LstSecNbrNodesIter(&secondNodeNeghbors);

	// Sanity Check
	if (pResults == NULL || pResults->GetNodeCount() != SearchGraph.GetNodeCount())
	  return; // TODO: This is an ERROR, maybe assert here

	uint64 NodeCount = SearchGraph.GetNodeCount();
	GraphIteratorID GraphIter = 0;


	// Set a tmp node array that will hold the merged list indication...
	// Since the nodes are 1-start index we will add 1 to the total count and ignore 0 index
	// Less bugs this way...
	int* tmpNodesArray = new int[NodeCount + 1];
	for (int i = 0; i < NodeCount + 1; i++)
		tmpNodesArray[i] = 10; // Set to the lowest possible type


time_t start_time = time(0);
	// Go over all nodes 

	for (int MainNode = 1; MainNode <= NodeCount; MainNode++)
	{
		#if PROGRESS_INFO
			fprintf(stderr, "\rCountGraphlets # %5d (%.1f%%) [%d min elapsed]", MainNode, 
              100.0 * (float) MainNode / NodeCount, (time(0) - start_time) / 60);
		#endif
		DBG_ASSERT(GraphIter == NULL);

		// Mark the main node, so we will know we whent over it...
		tmpNodesArray[MainNode] = MARKED_NODE;
		BuildNeighborList(1,MainNodeNeighbors,SearchGraph, MainNode, MainNode, tmpNodesArray);
		LstSecoundMainNodeNeighborsIter.RestartIterator(&MainNodeNeighbors);
	
		while (!LstSecoundMainNodeNeighborsIter.EndOfList())		
		{
			BuildNeighborList(2,secondNodeNeghbors,SearchGraph, LstSecoundMainNodeNeighborsIter.Value(), MainNode, tmpNodesArray);
			
			LstThirdNodeNeighborsIter.RestartIterator(LstSecoundMainNodeNeighborsIter);
			LstThirdNodeNeighborsIter.AdvanceIterator();

			while (!LstThirdNodeNeighborsIter.EndOfList())		
			{
				LstFourthNodeNeighborsIter.RestartIterator(LstThirdNodeNeighborsIter);
				LstFourthNodeNeighborsIter.AdvanceIterator();

				// FIRST PATTERN COUNT
				// -------------------
				while (!LstFourthNodeNeighborsIter.EndOfList())		
				{
					// TODO : !!! We can obtain 3 edges only from the iteration, leaving us with only 2 edges to
					// Check.. in this iteration we know all edges connected to the main node. all needed to check is the
					// edges connected to other nodes (2-3, 2-4, 3-4) 
					//UpdateCount();
					UpdateCount(pResults, *pNodeRoles, 0, SearchGraph, MainNode, 
								LstSecoundMainNodeNeighborsIter.Value(), 
								LstThirdNodeNeighborsIter.Value(), 
								LstFourthNodeNeighborsIter.Value());
					LstFourthNodeNeighborsIter.AdvanceIterator();
				}
				
				// SECOND PATTERN COUNT
				// --------------------
				// Create Neighbor lists of secound and third.
				// Remember to add to the list only memebers that were not noticed in a higher level
				// Instead of Merging both list Do updated count (No need to store the merged list)
				BuildNeighborList(2,thirdNodeNeghbors,SearchGraph, LstThirdNodeNeighborsIter.Value(), MainNode, tmpNodesArray, false);

				
				CountSecondPattern(pResults,
					*pNodeRoles, 
					SearchGraph, 
					MainNode, 
					LstSecoundMainNodeNeighborsIter.Value(), 
					LstThirdNodeNeighborsIter.Value(), 
					secondNodeNeghbors, 
					thirdNodeNeghbors, 
					tmpNodesArray );

				thirdNodeNeghbors.Clear();
				LstThirdNodeNeighborsIter.AdvanceIterator();
			}

			// Iterate over all neighbors(sons in BFS) of second node
			LstThirdNodeNeighborsIter.RestartIterator(&secondNodeNeghbors);

			// Iterate over all neighbors(sons in BFS) of second node, that appear after third (the previous iterator).
			while (!LstThirdNodeNeighborsIter.EndOfList())		
			{
				LstFourthNodeNeighborsIter.RestartIterator(LstThirdNodeNeighborsIter);
				LstFourthNodeNeighborsIter.AdvanceIterator();
				while (!LstFourthNodeNeighborsIter.EndOfList())		
				{

					// THIRD PATTERN COUNT
					// -------------------
					// mainnode, second (son of mainnode), third (son of second), fourth (son of second) 
					// first, secound, N_Second[x], N_Secound[x+1..]

					// TODO : !!! We can obtain 3 edges only from the iteration, leaving us with only 2 edges to
					// Check.. in this iteration we know all edges connected to the main node. all needed to check is the
					// edges connected to other nodes (2-3, 2-4, 3-4) 
					//UpdateCount();
					UpdateCount(pResults, *pNodeRoles, 0, SearchGraph, MainNode, 
								LstSecoundMainNodeNeighborsIter.Value(), 
								LstThirdNodeNeighborsIter.Value(), 
								LstFourthNodeNeighborsIter.Value());
					LstFourthNodeNeighborsIter.AdvanceIterator();
				}

				// Build Neighbor list of neighbors(sons in BFS) of third which is a son of second, which is a son of mainnode
				// (lowest level, sons of sons of sons of mainnode)
				thirdNodeNeghbors.Clear();
				BuildNeighborList(3,thirdNodeNeghbors,SearchGraph, LstThirdNodeNeighborsIter.Value(), MainNode, tmpNodesArray, false);
				LstFourthNodeNeighborsIter.RestartIterator(&thirdNodeNeghbors);
				while (!LstFourthNodeNeighborsIter.EndOfList())		
				{
					// FOURTH PATTERN COUNT
					// -------------------
					// mainnode, second (son of mainnode), third (son of second), fourth (son of third) 
					// first, secound, N_Second[x], N_Third[y]

					UpdateCount(pResults, *pNodeRoles, 0, SearchGraph, MainNode, 
								LstSecoundMainNodeNeighborsIter.Value(), 
								LstThirdNodeNeighborsIter.Value(), 
								LstFourthNodeNeighborsIter.Value());
					LstFourthNodeNeighborsIter.AdvanceIterator();
				}
				thirdNodeNeghbors.Clear();
				LstThirdNodeNeighborsIter.AdvanceIterator();
			}

			// Clear secondNodeNeghbors marking
			for (LstSecNbrNodesIter.RestartIterator(&secondNodeNeghbors);!LstSecNbrNodesIter.EndOfList();LstSecNbrNodesIter.AdvanceIterator())
			{
				tmpNodesArray[LstSecNbrNodesIter.Value()] = 5;
			}
			secondNodeNeghbors.Clear();
			LstSecoundMainNodeNeighborsIter.AdvanceIterator();
		}


		// Clear MainNodeNeghbors marking
		for (LstSecoundMainNodeNeighborsIter.RestartIterator(&MainNodeNeighbors);!LstSecoundMainNodeNeighborsIter.EndOfList();LstSecoundMainNodeNeighborsIter.AdvanceIterator())
		{
			tmpNodesArray[LstSecoundMainNodeNeighborsIter.Value()] = 5;
		}
		MainNodeNeighbors.Clear();
	}
	delete [] tmpNodesArray;
	delete pNodeRoles;
}


BIUDirectedMotifCount::BIUDirectedMotifCount(void)
{
}

BIUDirectedMotifCount::~BIUDirectedMotifCount(void)
{
}

