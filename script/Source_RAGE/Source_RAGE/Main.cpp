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

#include <iostream>
#include <fstream>
#include <time.h>
#include <string.h>
#include "cmsparse\mprdefs.h" 
#include "cmsparse\isramatrix.h" 
#include "IIntDictionary.h" 
#include "EdgeListGraphLoader.h" 
#include "Graph.h"
#include "UndirectedRAGECount.h"
#include "Full4PathAlg.h"
#include "MotifResults.h"
#include "CSVOutputResults.h"
#include "ArffOutputResults.h"
#include "BIUDirectedMotifCount.h"
#include "NodeMotifRolesFinder.h"
#include "DirectedMotifResults.h"
#include "DimesClassificationGraphLoader.h"
#include "GraphRandomizeTester.h"
#include "CaidaASNGraphLoader.h"
//#include "DictionaryHashList.h"

#include <direct.h>

#define RANDOM_GRAPH_COUNTS 15
#define GRAPH_NOISE_PRESENTAGE 0.10

// -------
#include <sstream>

// ======

using namespace std;
int CountFullEnumerate(char * file1, MotifResults* pResults);

#define MAKE_DIR(dir) _mkdir(dir)

void NormalizeResults(MotifResults* pResults)
{
	if (pResults == NULL)
		return;

	// Go over all nodes to normalize. 
	for (int i = 1; i<= pResults->GetNodeCount(); i++)
	{
		// The order for the normalization should not be changed!!!!!!!!!!!
		// This order is based on dependencies 

		// -----------------------------
		// No need to normalize Cliques
		// -----------------------------
		
		// ---------------
		// Normalize Chord
		// ---------------
		
		// CYC_CHORD_ONCHORD
		pResults->Result(MotifResults::CYC_CHORD_ONCHORD, i) -= 3*pResults->Result(MotifResults::CLIQUE4, i);

		// CYC_CHORD_REG
		pResults->Result(MotifResults::CYC_CHORD_REG, i) -= 3*pResults->Result(MotifResults::CLIQUE4, i);

		// ---------------
		// Normalize CYCLE
		// ---------------
		// CYCLE
		pResults->Result(MotifResults::CYCLE4, i) -= (3*pResults->Result(MotifResults::CLIQUE4, i) + 
													  pResults->Result(MotifResults::CYC_CHORD_ONCHORD, i)+
													  pResults->Result(MotifResults::CYC_CHORD_REG, i));

		// -------------------------
		// Normalize TAILED_TRIANGLE
		// -------------------------
		// TAILED_TGL_TAIL
		pResults->Result(MotifResults::TAILED_TGL_TAIL, i) -= (3*pResults->Result(MotifResults::CLIQUE4, i)+
													  2*pResults->Result(MotifResults::CYC_CHORD_REG, i));
		// TAILED_TGL_CNTR
		pResults->Result(MotifResults::TAILED_TGL_CNTR, i) -= (3*pResults->Result(MotifResults::CLIQUE4, i) + 
													  2*pResults->Result(MotifResults::CYC_CHORD_ONCHORD, i));

		// TAILED_TGL_REG
		pResults->Result(MotifResults::TAILED_TGL_REG, i) -= (6*pResults->Result(MotifResults::CLIQUE4, i) + 
													  2*pResults->Result(MotifResults::CYC_CHORD_ONCHORD, i)+
													  2*pResults->Result(MotifResults::CYC_CHORD_REG, i));

		// --------------
		// Normalize CLAW
		// --------------
		// CLAW_CNTR
		pResults->Result(MotifResults::CLAW_CNTR, i) -= (pResults->Result(MotifResults::CLIQUE4, i) + 
													  pResults->Result(MotifResults::CYC_CHORD_ONCHORD, i)+
													  pResults->Result(MotifResults::TAILED_TGL_CNTR, i));
		// CLAW_EDGE
		pResults->Result(MotifResults::CLAW_EDGE, i) -= (3*pResults->Result(MotifResults::CLIQUE4, i) + 
													  pResults->Result(MotifResults::CYC_CHORD_ONCHORD, i)+
													  2*pResults->Result(MotifResults::CYC_CHORD_REG, i) +
													  pResults->Result(MotifResults::TAILED_TGL_TAIL, i) +
													  pResults->Result(MotifResults::TAILED_TGL_REG, i));


		// ---------------
		// Normalize PATH4
		// ---------------
		// PATH4_EDGE
		pResults->Result(MotifResults::PATH4_EDGE, i) -= (6*pResults->Result(MotifResults::CLIQUE4, i) + 
													  2*pResults->Result(MotifResults::CYC_CHORD_ONCHORD, i)+
													  4*pResults->Result(MotifResults::CYC_CHORD_REG, i) +
													  2*pResults->Result(MotifResults::CYCLE4, i) +
													  2*pResults->Result(MotifResults::TAILED_TGL_TAIL, i) +
													  pResults->Result(MotifResults::TAILED_TGL_REG, i));

		// PATH4_INNR
		pResults->Result(MotifResults::PATH4_INNR, i) -= (6*pResults->Result(MotifResults::CLIQUE4, i) + 
													  4*pResults->Result(MotifResults::CYC_CHORD_ONCHORD, i)+
													  2*pResults->Result(MotifResults::CYC_CHORD_REG, i) +
													  2*pResults->Result(MotifResults::CYCLE4, i) +
													  2*pResults->Result(MotifResults::TAILED_TGL_CNTR, i) +
													  pResults->Result(MotifResults::TAILED_TGL_REG, i));

		// ------------------------------
		// No need to normalize TRIANGLES
		// ------------------------------

		// -----
		// PATH3
		// -----
		// PATH3_EDGE
		pResults->Result(MotifResults::PATH3_EDGE, i) -= 2*pResults->Result(MotifResults::TRIANGLE, i);

		// PATH3_INNER
		pResults->Result(MotifResults::PATH3_INNR, i) -= pResults->Result(MotifResults::TRIANGLE, i);

	}// End all nodes for loop

}

