#pragma once
#include <iostream>
#include "Graph.h"
#include "DirectedMotifResults.h"
#include "BIUDirectedMotifCount.h"
#include <vector>
using namespace std;

// Class that coputes for a given graph, the per-node motif count normalized by N random graphs
class GraphRandomizeTester
{

private:
	int m_NRandomGraphCount;

	// Path to store the random generated graphs and their respective results
	string m_GraphRootFolder;
	
	// Flag indicating if each of random graph will be stored (in seperate files)
	bool m_StoreGenGraphs;
	
	// Flag indicating if the results of each random graph will be stored (in seperate files)
	bool m_StoreGenGraphsRes;

	// The original graph without any changes
	Graph* m_OriginalGraph;
	
	// Total Edge count of the original graph, this *2 is the total degree count that will be used
	// to randomly select a node (each node has a |degree#| lotto tickets) 
	int m_EdgeCount_OriginalGraph;	
	
	// Helps compute selection of node to remove edge from (the probability using Lottery tickets)
	class LottoTikets
	{
	public:
		int NodeID;
		int StartToken;
	};
	std::vector<int> m_arrNodesTickets;

	// Randomize the graph (iterations is the # of edges to remove and then # edges to add to the graph
	void RandomizeGraph(Graph& CurrentGraph,
						//std::vector<int>& m_arrNodesTickets,
						int TotalTickets,
						int NoiseIterations);

	// Holds results for each iteration
	DirectedMotifResults* m_GraphResultsArray;

public:
	GraphRandomizeTester(void);
	~GraphRandomizeTester(void);

	void RunTest(Graph& InputGraph, 
				DirectedMotifCount& CountAlg,					  
				int RandomGraphCount, 
				float NoiseAmount, 
				string StorePath,
				bool StoreRandGraphs, 
				bool StorePerGraphResults,
				DirectedMotifResults* NormalizedResutls);
	
	//void RestartTest(DirectedMotifResults* NormalizedResutls);
};
