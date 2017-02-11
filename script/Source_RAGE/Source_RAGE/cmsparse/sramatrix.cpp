/** 
 * Copyright 2009 Tomer Tankel. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY TOMER TANKEL ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL TOMER TANKEL OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those of the
 * authors and should not be interpreted as representing official policies, either expressed
 * or implied, of Tomer Tankel.
 */

#include <string.h> // memcpy
#include <iostream>
#include <sstream>
#include "sramatrix.h"

//#define ORDERED_ADJACENCY_LISTS
#define LOG4CXX_ERROR

extern "C" ulong32 hash2(
register ulong32 *key,
register ulong32 keysize,
register ulong32 initval);

inline void SRAMatrixElement::Invalidate(const int *nDeadInds)
{
	m_nRow= nDeadInds[SDRow];
	m_nCol= nDeadInds[SDCol];
	m_pNext[SDCol]= 0;
	m_pNext[SDRow]= 0;
	MH_ASSERT(!*this);
}

void SRAMatrixElement::Init(int nRow,int nCol)
{
	m_nRow= nRow;
	m_nCol= nCol;
	m_pNext[SDCol]= this;
	m_pNext[SDRow]= this;
}

/*log4cxx::LoggerPtr ISRAMatrix::log= log4cxx::Logger::getLogger("ISRAMatrix");*/

ISRAMatrix::CreateParams::CreateParams(bool bIsSymmetric)
{
	SetFlag(CFSymmetric,bIsSymmetric);
}

ISRAMatrix::ContentStats::ContentStats()
{
	Reset();
}
void ISRAMatrix::ContentStats::Reset(const CreateParams * pCreatPars/*=0*/)
{
	int nDir;
	m_u32NofElements= 0;
	m_u32NofMarkedRemoved= 0;
	for (nDir=SDCol; nDir<=SDRow; nDir++)
	{
		m_u32MaxHead[nDir]=0;
		m_u32Added[nDir]=0;
	}
	m_u64MaintOps= 0;
	if (pCreatPars)
		m_bIsSymmetric= pCreatPars->IsSymmetric();
	else
		m_bIsSymmetric= false;
}

inline void ISRAMatrix::ContentStats::AddHeading(SRADirection eDir,ulong32 u32HeadInd)
{
	if (m_u32MaxHead[eDir]<u32HeadInd)
		m_u32MaxHead[eDir]= u32HeadInd;
	m_u32Added[eDir]++;
	MH_ASSERT(m_u32Added[eDir]<=m_u32MaxHead[eDir]);
}

inline void ISRAMatrix::ContentStats::AddElement()
{
	m_u32NofElements ++;
}
inline void ISRAMatrix::ContentStats::RemoveHeading(SRADirection eDir)
{
	MH_ASSERT(m_u32Added[eDir]>0);
	m_u32Added[eDir] --;
	m_u32NofMarkedRemoved ++;
}
inline void ISRAMatrix::ContentStats::RemoveElement()
{
	m_u32NofElements --;
	m_u32NofMarkedRemoved ++;
}
inline void ISRAMatrix::ContentStats::ResetMarkedRemoved()
{
	m_u32NofMarkedRemoved= 0;
}
inline void ISRAMatrix::ContentStats::MarkRemoved()
{
	m_u32NofMarkedRemoved ++;
}

inline void ISRAMatrix::ContentStats::AddMaintOp()
{
	m_u64MaintOps ++;
}

std::ostream & operator <<(std::ostream & os, const ISRAMatrix::ContentStats & stats)
{
	os << "Non-Zero Elements " << stats.m_u32NofElements
		<< " Marked Removed " << stats.m_u32NofMarkedRemoved
		<< " Max Head rows " << stats.m_u32MaxHead[SDRow] << " cols " << stats.m_u32MaxHead[SDCol]
		<< " Added rows " << stats.m_u32Added[SDRow] << " cols " << stats.m_u32Added[SDCol];
	return os;
}

SRAMatrixElement ISRAMatrix::m_xNullElement;
SRAMatrixElement * ISRAMatrix::m_pNullElement= 0;
const int ISRAMatrix::m_nHeadingsRow[NOFSRADirections]= {
	0,// SDCol
	-1 // SDRow
};
ulong32 ISRAMatrix::m_gu32WastedMemQuota= 1<<20; // 1M elements= 20MB