void LoadNodeTypes(map<int, string>* NodeTypes)
{
	ifstream TypesFile;
	TypesFile.open("NodeTypesList.txt");
	int Nodenum = 0;
	string NodeShortName;
	string NodeType;
	if (!TypesFile.is_open())
	{
		return;
	}
	while (!TypesFile.eof())
	{
		TypesFile >> Nodenum;
		TypesFile >> NodeShortName;
		TypesFile >> NodeType;
		NodeTypes->insert(pair<int,string>(Nodenum, NodeType));
	}
	TypesFile.close();
}

void CheckDiff(string sourcefile, 
				    Graph& G, 
					IDictionary* pDictionary,
					double RunAtime,
					double RunBtime,
					DirectedMotifResults* pResultsA,
					DirectedMotifResults* pResultsB,
					map<int,string>* ASTypes)
{

	ofstream RunResultsFile;
	MAKE_DIR("Results");
	string sFrmttdSrcName = "Results\\DiffLOG.txt";
	RunResultsFile.open(sFrmttdSrcName.c_str());
	int ASNum;

	if (RunResultsFile.is_open())
	{

		for (int i = 1; i<= pResultsA->GetNodeCount(); i++)
		{
			ASNum = atoi(pDictionary->_GetStringValue(i).c_str());
			assert(ASNum != 0);
 
			for (int MotifIndex = 0;MotifIndex < DirectedMotifResults::MOTIF_TYPES_NUM; MotifIndex++)
			{
				for (int RoleIndex = 0;RoleIndex < pResultsA->GetMotifTypeRoleCount(MotifIndex); RoleIndex ++)
				{
					if (pResultsA->GetResult(i, pResultsA->GetMotifID(MotifIndex), RoleIndex) !=
						pResultsB->GetResult(i, pResultsB->GetMotifID(MotifIndex), RoleIndex) && 
						pResultsB->GetMotifID(MotifIndex) != 4740 &&
						pResultsB->GetMotifID(MotifIndex) != 4742 &&
						pResultsB->GetMotifID(MotifIndex) != 4748)
					{
						RunResultsFile  << "DIFF (node-AS#, id, role) A:B - " 
										<< "(" << i << "-" << ASNum << ", "<< pResultsA->GetMotifID(MotifIndex) << ", "<< RoleIndex << ") "
										<< pResultsA->GetResult(i, pResultsA->GetMotifID(MotifIndex), RoleIndex) << " : " 
										<< pResultsB->GetResult(i, pResultsB->GetMotifID(MotifIndex), RoleIndex) 
										<< endl;
					}
				}
					
			}
			
		}
	}

	RunResultsFile.close();
}
void PrintUNDirectedResults(string sourcefile, 
				    Graph& G, 
					IDictionary* pDictionary,
					double ExhaustiveEnumarate,
					double DrorRuntime,
					bool bUndirectedStyleOut,
					MotifResults* pResults,
					map<int,string>* ASTypes)
{
	ofstream RunResultsFile;
	
	size_t found=sourcefile.find('\\');
	while (found!=string::npos)
	{
		sourcefile.replace(found,1,"_");
		found=sourcefile.find('\\');
	}
	MAKE_DIR("Results");

	string sFrmttdSrcName = "Results\\UNDIR_RESULTS_" + sourcefile;
	

	CSVOutputParam param;
	param.SaveFilename = sFrmttdSrcName;
	param.pDictionary = pDictionary;
	param.rawResults = pResults->GetRawResults();
	param.pAsTypes = ASTypes;
	param.outUndir = bUndirectedStyleOut;
	CSVOutputResults res;
	res.SaveResults(&param);

	ArffOutputParam Arffparam;
	Arffparam.SaveFilename = sFrmttdSrcName;
	Arffparam.pDictionary = pDictionary;
	Arffparam.rawResults = pResults->GetRawResults();
	Arffparam.SourceFile = sourcefile;
	Arffparam.pAsTypes = ASTypes;
	Arffparam.outUndir = bUndirectedStyleOut;
	ArffOutputResults resArff;
	resArff.SaveResults(&Arffparam);

	return;
}

