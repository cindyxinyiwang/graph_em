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
 * THIS SOFTWARE IS PROVIDED BY Dror Marcus ``AS IS'' AND ANY EXPRESS OR IMPLIED
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

#include <fstream>
#include <time.h>
#include "ArffOutputResults.h"
#include "assert.h"

ArffOutputResults::ArffOutputResults(void)
{
}

ArffOutputResults::~ArffOutputResults(void)
{
}


int ArffOutputResults::SaveResults(IOutputParam* params)
{
	if (params == NULL) return -1;
	ArffOutputParam* pArffOutputParam = (ArffOutputParam*)params;

	IDictionary* pDictionary = pArffOutputParam->pDictionary;
	DirectedMotifResults* pResults = pArffOutputParam->rawResults;

	ofstream RunResultsFile;
	char Filename[255];
	SetFileType(params->SaveFilename.c_str(), Filename, 254, ".arff");
	string sFrmttdSrcName = Filename;

	RunResultsFile.open(sFrmttdSrcName.c_str());

	if (pDictionary == NULL)
	{
		// TODO Maybe use a stub dictionary that does nothing if we recieved null.... 
		return -2;
	}

	if (RunResultsFile.is_open())
	{
		const int MAXLEN = 80;
		char s[MAXLEN];
		struct tm newtime;
		time_t t = time(0);
		localtime_s(&newtime, &t);
		strftime(s, MAXLEN, "%m/%d/%Y", &newtime);

		RunResultsFile << "% Per node motif count results" << endl << "%" << endl; 
		RunResultsFile << "% Tested Graph: " << pArffOutputParam->SourceFile << endl; 
		RunResultsFile << "% Run Date/Time: " << s << endl << "%" << endl << endl;  

		
		RunResultsFile << "@RELATION motif_count" << endl <<endl; 

		RunResultsFile << "@ATTRIBUTE NodeID NUMERIC"  << endl; 


		for (int MotifIndex = 0;MotifIndex < DirectedMotifResults::MOTIF_TYPES_NUM; MotifIndex++)
		{
			// Check if we should ignore directed motifs
			if (params->outUndir && !DirectedMotifResults::IsAlsoUndirected(pResults->GetMotifID(MotifIndex)))
				continue;

			for (int RoleIndex = 0;RoleIndex < pResults->GetMotifTypeRoleCount(MotifIndex); RoleIndex ++)
				RunResultsFile << "@ATTRIBUTE " << pResults->GetMotifID(MotifIndex) << "_" << RoleIndex 
								<< " NUMERIC" << endl;
		}
		

		RunResultsFile << "@ATTRIBUTE ASType {1, 2, 3, 4}"  << endl; 
		RunResultsFile << endl << "@DATA" << endl;

		ASTypesMap::iterator ASIterator;
		int ASNum;
		for (int i = 1; i<= pResults->GetNodeCount(); i++)
		{
			ASNum = atoi(pDictionary->_GetStringValue(i).c_str());
			//assert(ASNum != 0);

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