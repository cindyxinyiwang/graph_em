#pragma once
#include <iostream>
#include "NodeMotifRolesFinder.h"
#include "cmsparse\\sysTypes.h"

class DirectedMotifResults
{
private:
	enum EMotifID_Count_Offst_indx{
		E_MOTIF_ID_INDX = 0,
		E_MOTIF_ROLE_COUNT_INDX,
		E_MOTIF_OFFSET_INDX
	};

	
	unsigned int* _NodeResults;
	unsigned int _NodeCount;
public:
static const int _MotifIdOffsets[4096];


	int GetNodeCount(){return _NodeCount;}
	static const int MotifID_Count_Offst[199][3];
	inline int GetMotifID(int MotifTypeIndex);
	inline int GetMotifTypeRoleCount(int MotifTypeIndex);
	static const int MOTIF_ROLE_COUNT;
	static const int MOTIF_TYPES_NUM;
	unsigned int* Result(unsigned int Node, int motifID, unsigned char RoleID) 
	{
		if (Node == 0 || (Node - 1) < _NodeCount || motifID >= 4096) 
			return &_NodeResults[(Node-1)*MOTIF_ROLE_COUNT + _MotifIdOffsets[motifID] + RoleID]; 
		else return NULL;
	};
	unsigned int GetResult(unsigned int Node, int motifID, unsigned char RoleID) 
	{
		unsigned int* res = Result(Node, RoleHash::ParseID(motifID), RoleID);
		return (res != NULL ? *res:-1);
	}

	inline void CopyTo(DirectedMotifResults& results)
	{
		results._NodeCount = _NodeCount; 
		results._NodeResults = new unsigned int[_NodeCount * MOTIF_ROLE_COUNT];
		memcpy(results._NodeResults, _NodeResults, sizeof(unsigned int) * _NodeCount * MOTIF_ROLE_COUNT);

	}
	
	// Return true if the motif index is also undirected
	inline static bool IsAlsoUndirected(int MotifId)
	{
		if (MotifId == 4698   ||  //	"PATH4_EDGE", 
			 MotifId == 13260 ||  //"CYCLE4", 
			 MotifId == 4958  ||  //"TAILED_TGL_CNTR", 
			 MotifId == 13278 ||  //	"CYC_CHORD_REG",
			 MotifId == 4382  ||  //"CLAW_CNTR",
			 MotifId == 31710)   //"CLIQUE4"
		{
			return true;
		}
		else 
			return false;
	}

	inline void ReInit(int Count)
	{
		if (_NodeResults != NULL) 
			delete [] _NodeResults;
		_NodeCount = Count; 
		_NodeResults = new unsigned int[Count * MOTIF_ROLE_COUNT];
		memset(_NodeResults, 0, sizeof(unsigned int) * Count * MOTIF_ROLE_COUNT);
	}

	void ClearResults() {
		if (_NodeCount > 0 && _NodeResults != NULL)	
			memset(_NodeResults, 0, sizeof(unsigned int) * _NodeCount * MOTIF_ROLE_COUNT);
	}
	DirectedMotifResults(void);
	DirectedMotifResults(unsigned int NodeCount );
	~DirectedMotifResults(void);
};

inline int DirectedMotifResults::GetMotifTypeRoleCount(int MotifTypeIndex)
{
	return (MotifTypeIndex >= MOTIF_TYPES_NUM ? 0 : MotifID_Count_Offst[MotifTypeIndex][E_MOTIF_ROLE_COUNT_INDX]);
}

inline int DirectedMotifResults::GetMotifID(int MotifTypeIndex)
{
	return (MotifTypeIndex >= MOTIF_TYPES_NUM ? 0 : MotifID_Count_Offst[MotifTypeIndex][E_MOTIF_ID_INDX]);
}