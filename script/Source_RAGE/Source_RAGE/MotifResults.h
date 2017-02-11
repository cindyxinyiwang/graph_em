#pragma once
#include "cmsparse/sysTypes.h"
#include "DirectedMotifResults.h"
#include "NodeMotifRolesFinder.h"
#include <string.h>

class MotifResults
{
public:
	// TODO: Switch to a standard name from some article
	enum E_MOTIF_TYPES {PATH3_EDGE = 0,		// One of the edge nodes in a path length 3
					PATH3_INNR,				// One of the inner nodes in a path length 3
					TRIANGLE,				// A triangle or cycle of length 3
					PATH4_EDGE,				// One of the edge nodes in a path length 4
					PATH4_INNR,				// One of the inner nodes in a path length 3
					CYCLE4,					// A cycle of length 4
					TAILED_TGL_TAIL,		// The Tail node of the tailed triangle (the node not on the triangle)
					TAILED_TGL_CNTR,        // The node on the triangle connecting the tail to the triangle (degree = 3)
					TAILED_TGL_REG,			// A node on a tailed triangle with degree 2
					CYC_CHORD_REG,			// A node on a cyc with a chord but not on the chord
					CYC_CHORD_ONCHORD,		// A node on a chord in a cyc with a chord 
					CLAW_CNTR,				// The center node of a Claw\Small tree size 3 
					CLAW_EDGE,				// One of the three edge nodes of a Claw\Small tree size 4 
					CLIQUE4,				// A node on a clique of size 4
					MOTIF_TYPES_NUM		
					};

		static const char* MOTIF_TYPES_NAMES[MOTIF_TYPES_NUM];

		// IDS 0-index is MotifID, 1-index is Role
		static const int URI_MOTIF_TYPES_ID_CONVERTER[MOTIF_TYPES_NUM][2];

private:
	DirectedMotifResults* rawResutls;
	int m_NodeCount;
	uint32 tmpspam;
public:
	inline MotifResults();
	inline MotifResults(int NodeCount);
	inline ~MotifResults();
	inline void ReCreateMotifResults(int NodeCount);
	
	inline void CopyTo(MotifResults& results);
	inline DirectedMotifResults* GetRawRes(){return rawResutls;}
	inline uint32& Result(E_MOTIF_TYPES Motif, int Node);
	inline int GetNodeCount() {return m_NodeCount;}
	inline void ClearMotifResults(E_MOTIF_TYPES Motif);
	inline DirectedMotifResults* GetRawResults(){return rawResutls;};
};

inline void MotifResults::CopyTo(MotifResults& res)
{
	res.m_NodeCount = m_NodeCount;
	if (res.rawResutls != NULL)
		delete res.rawResutls;
	res.rawResutls = new DirectedMotifResults();
	rawResutls->CopyTo(*res.rawResutls);
}

inline void MotifResults::ClearMotifResults(E_MOTIF_TYPES Motif)
{
	if (rawResutls != NULL && m_NodeCount > 0)
		rawResutls->ClearResults();
}

inline uint32& MotifResults::Result(E_MOTIF_TYPES Motif, int Node) 
{
	tmpspam = 10;
	if (Node > m_NodeCount)
	{
		throw "Bad Result Value";
	}
	return *rawResutls->Result(Node, RoleHash::ParseID(URI_MOTIF_TYPES_ID_CONVERTER[(int)Motif][0]),URI_MOTIF_TYPES_ID_CONVERTER[(int)Motif][1]);
}

inline void MotifResults::ReCreateMotifResults(int NodeCount)
{
	if (rawResutls != NULL)
		delete rawResutls;
	
	if (NodeCount == 0)
		rawResutls = NULL;
	else
		rawResutls = new DirectedMotifResults(NodeCount);

	m_NodeCount = NodeCount;
}

inline  MotifResults::MotifResults(int NodeCount)
{

	rawResutls = new DirectedMotifResults(NodeCount);
	m_NodeCount = NodeCount;
}
inline  MotifResults::MotifResults()
{
	rawResutls = NULL;
	m_NodeCount = 0;
}
inline MotifResults::~MotifResults()
{
	if (rawResutls != NULL)
		delete rawResutls;
}