inline void ISRAMatrix::MemCpy(void *pDst,const void *pSrc,size_t uSize)
{
	if (uSize==2)
	{
		*(ushort16*)pDst= *(const ushort16*)pSrc;
		return;
	}
	if (uSize==4)
	{
		*(ulong32*)pDst= *(const ulong32*)pSrc;
		return;
	}
	memcpy(pDst,pSrc,uSize);
}

inline bool ISRAMatrix::IsFresh(SRAMatrixElement & rElement) const
{
	if (!m_pnRecycledDeadInds)
		return rElement.IsFresh(m_nDeadInds);
	if (!rElement.IsFresh(m_pnRecycledDeadInds))
		return false;
	else if (!rElement.IsFresh(m_nDeadInds))
	{
		rElement.Invalidate(m_pnRecycledDeadInds);
		m_pContentStats->MarkRemoved();
		return false;
	}
	else
		return true;
}
ISRAMatrix::ISRAMatrix() :
m_pContentStats(new ContentStats)
{
	_Create();
}
ISRAMatrix::ISRAMatrix(const ISRAMatrix &src) :
m_pContentStats(new ContentStats)
{
	_Create();
}


ISRAMatrix::~ISRAMatrix()
{
	PrivateClear();
	delete m_pContentStats;
}

void ISRAMatrix::_Create()
{
	m_pnRecycledDeadInds= 0;
	m_uValueSize=0;
	m_pu8DfltInitial= 0;
	m_uElementSize= sizeof(SRAMatrixElement);
	m_u32HashTableOrder= 0;
	m_pElementHashTable=0;
	m_xTopLeftCorner= m_xNullElement;
}

void ISRAMatrix::SetDfltInitial(const byte *pu8DfltInitial,size_t uSize)
{
	MH_ASSERT(pu8DfltInitial);
	MH_ASSERT(!*this);
	m_uValueSize= uSize;
	m_pu8DfltInitial= pu8DfltInitial;
	m_uElementSize= sizeof(SRAMatrixElement)+uSize;
}
void ISRAMatrix::IsDerivedInitVal(const void *)
{
}

void ISRAMatrix::Init(ulong32 u32MaxNofElements,const CreateParams & rCreatePars)
{
	PrivateClear();
	if (!u32MaxNofElements)
		return;
	ulong32 u32Order=0;
	uint64	u64TableSize;
	while (u32MaxNofElements > (1U<<u32Order))
		u32Order++;
	//u32Order++;
	m_xCreatePars= rCreatePars;
	m_pContentStats->Reset(&rCreatePars);
	m_u32HashTableOrder= u32Order;
	//m_u32HashTableMask= ~((~0)<<u32Order);
	m_u32MaxNofElements= (1<<u32Order);
	if ((m_u32MaxNofElements- u32MaxNofElements)< m_gu32WastedMemQuota)
	{
	}
	else
	{
		m_u32HashTableOrder= 0;
		if (u32Order >10)
			m_u32HashTableOrder= u32Order-10;
		ulong32 u32NofElements2OrderRatio;
		u32NofElements2OrderRatio= u32MaxNofElements/(1<<m_u32HashTableOrder);
		// search for nearest larger or eqal prime number
		bool bPrime= false;
		while (!bPrime)
		{
			ulong32 u32Div;

			u32NofElements2OrderRatio++;
			bPrime= true;
			for (u32Div=2; u32Div < u32NofElements2OrderRatio; u32Div++)
			{
				if (u32NofElements2OrderRatio % u32Div==0)
				{
					bPrime= false;
					break;
				}
			}
		}
		m_u32MaxNofElements= u32NofElements2OrderRatio <<m_u32HashTableOrder;
	}

	u64TableSize= m_u32MaxNofElements*m_uElementSize;
	m_pElementHashTable= new byte[u64TableSize];
	MH_ASSERT(m_pElementHashTable);
	if (!m_pElementHashTable)
		throw 0;
	for (uint64 u64ElmInd=0; u64ElmInd<m_u32MaxNofElements; u64ElmInd++)
	{
		SRAMatrixElement *pElement= reinterpret_cast<SRAMatrixElement*>
			(m_pElementHashTable+(u64ElmInd*m_uElementSize));
		pElement->Invalidate(m_nDeadInds);
		memset(pElement+1,'\0',m_uValueSize);
	}
	m_xTopLeftCorner= m_xNullElement;
	RecycleGarbage(true);
}

