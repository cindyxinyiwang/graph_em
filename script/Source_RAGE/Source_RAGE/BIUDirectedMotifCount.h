#pragma once

#include <iostream>
#include "Graph.h"
#include "DirectedMotifResults.h"
#include "List.h"
#include "NodeMotifRolesFinder.h"

class DirectedMotifCount
{
public:
	virtual void MotifCount(Graph& SearchGraph,
				   DirectedMotifResults* pResults, 
				       bool bIsGraphSymetric) = 0;
};

class BIUDirectedMotifCount
	:public DirectedMotifCount
{
private:
	static const int MARKED_NODE;
	void CountSecondPattern(DirectedMotifResults* pResults, NodeMotifRolesFinder& MotifRoles,
									Graph& graph, 
									int MainNode, 
									int SecondNode, 
									int ThirdNode, 
									SortedLinkedList<int>& secondNodeNeghbors, 
									SortedLinkedList<int>& thirdNodeNeghbors, 
									int* NodeMarkArray);

	static void BuildNeighborList(int Level, 
								SortedLinkedList<int>& list, 
								Graph& graph, 
								int NodeId, 
								int MainNode, 
								int* NodeMarkArray,
								bool bUpdateNodeArray = true);
	virtual void UpdateCount(DirectedMotifResults* pResults, NodeMotifRolesFinder& MotifRoles, int Connections, Graph& graph, int FirstNode, int SecNode, int ThirdNode, int FourthNode);
public:
	
	void MotifCount(Graph& SearchGraph,
						   DirectedMotifResults* pResults, 
					       bool bIsGraphSymetric);

	BIUDirectedMotifCount(void);
	~BIUDirectedMotifCount(void);
};
