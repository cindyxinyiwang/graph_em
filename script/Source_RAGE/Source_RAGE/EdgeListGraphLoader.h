#pragma once
#include "graph.h"
#include <string.h>

class EdgeListGraphLoader :
	public GraphLoader
{
	string m_filename;
public:
	EdgeListGraphLoader(string filename) {m_filename = filename;}
	EdgeListGraphLoader() {m_filename = "";}
	void SetEdgeListFilename(string filename) {m_filename = filename;}
	bool LoadGraph(Graph& graph, IDictionary* Dictionary, bool bIsDirected = false, bool bHasWeights = false);
};