ISRAMatrix & ISRAMatrix::operator =(const ISRAMatrix &rhs)
{
	if (this ==&rhs)
		return *this;
	// Initialize same size matrix
	Init(rhs.GetMaxNofElements(),rhs.GetCreateParams());
	// copy elements
	iterator xIter;
	int nRow, nCol;
	const SRAMatrixElement *pCurVal;
	for (const_cast<ISRAMatrix&>(rhs).restartIterator(xIter);
		(pCurVal=rhs.protGetCurrent(xIter));rhs.advanceIterator(xIter))
	{
		rhs.getCurrentInds(xIter,nRow,nCol);
		SRAMatrixElement & rNewVal= findOrAddElement(nRow,nCol);
		MemCpy((&rNewVal)+1,pCurVal+1,m_uValueSize);
	}

	return *this;
}

void ISRAMatrix::PrivateClear()
{
	if (m_pElementHashTable)
	{
		m_u32HashTableOrder= 0;
		delete [] m_pElementHashTable;
		m_pElementHashTable= 0;
	}
	m_pContentStats->Reset(&m_xCreatePars);
	m_nDeadInds[SDRow]= SRAMatrixElement::eMinLiveRowInd-1;
	m_nDeadInds[SDCol]= 0; // wrap arround after ULONG_MAX cycles
}

#ifndef ORDERED_ADJACENCY_LISTS

inline void ISRAMatrix::AppendAtTail(SRAMatrixElement &rListHead,SRAMatrixElement &rElm,SRADirection eSDir)
{
	SRAMatrixElement *pTail;
	pTail= rListHead.m_pNext[eSDir];
	rListHead.m_pNext[eSDir]= &rElm;
	rElm.m_pNext[eSDir]= pTail;
}

#else

void ISRAMatrix::InsertOrdered(SRAMatrixElement &rListHead,SRAMatrixElement &rElm,SRADirection eSDir)
{
	int nSortIndDir= 1-eSDir;
	int nSortInd;
	if (nSortIndDir==SDRow)
		nSortInd= rElm.Row();
	else
		nSortInd= rElm.Column();
	SRAMatrixElement **ppNext, *pNext;
	for (ppNext= &rListHead.m_pNext[eSDir];pNext=*ppNext;
		ppNext= &pNext->m_pNext[eSDir])
	{
		if ((nSortIndDir==SDRow && pNext->Row() > nSortInd) ||
			(nSortIndDir==SDCol && pNext->Column() > nSortInd))
			break;
	}
	rElm.m_pNext[eSDir]= pNext;
	*ppNext= &rElm;
}
#endif // ORDERED_ADJACENCY_LISTS

SRAMatrixElement & ISRAMatrix::findHeadingElement(SRADirection eSDir,int nHeadingInd) const
{
	register int nHeadingRow= m_nHeadingsRow[eSDir];
	ulong32 u32DirectVal;
	u32DirectVal= ((nHeadingInd << 1) + eSDir) % m_u32MaxNofElements;
	SRAMatrixElement* pElement;
	pElement= reinterpret_cast<SRAMatrixElement*>
		(m_pElementHashTable+(u32DirectVal*m_uElementSize));
	if ((pElement->Row()==nHeadingRow && pElement->Column()== nHeadingInd) || IsFresh(*pElement))
		return *pElement;

	SRAMatrixElement & rHashedElement= findElement(nHeadingRow,nHeadingInd);
	if (rHashedElement || *pElement)
		pElement= &rHashedElement;
	return *pElement;
}

