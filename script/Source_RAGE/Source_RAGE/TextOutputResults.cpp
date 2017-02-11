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

#include <fstream>
#include <time.h>
#include "TextOutputResults.h"

TextOutputResults::TextOutputResults(void)
{
}

TextOutputResults::~TextOutputResults(void)
{
}

int TextOutputResults::SaveResults(IOutputParam* params)
{

	if (params == NULL) return -1;

	TextOutputParam* pTextOutputParam = (TextOutputParam*)params;

	string sourcefile = pTextOutputParam->sourcefile; 
	Graph& G = pTextOutputParam->G;
		IDictionary* pDictionary = pTextOutputParam->pDictionary;
		double DrorRuntime = pTextOutputParam->Runtime;
		DirectedMotifResults* pResults = pTextOutputParam->rawResults;

	ofstream RunResultsFile;
	string sFrmttdSrcName = "Results\\RESULTS_" + sourcefile;
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
		
		RunResultsFile << "Result of Motife Approximation Run" << endl;
		RunResultsFile << "----------------------------------" << endl << endl;
		RunResultsFile << "Source Graph: " << sourcefile << endl;
		RunResultsFile << "Connected Node Count: " << G.GetNodeCount() << endl;
		RunResultsFile << "Undirected Edge Count: " << G.GetUndirEdgeCount() << endl;
		RunResultsFile << "Run Date: " << s << endl;
		RunResultsFile << endl;
		RunResultsFile << "Results: " << endl;
		RunResultsFile << "Runtime: " << DrorRuntime << " (sec)" << endl;

		RunResultsFile << "NodeId" << flush; 
		for (int MotifIndex = 0;MotifIndex < DirectedMotifResults::MOTIF_TYPES_NUM; MotifIndex++)
		{
			for (int RoleIndex = 0;RoleIndex < pResults->GetMotifTypeRoleCount(MotifIndex); RoleIndex ++)
				RunResultsFile << "," << pResults->GetMotifID(MotifIndex) << "_" << RoleIndex << flush; 
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