void PrintDirectedResults(string sourcefile, 
				    Graph& G, 
					IDictionary* pDictionary,
					double ExhaustiveEnumarate,
					double DrorRuntime,
					DirectedMotifResults* pResults,
					map<int,string>* ASTypes)
{
	ofstream RunResultsFile;
	
	size_t found=sourcefile.find('\\');
	while (found!=string::npos)
	{
		sourcefile.replace(found,1,"_");
		found=sourcefile.find('\\');
	}
	MAKE_DIR("Results");
	string sFrmttdSrcName = "Results\\DIR_RESULTS_" + sourcefile;
	

	CSVOutputParam param;
	param.SaveFilename = sFrmttdSrcName;
	param.pDictionary = pDictionary;
	param.rawResults = pResults;
	param.pAsTypes = ASTypes;
	CSVOutputResults res;
	res.SaveResults(&param);

	ArffOutputParam Arffparam;
	Arffparam.SaveFilename = sFrmttdSrcName;
	Arffparam.pDictionary = pDictionary;
	Arffparam.rawResults = pResults;
	Arffparam.SourceFile = sourcefile;
	Arffparam.pAsTypes = ASTypes;
	ArffOutputResults resArff;
	resArff.SaveResults(&Arffparam);

	return;
}

int RunUndirectedAlgorithm(int argc, char* argv[])
{
	map<int, string> ASTypes;
	LoadNodeTypes(&ASTypes);
	srand ( time(NULL) );
	char szfilename[250];
	string srcFilename = "SMALL_TEST.txt";

	bool hasWeight = false;
	char* tst;
	char* sfilename;
	if (argc > 1)
	{
		sfilename = argv[1];
	}
	else
	{
		printf("Filename: ");
		scanf("%s", szfilename);
		sfilename = szfilename;
	}
	
	bool bUndirectedStyleOut = true;
	if (argc > 3 && strncmp(argv[3],"OUTSTYLE_DIR", 11) == 0 )
	{
		bUndirectedStyleOut = false;
	}

	// Use _txt for weighted cases
	tst = strstr(sfilename, "_txt");
	if (tst != NULL)
	{
		tst[0] = '.';
		hasWeight = true;
	}
	srcFilename = sfilename;
	Graph g;
	EdgeListGraphLoader Loader;
	IIntDictionary Dictionary;

	Loader.SetEdgeListFilename(srcFilename);
	if (Loader.LoadGraph(g, &Dictionary, false, hasWeight) == false)
	{
		cout << "ERROR: Could not load Graph [" << srcFilename << "]!"<< endl << "Exiting program..." << endl;
		return 0;
	}

	MotifResults results(g.GetNodeCount());
	MotifResults normalizedResults;

	cout << endl << endl<<"Graph [" << srcFilename << "] loaded"<< endl;

	time_t start_dror,end_dror;


	time (&start_dror);
	UndirectedRAGECount::CountGraphlets(g, &results, true);
	time (&end_dror);	
	cout << "Finished dror time: (" << difftime (end_dror,start_dror) << ") "<< endl;
	results.CopyTo(normalizedResults);
	NormalizeResults(&normalizedResults);
		
	cout << "Saving..." << endl; 
	time_t start_Exhaustive =0;
	time_t end_Exhaustive = 0;

	PrintUNDirectedResults(srcFilename, g, &Dictionary,
				difftime (end_Exhaustive,start_Exhaustive), 
				difftime (end_dror,start_dror), 
				bUndirectedStyleOut,
				&normalizedResults,
				&ASTypes);

	cout<<endl<<endl<<endl<<"------------------"<<endl<<"OK-OK-OK"<<endl<<endl<<endl;
	return 0;
}