SRAMatrixElement & ISRAMatrix::findElement(int & row,int & col, bool AddNewLocation) const
{
	static const ulong32 u32Initval= 0;
	ulong32 u32Key[2];
	ulong32 u32HashVal, u32HashInd;

	if (row > col && IsSymmetric())
	{
		register int temp= row;
		row= col;
		col= temp;
	}
	u32Key[0]= row;
	u32Key[1]= col;
	u32HashVal= hash2(u32Key,2,u32Initval);
	u32HashInd= u32HashVal % m_u32MaxNofElements;
	SRAMatrixElement* pElement;
	pElement= reinterpret_cast<SRAMatrixElement*>
		(m_pElementHashTable+(u32HashInd*m_uElementSize));
	if ((pElement->Row()==row && pElement->Column()== col) || IsFresh(*pElement))
		return *pElement;

	SRAMatrixElement *pAvail;
	if (*pElement)
		pAvail= 0;
	else
		pAvail= pElement;

	ulong32 u32Step, u32ProbeVal;
	u32Step= hash2(u32Key,2,u32HashVal);
	u32Step= (u32Step % m_u32MaxNofElements)|1;
	// case of u32Step equal to m_u32MaxNofElements it is detected by next check
	if (u32Step % (m_u32MaxNofElements>>m_u32HashTableOrder)==0)
		u32Step -= 2;
	for (u32ProbeVal= (u32HashInd+u32Step) % m_u32MaxNofElements;
		u32ProbeVal != u32HashInd;
		u32ProbeVal = (u32ProbeVal+u32Step) % m_u32MaxNofElements)
	{
		pElement= reinterpret_cast<SRAMatrixElement*>
			(m_pElementHashTable+(u32ProbeVal*m_uElementSize));
		if ((pElement->Row()==row && pElement->Column()== col))
			return *pElement;
		if (!*pElement)
		{
			if (!pAvail)
				pAvail= pElement;
			if (IsFresh(*pElement))
				return *pAvail;
		}
	}
	if (pAvail)
		return *pAvail;
	bool bTableFull;
	//if (IsSymmetric())
	//	bTableFull=(m_u32NofElements==(m_u32HashTableMask&~1));
	//else

	// Should we relocate an element for the cell?
	if (!AddNewLocation)
		return m_xNullElement;

	bTableFull=(m_pContentStats->GetTotalNofUsed()==GetMaxNofElements());
/*	LOG4CXX_ERROR(log,"findElement(" << row << "," << col <<") Matrix " << (void*)this
		<< (bTableFull ? " is full " : " is NOT full ") << *m_pContentStats << std::endl
		<< "TotalNofUsed " << m_pContentStats->GetTotalNofUsed()
		<< ' ' << (bTableFull ? '=' : '!') << "= max " << GetMaxNofElements()
		<< " u32Step "<< u32Step << std::endl);
*/	if (!bTableFull)
	{
		int nElements,nFresh,nRemoved;
		int nAdded[NOFSRADirections];
		nElements=nFresh=nRemoved= 0;
		nAdded[SDCol]=nAdded[SDRow]= 0;
		u32ProbeVal= u32HashInd;
		do
		{
			pElement= reinterpret_cast<SRAMatrixElement*>
				(m_pElementHashTable+(u32ProbeVal*m_uElementSize));
			if (*pElement)
			{
				if (pElement->Row()==m_nHeadingsRow[SDCol])
					nAdded[SDCol]++;
				else if (pElement->Row()==m_nHeadingsRow[SDRow])
					nAdded[SDRow]++;
				else
					nElements++;
			}
			else if (pElement->IsFresh(m_nDeadInds))
				nFresh++;
			else
				nRemoved++;
			u32ProbeVal = (u32ProbeVal+u32Step) % m_u32MaxNofElements;
		}
		while (u32ProbeVal != u32HashInd);
/*		LOG4CXX_ERROR(log,"findElement(" << row << "," << col <<") Matrix " << (void*)this
			<< "is NOT full: " << " max " << GetMaxNofElements()
			<< " scan Results: " << std::endl
			<< "\tnElements " << nElements << " nFresh " << nFresh << " nRemoved " << nRemoved
			<< " nAddedCols " << nAdded[SDCol] << " nAddedRows " << nAdded[SDRow]
			<< std::endl);*/
		// table isn't full???
		MH_ASSERT(bTableFull);
		throw 1;
	}
	// table is full
	MH_ASSERT(!bTableFull);
	return m_xNullElement;
}

