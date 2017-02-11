#include <fstream>
#include "CSVOutputResults.h"
#include "assert.h"

CSVOutputResults::CSVOutputResults(void)
{
}

CSVOutputResults::~CSVOutputResults(void)
{
}
int CSVOutputResults::SaveResults(IOutputParam* params)
{
	if (params == NULL) return -1;
	IDictionary* pDictionary = params->pDictionary;
	DirectedMotifResults* pResults = params->rawResults;

	ofstream RunResultsFile;
	char Filename[255];
	SetFileType(params->SaveFilename.c_str(), Filename, 254, ".csv");
	string sFrmttdSrcName = Filename;
	
	RunResultsFile.open(sFrmttdSrcName.c_str());

	if (pDictionary == NULL)
	{
		// TODO Maybe use a stub dictionary that does nothing if we recieved null.... 
		return -2;
	}

	if (RunResultsFile.is_open())
	{
		RunResultsFile << "NodeID"  << flush; 
		for (int MotifIndex = 0;MotifIndex < DirectedMotifResults::MOTIF_TYPES_NUM; MotifIndex++)
		{
			// Check if we should ignore directed motifs
			if (params->outUndir && !DirectedMotifResults::IsAlsoUndirected(pResults->GetMotifID(MotifIndex)))
				continue;

			for (int RoleIndex = 0;RoleIndex < pResults->GetMotifTypeRoleCount(MotifIndex); RoleIndex ++)
				RunResultsFile << "," << pResults->GetMotifID(MotifIndex) << "_" << RoleIndex 
							   << flush;
		}
		RunResultsFile << ",ASType" << flush; 
		RunResultsFile << endl;

		ASTypesMap::iterator ASIterator;
		int ASNum;
		for (int i = 1; i<= pResults->GetNodeCount(); i++)
		{
			ASNum = atoi(pDictionary->_GetStringValue(i).c_str());
			assert(ASNum != 0);

			RunResultsFile << ASNum << flush; 
			for (int MotifIndex = 0;MotifIndex < DirectedMotifResults::MOTIF_TYPES_NUM; MotifIndex++)
			{
				// Check if we should ignore directed motifs
				if (params->outUndir && !DirectedMotifResults::IsAlsoUndirected(pResults->GetMotifID(MotifIndex)))
					continue;

				for (int RoleIndex = 0;RoleIndex < pResults->GetMotifTypeRoleCount(MotifIndex); RoleIndex ++)
					RunResultsFile << "," << pResults->GetResult(i, pResults->GetMotifID(MotifIndex), RoleIndex) << flush;
			}

			ASIterator = params->pAsTypes->find(ASNum);
			if (ASIterator == params->pAsTypes->end())
			{
				RunResultsFile << ",?" << flush;
			}
			else
			{
				RunResultsFile << "," << ASIterator->second << flush;
			}
			RunResultsFile << endl;
		}
	}

	RunResultsFile.close();
	return 0;
}