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

#include "UndirectedRAGECount.h"
#include "Math.h"
#include "List.h"
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

UndirectedRAGECount::UndirectedRAGECount(void)
{
}

UndirectedRAGECount::~UndirectedRAGECount(void)
{
}

/**
 * The main algorithm, see www.eng.tau.ac.il/~shavitt/pub/Simplex2010.pdf
 */
void UndirectedRAGECount::CountGraphlets(Graph& SearchGraph, 
								MotifResults* pResults, 
								bool bIsGraphSymetric,
								bool bConvertToInduced)
{
	SortedLinkedList<int> MainNodeNeighbors;
	SortedLinkedList<int> NbrNodeNeghbors;
	SortedLinkedList<int> MergedNodeNeghbors;
	SortedLinkedList<int>::Iterator MainNodeNeighborsIter(&MainNodeNeighbors);
	SortedLinkedList<int>::Iterator MainNodeNeighborsIter2(&MainNodeNeighbors);
	SortedLinkedList<int>::Iterator NbrNodeNeghborsIter(&NbrNodeNeghbors);
	SortedLinkedList<int>::Iterator MergedNodeNeghborsIter(&MergedNodeNeghbors);

	// Sanity Check
	if (pResults == NULL || pResults->GetNodeCount() != SearchGraph.GetNodeCount())
	  return; // TODO: This is an ERROR, maybe assert here

	uint64 NodeCount = SearchGraph.GetNodeCount();
	int NbrNode;
	int NbrNodeNbr;
	GraphIteratorID GraphIter = 0;

	// Set a tmp node array that will hold the merged list indication...
	// Since the nodes are 1-start index we will add 1 to the total count and ignore 0 index
	// Less bugs this way...
	int* tmpNodesArray = new int[NodeCount + 1];
	for (int i = 0; i < NodeCount + 1; i++)
		tmpNodesArray[i] = 0;

	// Clear all results arrays
	pResults->ClearMotifResults(MotifResults::TRIANGLE);
	pResults->ClearMotifResults(MotifResults::TAILED_TGL_CNTR);
	pResults->ClearMotifResults(MotifResults::TAILED_TGL_REG);
	pResults->ClearMotifResults(MotifResults::TAILED_TGL_TAIL);
	pResults->ClearMotifResults(MotifResults::CLIQUE4);
	pResults->ClearMotifResults(MotifResults::CYC_CHORD_ONCHORD);
	pResults->ClearMotifResults(MotifResults::CYC_CHORD_REG);
	pResults->ClearMotifResults(MotifResults::CLAW_CNTR);
	pResults->ClearMotifResults(MotifResults::CLAW_EDGE);

	/*********** NEW IMP *******/
	pResults->ClearMotifResults(MotifResults::PATH3_EDGE);
	pResults->ClearMotifResults(MotifResults::PATH3_INNR);
	pResults->ClearMotifResults(MotifResults::PATH4_EDGE);
	pResults->ClearMotifResults(MotifResults::PATH4_INNR);
	pResults->ClearMotifResults(MotifResults::CYCLE4);
	/*********** NEW IMP *******/

time_t start_time = time(0);
	// Go over all edges (by going over all two nbr nodes), 
	// If we are symmetric for now we go over all v \in N(u) s.t. u <= v
	// ------------------------------------------------------------------
	for (int MainNode = 1; MainNode <= NodeCount; MainNode++)
	{
		#if PROGRESS_INFO
			fprintf(stderr, "\rCountGraphlets # %5d (%.1f%%) [%d min elapsed]", MainNode, 
              100.0 * (float) MainNode / NodeCount, (time(0) - start_time) / 60);
		#endif
		DBG_ASSERT(GraphIter == NULL);
		GraphIter = SearchGraph.GetNeighborListIterator(MainNode);
		NbrNode = SearchGraph.GetNextNeighbor(GraphIter);
		
		// Get the main node neighbors in a list (will be used in list merge later)
		while (NbrNode > 0)
		{
			MainNodeNeighbors.AddToList(NbrNode);
			NbrNode = SearchGraph.GetNextNeighbor(GraphIter);
		}
		SearchGraph.ClearNeighborListIterator(GraphIter);

		MainNodeNeighborsIter.RestartIterator();
	
		// Go over all edges connected to the Main Node (by going through all its neighbors
		while (!MainNodeNeighborsIter.EndOfList())		
		{
			NbrNode = MainNodeNeighborsIter.GetNextValue();

			// If we are sym. we only go over half the nodes (so not to count an edge twice)
			if (MainNode <= NbrNode || !bIsGraphSymetric)
			{
				DBG_ASSERT(GraphIter == NULL);
				GraphIter = SearchGraph.GetNeighborListIterator(NbrNode);
				NbrNodeNbr = SearchGraph.GetNextNeighbor(GraphIter);

				// Get the Path count for the color 
				while (NbrNodeNbr > 0)
				{
					NbrNodeNeghbors.AddToList(NbrNodeNbr);
					NbrNodeNbr = SearchGraph.GetNextNeighbor(GraphIter);
				}

				// Merge both lists to get L
				MainNodeNeighborsIter2.RestartIterator();
				NbrNodeNeghborsIter.RestartIterator();
				MergedNodeNeghbors.Clear();
				
				// While there are still possible equal elements
				while(!MainNodeNeighborsIter2.EndOfList() || !NbrNodeNeghborsIter.EndOfList())
				{
					if (NbrNodeNeghborsIter.EndOfList() || 
						(!MainNodeNeighborsIter2.EndOfList() && MainNodeNeighborsIter2.Value() < NbrNodeNeghborsIter.Value()))
					{
						// Make sure this is not the nbr node
						if (MainNodeNeighborsIter2.Value() != NbrNode)
						tmpNodesArray[MainNodeNeighborsIter2.Value()] = NODE_IN_MAIN_LIST;
						MainNodeNeighborsIter2.AdvanceIterator();
					}
					else if (MainNodeNeighborsIter2.EndOfList() || 
						     NbrNodeNeghborsIter.Value() < MainNodeNeighborsIter2.Value())
					{
						// Make sure this is not the main node
						if (NbrNodeNeghborsIter.Value() != MainNode)
							tmpNodesArray[NbrNodeNeghborsIter.Value()] = NODE_IN_NBR_LIST;
						NbrNodeNeghborsIter.AdvanceIterator();
					}
					else // they are equal
					{
						// Add to merged list (need be make sure we are not adding the main, nbr nodes)
						if (MainNodeNeighborsIter2.Value() != MainNode &&
							MainNodeNeighborsIter2.Value() != NbrNode)
						{
								MergedNodeNeghbors.AddToList(MainNodeNeighborsIter2.Value());
								tmpNodesArray[MainNodeNeighborsIter2.Value()] = NODE_IN_MERGED_LIST;
						}

						MainNodeNeighborsIter2.AdvanceIterator();
						NbrNodeNeghborsIter.AdvanceIterator();
					}
				}
				SearchGraph.ClearNeighborListIterator(GraphIter);

				/**
					Now Count the triangles, cliques, chords... 
				**/

				/***************/
				/** Triangles **/
				/***************/
				// For each node in L (MergedNodeNeghbors) we get a triangle, dont forget we count each triangle 
				// twice (once for each edge connected to u  on the triangle)
				// so \2 FIXUP NEEDED
				// (remember that we handled that if we are symmetric this edge will be only reached once
				//  so we need to update both Main and Nbr Nodes)
				pResults->Result(MotifResults::TRIANGLE, MainNode) += MergedNodeNeghbors.GetCount(); 
				pResults->Result(MotifResults::TRIANGLE, NbrNode) += MergedNodeNeghbors.GetCount(); 
				
				/*********************/
				/** Tailed Triangle **/
				/*********************/
				
				// CNTR NODE
				// =========
				// For triangle we got above (remember that for each triangle we get the nodes twice so we need to fix later)
				// so \2 FIXUP NEEDED LATER
				// We can know by the (degree - 2) the amount of tailed triangle with the node as the center
				uint64 MainNodeTails = (SearchGraph.GetNodeDegree(MainNode) - 2);
				if (MainNodeTails < 0 ) MainNodeTails =0;
				uint64 MainNodeTailTrngls = MergedNodeNeghbors.GetCount() * (SearchGraph.GetNodeDegree(MainNode) - 2);
				pResults->Result(MotifResults::TAILED_TGL_CNTR, MainNode) += MainNodeTailTrngls; 
				uint64 NbrNodeTails = (SearchGraph.GetNodeDegree(NbrNode) - 2);
				if (NbrNodeTails < 0 ) NbrNodeTails =0;
				uint64 NbrNodeTailedTrngls = MergedNodeNeghbors.GetCount() * (SearchGraph.GetNodeDegree(NbrNode) - 2);
				pResults->Result(MotifResults::TAILED_TGL_CNTR, NbrNode) += NbrNodeTailedTrngls; 

				// Theoretically, we do not add the reg node count for the nbr/main nodes, we can add it only once later, 
				// when the edge disjoint to each of the nodes in the triangle will be countd. BUT
				// We fix all reg nodes because we find each triangle twice. If we do not set the reg nodes in this case
				// also twice (*2 larger) then the fixup will mess things up.
				// NOTICE: If we were to choose to fix the TAILED_TGL_CNTR before calculating the TAILED_TGL_REG
				// Then we should comment out the following 2 commands.??????????????
				//pResults->Result(MotifResults::TAILED_TGL_REG, NbrNode) += MainNodeTailTrngls; 
				//pResults->Result(MotifResults::TAILED_TGL_REG, MainNode) += NbrNodeTailedTrngls; 

				// REG NODE
				// =========
				// Add this result to all merged nodes as nodes not on the center
				// Each node gets updated for this only once, when the selected Edge, defined by (MainNode,NbrNode)
				// which is not connected to the node. Since only one such edge exists in a triangle, this only happance once.
				// so NO FIXUP NEEDED
				// TO IMPROVE run time we do this inside the cycle with chord loop
				/*MergedNodeNeghborsIter.RestartIterator();
				while (!MergedNodeNeghborsIter.EndOfList())
				{
					pResults->Result(MotifResults::TAILED_TGL_REG, MergedNodeNeghborsIter.Value()) += (MainNodeTailTrngls + NbrNodeTailedTrngls); 
					MergedNodeNeghborsIter.AdvanceIterator();
				}*/ // COMMENTED AND MOVED TO CYCLE WITH CHORD

				// TAIL NODE
				// =========
				// Add a result to all tails. This is "hard" (time complexity...)
				// to do this we want to go over all the nodes in the graph, and for each node add the trngles the 
				// neghibor node is in, BUT the main node will be in some of those triangles so we will remove 
				// the total result with its own triangle count * 2 (for each node in the tringle we counted the triangle)
				// so FIXUP NEEDED: [- 2*triangles(node)] (remember to use this after fixing triangle results)
				/* No Code here, Tail node code will appeare in the function end, after fixup */

				
				/**********************/
				/** Cycle with Chord **/
				/**********************/
				// For each 2 node in L (MergedNodeNeighbors) we get a chord, 
				// This is interesting: we only find this chord motif once! when we go over the chord edge,
				// so NO FIXUP NEEDED (both ONCHORD and REGNODE)

				// We need to calculate for the MainNode, NbrNode the binom(|list|, 2) but for each node in the list
				// we get a chord by adding any other edge, that is: binom(|list|-1, 1) = |list|-1.
				uint64 MainNodeChordCount = /*binom(|list|,2) = |list|*(|list|-1)/2*/ MergedNodeNeghbors.GetCount() * (MergedNodeNeghbors.GetCount()-1) / 2;

				// ONCHORD NODE
				// ============
				pResults->Result(MotifResults::CYC_CHORD_ONCHORD, MainNode) += MainNodeChordCount; 
				pResults->Result(MotifResults::CYC_CHORD_ONCHORD, NbrNode) += MainNodeChordCount; 

				// REG NODE
				// =========
				// Set all chord nodes not on chord (CYC_CHORD_REG)
				MergedNodeNeghborsIter.RestartIterator();
				while (!MergedNodeNeghborsIter.EndOfList())
				{
					// Updatge the chord count
					// for each node in the list we get a chord by adding any other edge, that is: binom(|list|-1, 1) = |list|-1.
					pResults->Result(MotifResults::CYC_CHORD_REG, MergedNodeNeghborsIter.Value()) += MergedNodeNeghbors.GetCount()-1; 

					// Update the Triangle REG NODE count (see comment in the above triangle section)
					pResults->Result(MotifResults::TAILED_TGL_REG, MergedNodeNeghborsIter.Value()) += (MainNodeTails + NbrNodeTails); 

					MergedNodeNeghborsIter.AdvanceIterator();
				}


				/*************/
				/** Cliques **/
				/*************/
				// Each node pair in MergedNodeNeighbors that are also neighbors indicate a clique
				// need to devide by 3, not 6 (each pair is counted once and not twice as in Mira's artical, 
				// each mainnode has 3 center edges in clique)
				// so \3 FIXUP NEEDED
				MergedNodeNeghborsIter.RestartIterator();
				while (!MergedNodeNeghborsIter.EndOfList())
				{
					// TODO : decide if we should keep 	MergedNodeNeghbors as a list or array of whole nodes... not sure which is better
					// The best thing is if we knew that the elements arrive sorted, then we could really do this efficiantly
					DBG_ASSERT(GraphIter == NULL);
					GraphIter = SearchGraph.GetNeighborListIterator(MergedNodeNeghborsIter.Value());
					NbrNodeNbr = SearchGraph.GetNextNeighbor(GraphIter);
					
					// Get the Path count for the color 
					while (NbrNodeNbr > 0)
					{
						// If we are symmetric only count the neighboring pairs once
						if ((MergedNodeNeghborsIter.Value() <= NbrNodeNbr || !bIsGraphSymetric)
							&& NbrNodeNbr != NbrNode // Just in case we get self-loops for some reason
							&& tmpNodesArray[NbrNodeNbr] == NODE_IN_MERGED_LIST) // Main condition: an element of the list has a neighbor in the list
						{
							pResults->Result(MotifResults::CLIQUE4,MainNode)++;
							pResults->Result(MotifResults::CLIQUE4,NbrNode)++;
						}
						NbrNodeNbr = SearchGraph.GetNextNeighbor(GraphIter);
					}
					SearchGraph.ClearNeighborListIterator(GraphIter);
					MergedNodeNeghborsIter.AdvanceIterator();
				}
				
			
				/**************************
				/**************************
				/* NEW IMPLEMENTS!!!!!!!!!
				/**************************
				/**************************/
				/*************/
				/** PATH    **/
				/*************/
				uint64 pathval = (NbrNodeNeghbors.GetCount()-1)*(MainNodeNeighbors.GetCount()-1) - MergedNodeNeghbors.GetCount();
				pResults->Result(MotifResults::PATH4_INNR,MainNode) +=pathval;
				pResults->Result(MotifResults::PATH4_INNR,NbrNode)+= pathval;
								
				MainNodeNeighborsIter2.RestartIterator();
				NbrNodeNeghborsIter.RestartIterator();
				
				// While there are still possible equal elements

				while(!MainNodeNeighborsIter2.EndOfList())
				{
					if (MainNodeNeighborsIter2.Value() != NbrNode)
					{
						// If we are merged then there is one less option...
						if (tmpNodesArray[MainNodeNeighborsIter2.Value()] == NODE_IN_MERGED_LIST)
							pResults->Result(MotifResults::PATH4_EDGE,MainNodeNeighborsIter2.Value())+= (NbrNodeNeghbors.GetCount() - 1) - 1;
						else
							pResults->Result(MotifResults::PATH4_EDGE,MainNodeNeighborsIter2.Value())+= (NbrNodeNeghbors.GetCount() - 1) ;
					}
					MainNodeNeighborsIter2.AdvanceIterator();
				}

				while(!NbrNodeNeghborsIter.EndOfList())
				{
					if (NbrNodeNeghborsIter.Value() != MainNode)
					{
						// TODO : DROR YOU MUST CHECK HERE IF THIS IS MERGED, FOR NOW VALUE IS BAD
						// RUNTIME ANALYSIS ONLY
						if (tmpNodesArray[NbrNodeNeghborsIter.Value()] == NODE_IN_MERGED_LIST)
							pResults->Result(MotifResults::PATH4_EDGE,NbrNodeNeghborsIter.Value())+= (MainNodeNeighbors.GetCount() - 1) - 1;
						else
							pResults->Result(MotifResults::PATH4_EDGE,NbrNodeNeghborsIter.Value())+= (MainNodeNeighbors.GetCount() - 1);
					}
					NbrNodeNeghborsIter.AdvanceIterator();
				}

				
				/*************/
				/** CYCLE   **/
				/*************/
				MainNodeNeighborsIter2.RestartIterator();
				while (!MainNodeNeighborsIter2.EndOfList())
				{
					// Ignor the current neighbor (we are looking for different neighbors)
					if (MainNodeNeighborsIter2.Value() != NbrNode)
					{
						// TODO : decide if we should keep 	MergedNodeNeghbors as a list or array of whole nodes... not sure which is better
						// The best thing is if we knew that the elements arrive sorted, then we could really do this efficiantly
						DBG_ASSERT(GraphIter == NULL);
						GraphIter = SearchGraph.GetNeighborListIterator(MainNodeNeighborsIter2.Value());
						NbrNodeNbr = SearchGraph.GetNextNeighbor(GraphIter);
						
						// Get the Path count for the color 
						while (NbrNodeNbr > 0)
						{
							// If we are symmetric only count the neighboring pairs once
							if (NbrNodeNbr != NbrNode // Just in case we get self-loops for some reason
								&& tmpNodesArray[NbrNodeNbr] == NODE_IN_MERGED_LIST || tmpNodesArray[NbrNodeNbr] == NODE_IN_NBR_LIST) // Main condition: an element of the list has a neighbor in the list
							{
								pResults->Result(MotifResults::CYCLE4,MainNode)++;
								pResults->Result(MotifResults::CYCLE4,NbrNode)++;
							}
							NbrNodeNbr = SearchGraph.GetNextNeighbor(GraphIter);
						}
						SearchGraph.ClearNeighborListIterator(GraphIter);
					}
					MainNodeNeighborsIter2.AdvanceIterator();
				}
				/**************************
				/**************************
				/* END NEW IMPLEMENTS !!!!!
				/**************************/
				
				// Clear the tmp node array with the nbr valus				
				NbrNodeNeghborsIter.RestartIterator();
				while(!NbrNodeNeghborsIter.EndOfList())
				{
					tmpNodesArray[NbrNodeNeghborsIter.Value()] = 0;
					NbrNodeNeghborsIter.AdvanceIterator();
				}
				
				MergedNodeNeghbors.Clear();
				NbrNodeNeghbors.Clear();
			} // end of If 
		} // End of nbr loop

		// Clear the tmp node array with the main node valus				
		MainNodeNeighborsIter2.RestartIterator();
		while(!MainNodeNeighborsIter2.EndOfList())
		{
			tmpNodesArray[MainNodeNeighborsIter2.Value()] = 0;
			MainNodeNeighborsIter2.AdvanceIterator();
		}
		MainNodeNeighbors.Clear();
	}
	
	// Fix results arrays and Calculate CLAW MOTIF and the Center of 3-Path
	uint64 TempCount = 0;
	uint64 CurrClawCount = 0;
	for (int i = 1; i <= NodeCount; i++)
	{
		// FIXUP
		pResults->Result(MotifResults::TRIANGLE, i) /= 2;
		pResults->Result(MotifResults::TAILED_TGL_CNTR, i) /= 2;
		pResults->Result(MotifResults::CYCLE4, i) /= 2;
		pResults->Result(MotifResults::CLIQUE4,i) /= 3;

		/*************/
		/** CLAWS   **/
		/*************/
		// Claw CNTR
		//==========
		// Claw center is defined by binom(Degree, 3) = (deg)!/[(deg - 3)! * 3!] = [(deg)(deg-1)(deg-2)]/6
		// (Make sure we have degree >= 3)
		CurrClawCount = SearchGraph.GetNodeDegree(i);
		if (CurrClawCount >= 3)
		{
			TempCount = (((CurrClawCount)*(CurrClawCount - 1)*(CurrClawCount - 2))/6);
			pResults->Result(MotifResults::CLAW_CNTR, i) = TempCount;
		}

		// 3Path CNTR (PATH3_INNR)
		//========================
		// 3Path center is defined by binom(Degree, 2) = (deg)!/[(deg - 2)! * 2!] = [(deg)(deg-1)]/2]
		// (Make sure we have degree >= 2)
		if (CurrClawCount >= 2)
		{
			TempCount = (((CurrClawCount)*(CurrClawCount - 1))/2);
			pResults->Result(MotifResults::PATH3_INNR, i) = TempCount;
		}

		// CLAW END
		// ========
		// For runtime optimization this will appear in TAILED TRIANGLE LOOP COUNT
	}

	
	// TAIL TRIANGLE NODE CALCULATION
	// ==============================
	// As written above, we go over all nodes, for each add the total triangle count of its neighbors, and remove the 
	// amount of (triangles the node appear in)*2, because we overcount by that amount..
	for (int MainNode = 1; MainNode <= NodeCount; MainNode++)
	{
		DBG_ASSERT(GraphIter == NULL);
		GraphIter = SearchGraph.GetNeighborListIterator(MainNode);
		NbrNode = SearchGraph.GetNextNeighbor(GraphIter);

		while (NbrNode > 0)
		{
			// TAILED CONT
			// ===========
			pResults->Result(MotifResults::TAILED_TGL_TAIL, MainNode) += pResults->Result(MotifResults::TRIANGLE, NbrNode);

			// CLAW END
			// ========
			// For each MainNode Claw end is simply the summation of each nbr's binom(degree-1, 2) = [(deg-1)(deg-2)]/2
			// that is select two out of the other negibor's neighbors
			// Make sure the nbr has degree >= 3
			CurrClawCount = SearchGraph.GetNodeDegree(NbrNode);
			if (CurrClawCount >= 3)
			{
				TempCount = ((CurrClawCount-1)*(CurrClawCount - 2))/2;
				pResults->Result(MotifResults::CLAW_EDGE, MainNode) += TempCount;
			}

			// PATH 3 END
			// ==========
			// For each MainNode path 3 end is simply the summation of each nbr's degree-1
			// that is select one out of the other negibor's neighbors
			// Make sure the nbr has degree >= 2
			if (CurrClawCount >= 2)
			{
				pResults->Result(MotifResults::PATH3_EDGE, MainNode) += (CurrClawCount-1);
			}

			NbrNode = SearchGraph.GetNextNeighbor(GraphIter);
		}
		SearchGraph.ClearNeighborListIterator(GraphIter);
		// Fix the tail count by removing the tringles this node appears in times 2 (as described above)
		pResults->Result(MotifResults::TAILED_TGL_TAIL, MainNode) -= 2*pResults->Result(MotifResults::TRIANGLE, MainNode);
	}

	delete [] tmpNodesArray;

	if (bConvertToInduced)
		ConvertResultsToInduced(pResults);
}