SRAMatrixElement & ISRAMatrix::findOrAddElement(int row,int col)
{
	SRAMatrixElement & rElement= findElement(row,col);
	if (!rElement)
		rElement.Init(row,col);
	bool bHeadElm[NOFSRADirections];
	int nElmDir, nTotHeadElm;
	for (nTotHeadElm= 0, nElmDir=SDCol; nElmDir<=SDRow;nElmDir++)
	{
		SRADirection eElmDir;
		eElmDir= (SRADirection)nElmDir;
		nTotHeadElm+=
			(bHeadElm[nElmDir]= rElement.IsMatrixElement(eElmDir));
	}
	if (nTotHeadElm==2)
		return rElement;
	if (nTotHeadElm==0)
	{
		if (m_pu8DfltInitial)
			MemCpy(&rElement+1,m_pu8DfltInitial,m_uValueSize);

		m_pContentStats->AddElement();//+= 1+ IsSymmetric();
	}

	int nHeadInd= col;
	for (nElmDir=SDCol; nElmDir<=SDRow;nElmDir++,nHeadInd=row)
	{
		if (!bHeadElm[nElmDir])
		{	// Chain to row/column heading
			SRADirection eElmDir,eHeadingDir;
			eElmDir= (SRADirection)nElmDir;
			eHeadingDir= (SRADirection)(1-nElmDir);
			SRAMatrixElement & rHeading= findHeadingElement(eElmDir,nHeadInd);
			if (!rHeading)
				rHeading.Init(m_nHeadingsRow[nElmDir],nHeadInd);
			if (!rHeading.IsMatrixElement(eHeadingDir))
			{	// Add column heading
	#ifndef ORDERED_ADJACENCY_LISTS
				AppendAtTail(m_xTopLeftCorner,rHeading,eHeadingDir);
	#else
				InsertOrdered(m_xTopLeftCorner,rHeading,eHeadingDir);
	#endif // ORDERED_ADJACENCY_LISTS
				rHeading.m_pNext[nElmDir]= 0;
				m_pContentStats->AddHeading(eElmDir,nHeadInd);
				if (m_pu8DfltInitial)
					MemCpy(&rHeading+1,m_pu8DfltInitial,m_uValueSize);
			}

	#ifndef ORDERED_ADJACENCY_LISTS
			AppendAtTail(rHeading,rElement,eElmDir);
	#else
			InsertOrdered(rHeading,rElement,eElmDir);
	#endif // ORDERED_ADJACENCY_LISTS
		}
	}

	return rElement;

}


void ISRAMatrix::restartIterator(iterator & riter,SRADirection eSDir/*=SDCol*/) const
{
	/*
	if (IsSymmetric())
	{
		MH_ASSERT(eSDir==SDCol);
		eSDir= SDCol;
	}
	*/
	riter.u32SingleIndex= 0; // All elements iteration
	riter.eSDirection= eSDir;
	int nHeadingDir= 1-eSDir;
	riter.ppNextHeading= &m_xTopLeftCorner.m_pNext[nHeadingDir];
	if ((riter.pCurHeading= *riter.ppNextHeading))
	{
		riter.ppNext= &(riter.pCurHeading->m_pNext[eSDir]);
		if (!*riter.ppNext)
			advanceHeading(riter,eSDir);
	}
	else
	{
		riter.ppNext= &m_pNullElement;
	}

}

void ISRAMatrix::removeCurrent(iterator & rIter)
{
	SRAMatrixElement **& rppNext=rIter.ppNext;

	if (*rppNext)
	{
		SRADirection eIterDir= rIter.eSDirection;
		SRADirection eHeadingDir= (SRADirection)(1-eIterDir);
		SRAMatrixElement *pCur= *rppNext;
		SRAMatrixElement * & rpCurNext= pCur->m_pNext[eIterDir];
		*rppNext= rpCurNext;

		rpCurNext= pCur;
		if (!pCur->IsMatrixElement(eHeadingDir))
		{
			m_pContentStats->RemoveElement();
			pCur->Invalidate(m_nDeadInds);
		}

		if (!*rppNext)
		{
			// removed last element --> advance to next heading
			advanceIterator(rIter);
		}
	}
}

