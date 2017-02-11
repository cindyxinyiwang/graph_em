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

#include "Full4PathAlg.h"
#include <time.h>

Full4PathAlg::Full4PathAlg(void)
{
}

Full4PathAlg::~Full4PathAlg(void)
{
}

void Full4PathAlg::Path(Graph& SearchGraph, 
						MotifResults* pResults,
						bool bIsSymmetric)
{
	int NodeCount = SearchGraph.GetNodeCount();
	GraphIteratorID Iter;
	GraphIteratorID Iter2;
	GraphIteratorID Iter3;

	// Sanity Check
	if (pResults == NULL || pResults->GetNodeCount() != SearchGraph.GetNodeCount())
	  return; // TODO: This is an ERROR, maybe assert here
	
	time_t start,end;

	time (&start);

	// Clear Results
	pResults->ClearMotifResults(MotifResults::PATH3_EDGE);
	pResults->ClearMotifResults(MotifResults::PATH3_INNR);
	pResults->ClearMotifResults(MotifResults::PATH4_EDGE);
	pResults->ClearMotifResults(MotifResults::PATH4_INNR);
	pResults->ClearMotifResults(MotifResults::CYCLE4);

	for (int i1 = 1; i1 <= NodeCount; i1++)
	{
		Iter = SearchGraph.GetNeighborListIterator(i1);
		
		for (int i2 = SearchGraph.GetNextNeighbor(Iter); 
			 i2 != -1; 
			 i2 = SearchGraph.GetNextNeighbor(Iter))
		{
			if (i1==i2) continue;

			Iter2 = SearchGraph.GetNeighborListIterator(i2);
			for (int i3 = SearchGraph.GetNextNeighbor(Iter2); 
			 i3 != -1; 
			 i3 = SearchGraph.GetNextNeighbor(Iter2))
			{
				if (i3==i2 || i3==i1) continue;

				pResults->Result(MotifResults::PATH3_EDGE, i1)++;
				pResults->Result(MotifResults::PATH3_INNR, i2)++;
				pResults->Result(MotifResults::PATH3_EDGE, i3)++;
				
				Iter3 = SearchGraph.GetNeighborListIterator(i3);
				for (int i4 = SearchGraph.GetNextNeighbor(Iter3); 
				 i4 != -1; 
				 i4 = SearchGraph.GetNextNeighbor(Iter3))
				{
					if (i4 != i1 &&
						i4 != i2 &&
						i4 != i3)
					{
						pResults->Result(MotifResults::PATH4_EDGE, i1)++;
						pResults->Result(MotifResults::PATH4_INNR, i2)++;
						pResults->Result(MotifResults::PATH4_INNR, i3)++;
						pResults->Result(MotifResults::PATH4_EDGE, i4)++;

						if (SearchGraph.CheckIfNeigbors(i4, i1))
						{
						pResults->Result(MotifResults::CYCLE4, i1)++;
						pResults->Result(MotifResults::CYCLE4, i2)++;
						pResults->Result(MotifResults::CYCLE4, i3)++;
						pResults->Result(MotifResults::CYCLE4, i4)++;
						}
					}
				}
				SearchGraph.ClearNeighborListIterator(Iter3);
			}
			SearchGraph.ClearNeighborListIterator(Iter2);
		}
		SearchGraph.ClearNeighborListIterator(Iter);
	}


	// Devide the cycle results by 4 ( each cycle is counted 4 times)
	// 8 times if we are symmetric
	int CycleDivCount = 4;
	if (bIsSymmetric) 
		CycleDivCount = 8;

	for (int i1 = 1; i1 <= NodeCount; i1++)
	{
		pResults->Result(MotifResults::CYCLE4, i1) /= CycleDivCount;
	}


	// TODO : if graph is symmetric then we should devide the path results by 2
	if (bIsSymmetric) 
	{
		for (int i1 = 1; i1 <= NodeCount; i1++)
		{
			pResults->Result(MotifResults::PATH4_EDGE, i1) /= 2;
			pResults->Result(MotifResults::PATH4_INNR, i1) /= 2;
			pResults->Result(MotifResults::PATH3_EDGE, i1) /= 2;
			pResults->Result(MotifResults::PATH3_INNR, i1) /= 2;
		}
	}

	time (&end);
	cout << "Finished Real Path time: (" << difftime (end,start) << ") "<< flush;
}