// These algorithms for directed motifs should be reavaluated and verified for correctness.
// See more information on the algorithms at: R. Itzhack, Y. Mogilevski, Y. Louzoun, An 
// optimal algorithm for counting network motifs, Physica A 381 (2007) 482{490.
int RunDirectedAlgorithm(int argc, char* argv[])
{
		map<int, string> ASTypes;
	LoadNodeTypes(&ASTypes);
	srand ( time(NULL) );
	char szfilename[250];
	string srcFilename = "SMALL_TEST.txt";
		//"asedges_2009_week_30_Compare.txt";
	//"BorkTest.txt";
	//"asedges_2009_week_30.txt";
	//asedges_2009_month_8
	bool hasWeight = false;
	char* tst;
	char* sfilename;
	if (argc > 1)
	{
		sfilename = argv[1];
	}
	else
	{
		printf("Filename: ");
		scanf("%s", szfilename);
		sfilename = szfilename;
	}
	// Use _txt for weighted cases
	tst = strstr(sfilename, "_txt");
	if (tst != NULL)
	{
		tst[0] = '.';
		hasWeight = true;
	}
	srcFilename = sfilename;
	Graph g;
	EdgeListGraphLoader Loader;
	IIntDictionary Dictionary;

	Loader.SetEdgeListFilename(srcFilename);
	if (Loader.LoadGraph(g, &Dictionary, true, hasWeight) == false)
	{
		cout << "ERROR: Could not load Graph [" << srcFilename << "]!"<< endl << "Exiting program..." << endl;
		return 0;
	}
	cout << endl << endl<<"Graph [" << srcFilename << "] loaded"<< endl;

	time_t start_Exhaustive,end_Exhaustive;
	time_t start_dror,end_dror;

	DirectedMotifResults* dresults = new DirectedMotifResults(g.GetNodeCount());
	BIUDirectedMotifCount Alg;
	Alg.MotifCount(g, dresults, false);

	start_Exhaustive =0;
	end_Exhaustive = 0;
	start_dror =0;
	end_dror=0;

	cout << "Saving..." << endl; 

	PrintDirectedResults(srcFilename, g, &Dictionary,
				difftime (end_Exhaustive,start_Exhaustive), 
				difftime (end_dror,start_dror), 
				dresults,
				&ASTypes);

	delete dresults;
	cout<<endl<<endl<<endl<<"------------------"<<endl<<"OK-OK-OK"<<endl<<endl<<endl;
	return 0;
}

int RunCaidaASNDirectedAlgorithm(int argc, char* argv[])
{
	map<int, string> ASTypes;
	LoadNodeTypes(&ASTypes);
	srand ( time(NULL) );
	char szfilename[250];
	string srcFilename = "SMALL_TEST.txt";

	bool hasWeight = false;
	char* tst;
	char* sfilename;
	if (argc > 1)
	{
		sfilename = argv[1];
	}
	else
	{
		printf("Filename: ");
		scanf("%s", szfilename);
		sfilename = szfilename;
	}
	// Use _txt for weighted cases
	tst = strstr(sfilename, "_txt");
	if (tst != NULL)
	{
		tst[0] = '.';
		hasWeight = true;
	}
	srcFilename = sfilename;
	Graph g;
	CaidaASNGraphLoader Loader;
	IIntDictionary Dictionary;

	Loader.SetEdgeListFilename(srcFilename);
	if (Loader.LoadGraph(g, &Dictionary, hasWeight) == false)
	{
		cout << "ERROR: Could not load Graph [" << srcFilename << "]!"<< endl << "Exiting program..." << endl;
		return 0;
	}
	cout << endl << endl<<"Graph [" << srcFilename << "] loaded"<< endl;

	time_t start_Exhaustive,end_Exhaustive;
	time_t start_dror,end_dror;

	DirectedMotifResults* dresults = new DirectedMotifResults(g.GetNodeCount());
	BIUDirectedMotifCount Alg;
	Alg.MotifCount(g, dresults, false);

	start_Exhaustive =0;
	end_Exhaustive = 0;
	start_dror =0;
	end_dror=0;

	cout << "Saving..." << endl; 

	PrintDirectedResults(srcFilename, g, &Dictionary,
				difftime (end_Exhaustive,start_Exhaustive), 
				difftime (end_dror,start_dror), 
				dresults,
				&ASTypes);
	delete dresults;
	cout<<endl<<endl<<endl<<"------------------"<<endl<<"OK-OK-OK"<<endl<<endl<<endl;
	return 0;
}

int main( int argc, char* argv[] )
{
	if (argc > 2 && strncmp(argv[2],"DIRECTED", 11) == 0 )
	{
		// These algorithms for directed motifs should be reavaluated and verified for correctness.
		// See more information on the algorithms at: R. Itzhack, Y. Mogilevski, Y. Louzoun, An 
		// optimal algorithm for counting network motifs, Physica A 381 (2007) 482{490.
		cout << "Computing directed Algorithm" << endl;
		RunDirectedAlgorithm(argc, argv);
	}
	else if (argc > 2 && strncmp(argv[2],"ASN", 11) == 0 )
	{
		cout << "Computing CAIDA GRAPH Undirected Algorithm" << endl;
		RunCaidaASNDirectedAlgorithm(argc, argv);
	}
	else
	{
		cout << "Computing Undirected Algorithm" << endl;
		RunUndirectedAlgorithm(argc, argv);
	}
	return 0;
}