void
ISRAMatrix::advanceHeading(iterator & riter,SRADirection eSDir) const
{

	int nHeadingDir= 1-eSDir;

	SRAMatrixElement **ppNext= 0;
	SRAMatrixElement *pCurHeading= riter.pCurHeading;
	SRAMatrixElement *pPrevHeading= pCurHeading;
	while (pPrevHeading)
	{
		if ((pCurHeading= *riter.ppNextHeading))
		{
			if (pCurHeading==pPrevHeading)
				// Passed PrevHeading
				pPrevHeading= 0;
			ppNext= &(pCurHeading->m_pNext[eSDir]);
			if (*ppNext)
				riter.ppNextHeading=&(pCurHeading->m_pNext[nHeadingDir]);
		}

		// either by advancing the iterator or by removing the current heading
		// the iterator now points at the next heading
		while ((pCurHeading= *riter.ppNextHeading))
		{
			ppNext= &(pCurHeading->m_pNext[eSDir]);
			if (*ppNext)
			{
				break;
			}
			else
			{
				if (pCurHeading==pPrevHeading)
					// Passed removed PrevHeading
					pPrevHeading= 0;
				SRAMatrixElement * & rpCurNextHeading= pCurHeading->m_pNext[nHeadingDir];
				*riter.ppNextHeading= rpCurNextHeading;
				m_pContentStats->RemoveHeading(eSDir);
				if (!this->m_pnRecycledDeadInds)
					pCurHeading->Invalidate(m_nDeadInds);
				else
				{	// remove heading during RecycleGarbage
					pCurHeading->Invalidate(m_pnRecycledDeadInds);
					m_pContentStats->MarkRemoved();
				}

			}
		}	/// while !bFinish

		if (!pCurHeading)
		{
			// reached end of headings line
			ppNext= &m_pNullElement;
			MH_ASSERT(!pPrevHeading);
		}
	} // while pPrevHeading
	riter.ppNext= ppNext;
	riter.pCurHeading= pCurHeading;

}

int ISRAMatrix::removeHalfRemoved(SRADirection eOppositeDir)
{
	iterator xIter;
	SRAMatrixElement *pCur;
	SRADirection eInitialDir= (SRADirection)(1-eOppositeDir);
	int nFound= 0;
	for (restartIterator(xIter,eOppositeDir);
		(pCur= protGetCurrent(xIter));)
	{
		MH_ASSERT(pCur->IsMatrixElement(eOppositeDir));
		if (!pCur->IsMatrixElement(eInitialDir))
		{
			removeCurrent(xIter);
			nFound++;
		}
		else
		{
			advanceIterator(xIter);
		}
	}

	return nFound;
}
int ISRAMatrix::rejoinHalfRemoved(SRADirection eOppositeDir)
{
	iterator xIter;
	SRAMatrixElement *pCur;
	SRADirection eInitialDir= (SRADirection)(1-eOppositeDir);
	int nJoined= 0;
	for (restartIterator(xIter,eOppositeDir);
		(pCur= protGetCurrent(xIter));)
	{
		MH_ASSERT(pCur->IsMatrixElement(eOppositeDir));
		if (!pCur->IsMatrixElement(eInitialDir))
		{
			int nRow, nCol;
			getCurrentInds(xIter,nRow,nCol);
			SRAMatrixElement &rJoined= findOrAddElement(nRow,nCol);
			nJoined++;
		}
		advanceIterator(xIter);
	}

	return nJoined;
}
void ISRAMatrix::removeCol(int col)
{
	iterator xIter,xRowIter;
	SRAMatrixElement *pCur;
	int nFound= 0, nRowFound= 0;
	for (restartIterator(xIter,col);
		(pCur= protGetCurrent(xIter));)
	{
		int rowInd= getCurrentRowInd(xIter);
		removeCurrent(xIter);
		nFound++;
		iterator xRowIter;
		for (restartRowIterator(xRowIter,rowInd);
			(pCur= protGetCurrent(xRowIter));)
		{
			MH_ASSERT(pCur->IsMatrixElement(SDRow));
			if (getCurrentColInd(xRowIter)==col) {
				MH_ASSERT(!pCur->IsMatrixElement(SDCol));
				removeCurrent(xRowIter);
				nRowFound++;
			}
			else
			{
				advanceIterator(xRowIter);
			}
		}
	}

	MH_ASSERT(nRowFound==nFound);
}


