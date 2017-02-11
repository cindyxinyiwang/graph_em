#pragma once
#include "graph.h"

class CaidaASNGraphLoader :
	public GraphLoader
{
	enum EEdgeTypes
	{
		CustomerProvider = 1,
		ProviderCustomer = 2,
		P2P = 3,
		S2S = 0,
		Unknown = 10000
	};

	string m_filename;
public:
	CaidaASNGraphLoader (string filename) {m_filename = filename;}
	CaidaASNGraphLoader () {m_filename = "";}
	void SetEdgeListFilename(string filename) {m_filename = filename;}
	bool LoadGraph(Graph& graph, IDictionary* Dictionary, bool bIsDirected = true, bool bHasWeights = false);

};
