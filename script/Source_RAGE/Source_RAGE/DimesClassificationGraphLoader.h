#pragma once
#include "graph.h"

class DimesClassificationGraphLoader :
	public GraphLoader
{
	enum EEdgeTypes
	{
		CustomerProvider = -1,
		ProviderCustomer = 1,
		P2P = 0,
		S2S = 2,
		Unknown = 10000
	};

	string m_filename;
public:
	DimesClassificationGraphLoader (string filename) {m_filename = filename;}
	DimesClassificationGraphLoader () {m_filename = "";}
	void SetEdgeListFilename(string filename) {m_filename = filename;}
	bool LoadGraph(Graph& graph, IDictionary* Dictionary, bool bIsDirected = true, bool bHasWeights = false);
};