void ISRAMatrix::ClearAll()
{
	iterator xIter;
	int nDir;
	SRAMatrixElement *pCur;
	for (nDir=SDCol; nDir<=SDRow; nDir++)
	{
		SRADirection eDir= (SRADirection )nDir;
		for (restartIterator(xIter,eDir);
			(pCur= protGetCurrent(xIter));)
		{
			removeCurrent(xIter);
		}
	}
	MH_ASSERT(m_pContentStats->GetTotalNofUsed()==0);
	m_pContentStats->Reset(&m_xCreatePars);
}
ulong32 ISRAMatrix::RecycleGarbage(bool bForced/*=false*/)
{
	if (!bForced &&
		(m_pContentStats->GetNofMarkedRemoved()*2 < m_pContentStats->GetTotalNofUsed() ||
		(m_pContentStats->GetNofMarkedRemoved()+ m_pContentStats->GetTotalNofUsed())*2 <
			GetMaxNofElements()))
		return 0;

	ulong32 u32NofRemovedSaved= m_pContentStats->GetNofMarkedRemoved();
	iterator xIter;
	int nDir;
	SRAMatrixElement *pCur;
	int nRecycledDeadInds[NOFSRADirections];
	for (nDir=SDCol; nDir<=SDRow; nDir++)
		nRecycledDeadInds[nDir]= m_nDeadInds[nDir];
	nRecycledDeadInds[SDCol]--;
	if (nRecycledDeadInds[SDCol]== 0)
		nRecycledDeadInds[SDRow]--;
	m_pnRecycledDeadInds= nRecycledDeadInds;
	m_pContentStats->ResetMarkedRemoved();
	for (nDir=SDCol; nDir<=SDRow; nDir++)
	{
		int nPrevInd= 0;
		SRADirection eDir= (SRADirection )nDir;
		for (restartIterator(xIter,eDir);
			(pCur= protGetCurrent(xIter));advanceIterator(xIter))
		{
			int nCurInds[NOFSRADirections];
			getCurrentInds(xIter,nCurInds[SDRow],nCurInds[SDCol]);
			if (nCurInds[nDir]!=nPrevInd)
			{
				nPrevInd= nCurInds[nDir];
				findHeadingElement((SRADirection)nDir,nPrevInd);
			}
			if (nDir==SDCol || !pCur->IsMatrixElement(SDCol))
			{
				const SRAMatrixElement &rUsedElm= findElement(nCurInds[SDRow],nCurInds[SDCol]);
				MH_ASSERT(&rUsedElm==pCur && rUsedElm);
			}
		}
	}
	for (nDir=SDCol; nDir<=SDRow; nDir++)
		m_nDeadInds[nDir]= nRecycledDeadInds[nDir];
	m_pnRecycledDeadInds= 0;
	m_pContentStats->AddMaintOp();

	ulong32 u32Recycled= u32NofRemovedSaved- m_pContentStats->GetNofMarkedRemoved();

	return u32Recycled;
}

std::ostream & operator <<(std::ostream & ost, const ISRAMatrix & mat)
{
	ost << "Matrix: Flags(" << (mat.IsSymmetric() ? "Sym" : "Asym")
		<< ") nnz " << mat.GetNofElements() << " : max " << mat.GetMaxNofElements();
	if (mat)
	{
		ISRAMatrix::iterator xIter;
		int nRow,nCol,nLastCol= 0;
		const SRAMatrixElement *pCurElm;
		for (const_cast<ISRAMatrix &>(mat).restartIterator(xIter);
			(pCurElm=mat.protGetCurrent(xIter)); mat.advanceIterator(xIter))
		{
			mat.getCurrentInds(xIter,nRow,nCol);
			if (nCol != nLastCol)
			{
				ost<< std::endl << "Column " << nCol << ":";
				nLastCol= nCol;
			}
			ost << " " << nRow << ",";
			mat.PrintVal(ost,*pCurElm);
		}
	}
	ost << std::endl;

	return ost;
}