void UndirectedRAGECount::ConvertResultsToInduced(MotifResults* pResults)
{
	if (pResults == NULL)
		return;

	// Go over all nodes to normalize. 
	for (int i = 1; i<= pResults->GetNodeCount(); i++)
	{
		// The order for the normalization should not be changed!!!!!!!!!!!
		// This order is based on dependencies 

		// -----------------------------
		// No need to normalize Cliques
		// -----------------------------
		
		// ---------------
		// Normalize Chord
		// ---------------
		
		// CYC_CHORD_ONCHORD
		pResults->Result(MotifResults::CYC_CHORD_ONCHORD, i) -= 3*pResults->Result(MotifResults::CLIQUE4, i);

		// CYC_CHORD_REG
		pResults->Result(MotifResults::CYC_CHORD_REG, i) -= 3*pResults->Result(MotifResults::CLIQUE4, i);

		// ---------------
		// Normalize CYCLE
		// ---------------
		// CYCLE
		pResults->Result(MotifResults::CYCLE4, i) -= (3*pResults->Result(MotifResults::CLIQUE4, i) + 
													  pResults->Result(MotifResults::CYC_CHORD_ONCHORD, i)+
													  pResults->Result(MotifResults::CYC_CHORD_REG, i));

		// -------------------------
		// Normalize TAILED_TRIANGLE
		// -------------------------
		// TAILED_TGL_TAIL
		pResults->Result(MotifResults::TAILED_TGL_TAIL, i) -= (3*pResults->Result(MotifResults::CLIQUE4, i)+
													  2*pResults->Result(MotifResults::CYC_CHORD_REG, i));
		// TAILED_TGL_CNTR
		pResults->Result(MotifResults::TAILED_TGL_CNTR, i) -= (3*pResults->Result(MotifResults::CLIQUE4, i) + 
													  2*pResults->Result(MotifResults::CYC_CHORD_ONCHORD, i));

		// TAILED_TGL_REG
		pResults->Result(MotifResults::TAILED_TGL_REG, i) -= (6*pResults->Result(MotifResults::CLIQUE4, i) + 
													  2*pResults->Result(MotifResults::CYC_CHORD_ONCHORD, i)+
													  2*pResults->Result(MotifResults::CYC_CHORD_REG, i));

		// --------------
		// Normalize CLAW
		// --------------
		// CLAW_CNTR
		pResults->Result(MotifResults::CLAW_CNTR, i) -= (pResults->Result(MotifResults::CLIQUE4, i) + 
													  pResults->Result(MotifResults::CYC_CHORD_ONCHORD, i)+
													  pResults->Result(MotifResults::TAILED_TGL_CNTR, i));
		// CLAW_EDGE
		pResults->Result(MotifResults::CLAW_EDGE, i) -= (3*pResults->Result(MotifResults::CLIQUE4, i) + 
													  pResults->Result(MotifResults::CYC_CHORD_ONCHORD, i)+
													  2*pResults->Result(MotifResults::CYC_CHORD_REG, i) +
													  pResults->Result(MotifResults::TAILED_TGL_TAIL, i) +
													  pResults->Result(MotifResults::TAILED_TGL_REG, i));


		// ---------------
		// Normalize PATH4
		// ---------------
		// PATH4_EDGE
		pResults->Result(MotifResults::PATH4_EDGE, i) -= (6*pResults->Result(MotifResults::CLIQUE4, i) + 
													  2*pResults->Result(MotifResults::CYC_CHORD_ONCHORD, i)+
													  4*pResults->Result(MotifResults::CYC_CHORD_REG, i) +
													  2*pResults->Result(MotifResults::CYCLE4, i) +
													  2*pResults->Result(MotifResults::TAILED_TGL_TAIL, i) +
													  pResults->Result(MotifResults::TAILED_TGL_REG, i));

		// PATH4_INNR
		pResults->Result(MotifResults::PATH4_INNR, i) -= (6*pResults->Result(MotifResults::CLIQUE4, i) + 
													  4*pResults->Result(MotifResults::CYC_CHORD_ONCHORD, i)+
													  2*pResults->Result(MotifResults::CYC_CHORD_REG, i) +
													  2*pResults->Result(MotifResults::CYCLE4, i) +
													  2*pResults->Result(MotifResults::TAILED_TGL_CNTR, i) +
													  pResults->Result(MotifResults::TAILED_TGL_REG, i));

		// ------------------------------
		// No need to normalize TRIANGLES
		// ------------------------------

		// ----- 
		// PATH3
		// -----
		// PATH3_EDGE
		pResults->Result(MotifResults::PATH3_EDGE, i) -= 2*pResults->Result(MotifResults::TRIANGLE, i);

		// PATH3_INNER
		pResults->Result(MotifResults::PATH3_INNR, i) -= pResults->Result(MotifResults::TRIANGLE, i);

	}// End all nodes for loop

}