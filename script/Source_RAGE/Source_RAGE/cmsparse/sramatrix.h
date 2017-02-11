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

#ifndef SRAMATRIX_H_
#define SRAMATRIX_H_

#include <iostream>
#include <bitset>
#include "mprdefs.h"
#include "mprtypes.h"

enum SRADirection {SDCol,SDRow,NOFSRADirections};

class MPR_API SRAMatrixElement {
public:
	enum {eMinLiveRowInd= -1}; // min live, that is not Dead, RowInd vals
	bool IsFresh(const int *nDeadInds) const;	// fresh element?
	inline void Invalidate(const int *nDeadInds); // remove element, clearing next/prev
	void Init(int nRow,int nCol);//initialize element setting next/prev= self
	operator const void *() const;
	bool IsMatrixElement(SRADirection eSDir) const;
	bool IsMatrixElement() const;
	int Row() const;
	int Column() const;
public:
	SRAMatrixElement *m_pNext[NOFSRADirections];
private:
	int m_nRow;
	int m_nCol;
};

inline SRAMatrixElement::operator const void *() const
{
	return m_nRow>=eMinLiveRowInd ? reinterpret_cast<const void *>(1) : 0;
}

inline bool SRAMatrixElement::IsFresh(const int *nDeadInds) const
{
	return m_nRow<eMinLiveRowInd && (m_nRow!=nDeadInds[SDRow] || m_nCol!=nDeadInds[SDCol]);
}

inline int SRAMatrixElement::Row() const
{
	return m_nRow;
}

inline int SRAMatrixElement::Column() const
{
	return m_nCol;
}

inline bool SRAMatrixElement::IsMatrixElement(SRADirection eSDir) const
{
	return m_pNext[eSDir]!=this;
}

inline bool SRAMatrixElement::IsMatrixElement() const
{
	return IsMatrixElement(SDCol) || IsMatrixElement(SDRow);
}

#ifdef WIN32
#pragma warning(disable:4251)
#endif// WIN32

// Column lists sparse matrix
class MPR_API ISRAMatrix {
	/*static log4cxx::LoggerPtr log;*/
public:
	enum CreateFlag {
		CFSymmetric=0,
		NOFCreateFlags
	};
	class iterator {
	private:
		SRAMatrixElement	**ppNext;
		SRAMatrixElement	**ppNextHeading;
		SRADirection eSDirection;
		ulong32 u32SingleIndex;
		SRAMatrixElement	*pCurHeading;
	friend class ISRAMatrix;
	};
	typedef iterator ColIterator;
	typedef iterator RowIterator;
public:
	class MPR_API CreateParams
	{
	public:
		CreateParams(bool bIsSymmetric=true);
		bool GetFlag(CreateFlag eFlag) const;
		void SetFlag(CreateFlag eFlag,bool bValue=true);
		bool IsSymmetric() const;
	private:
		std::bitset<NOFCreateFlags> m_xFlags;
	};
	class MPR_API ContentStats
	{
	public:
		ContentStats();
		void Reset(const CreateParams * pCreatPars=0);
		ulong32	GetNofElements() const;
		ulong32	GetTotalNofUsed() const;
		ulong32	GetNofMarkedRemoved() const;
		ulong32 GetMaxHeadings() const;
		ulong32 GetMaxHeading(SRADirection eDir) const;
		bool IsSymmetric() const;
		ulong32 GetMaxNonZeroHeadings() const;
		ulong32 GetTotalNonZeroHeadings() const;
		ulong32 GetNonZeroHeading(SRADirection eDir) const;
		uint64 GetMaintOps() const;
	private:
		inline void AddHeading(SRADirection eDir,ulong32 u32HeadInd);
		inline void AddElement();
		inline void RemoveHeading(SRADirection eDir);
		inline void RemoveElement();
		inline void ResetMarkedRemoved();
		inline void MarkRemoved();
		inline void AddMaintOp();

	private:
		ulong32	m_u32NofElements;
		ulong32 m_u32NofMarkedRemoved;
		ulong32 m_u32MaxHead[NOFSRADirections];
		bool	m_bIsSymmetric;
		ulong32 m_u32Added[NOFSRADirections];
		uint64	m_u64MaintOps;
	friend MPR_API std::ostream & operator <<(std::ostream & os, const ContentStats & stats);
	friend class ISRAMatrix;
	};
public:
	// if extra 2^X memory > quota than Init() sets elarged table
	static ulong32 m_gu32WastedMemQuota;
	ISRAMatrix();
	~ISRAMatrix();
	ISRAMatrix & operator =(const ISRAMatrix &rhs);
	virtual void Init(ulong32 u32MaxNofElements,const CreateParams & rCreatePars);
	operator const void *() const;
	const CreateParams & GetCreateParams() const;
	void ClearAll();
	ulong32 RecycleGarbage(bool bForced= false);
	bool IsSymmetric() const;
	ulong32 GetMaxNofElements() const;
	const ContentStats & GetContentStats() const;
	ulong32 GetNofElements() const;

	//Remove a comlete column.
	void removeCol(int col);

	// iteration methods
	// start all Matrix iteration
	void restartIterator(iterator & rIter,SRADirection eSDir=SDCol) const;
	// start Single col/row iteration
	void restartIterator(iterator & rIter,int nSingleIndex,SRADirection eSDir=SDCol) const;
	// advance iterator to next col/row element (depending on
	void advanceIterator(iterator & rIter) const;
	// Peer iteration index (row or col index if iterating in SDCol or SDRow direction respectively
	// returns 0 if at end of iteration
	int getCurrentInd(const iterator & rIter) const;
	// current iteration row + col indexes
	// returns 0s if at end of iteration
	void getCurrentInds(const iterator & rIter,int & rRow, int & rCol) const;
	// check end of iteration
	bool atEnd(const iterator & rIter) const;
	// remove current col/row element
	void removeCurrent(iterator & rIter);
	// completely remove half-removed pairs of elements
	// The given iteration direction, eOppositeDir, should be opposite to the
	// iteration direction used initially to remove the link to the pair node
	int removeHalfRemoved(SRADirection eOppositeDir);
	// reconnect the two way link between half-linked pairs of element
	// The given iteration direction, eOppositeDir, should be opposite to the
	// iteration direction used initially to remove the link to the pair node
	int rejoinHalfRemoved(SRADirection eOppositeDir);

	// col iteration methods- for compatibility with ContMemSM
	// start all Matrix iteration
	void restartColIterator(ColIterator & rIter) const;
	// start Single col/row iteration
	void restartColIterator(ColIterator & rIter,int nSingleIndex) const;
	// advance iterator to next col/row element (depending on
	void advanceColIterator(ColIterator & rIter) const;
	// Peer iteration index (row or col index if iterating in SDCol or SDRow direction respectively
	// returns 0 if at end of iteration
	int getCurrentRowInd(const ColIterator & rIter) const;

	// row iteration methods- for compatibility with ContMemSM
	// start all Matrix iteration
	void restartRowIterator(RowIterator & rIter) const;
	// start Single col/row iteration
	void restartRowIterator(RowIterator & rIter,int nSingleIndex) const;
	// advance iterator to next col/row element (depending on
	void advanceRowIterator(RowIterator & rIter) const;
	// Peer iteration index (row or col index if iterating in SDCol or SDRow direction respectively
	// returns 0 if at end of iteration
	int getCurrentColInd(const RowIterator & rIter) const;

protected:
	static SRAMatrixElement m_xNullElement;
	ISRAMatrix(const ISRAMatrix &src);
	int check_range(int row, int col) const;
	void SetDfltInitial(const byte *pu8DfltInitial,size_t uSize);
	void IsDerivedInitVal(const void *);
	virtual void PrintVal(std::ostream & ost,const SRAMatrixElement & rElm) const= 0;
	// random-access(direct) accessor to head
	// in case the direct slot is occupied fall back to ordinary hashed findElement()
	SRAMatrixElement & findHeadingElement(SRADirection eSDir,int nHeadingInd) const;
	// search valid head following the order of symmetric restartIterator (SDCol,SDRow)
	//TODO: fix inconsistency in symmetric heading element
	//SRAMatrixElement * findSymmetricHeading(int nHeadingInd) const;
	// random-access(hashed) accessors
	// return element at location if exists, otherwise returns empty element at first free probing point
	// if matrix is symmetric adjust sets given row col to upper right triangle (i.e. row<=col)
	// AddNewLocation - Add new location if the element was not found? (false if called from a const function)
	SRAMatrixElement & findElement(int & row,int & col, bool AddNewLocation = false) const;
	// If location is set return its element.
	// Otherwise add and initialize a new row/col element and it, and return its address
	SRAMatrixElement & findOrAddElement(int row,int col);
	// iteration accessors
	// returns 0 if at end of iteration
	SRAMatrixElement * protGetCurrent(const iterator & rIter);
	// returns 0 if at end of iteration
	const SRAMatrixElement * protGetCurrent(const iterator & rIter) const;
private:
	inline static void MemCpy(void *pDst,const void *pSrc,size_t uSize);
	// If element is fresh return true
	// Otherwise while in RecycleGarbage update its DeadInds
	inline bool IsFresh(SRAMatrixElement & rElement) const;
	void _Create();
	void PrivateClear();
	void AppendAtTail(SRAMatrixElement &rListHead,SRAMatrixElement &rElm,SRADirection eSDir);
	void InsertOrdered(SRAMatrixElement &rListHead,SRAMatrixElement &rElm,SRADirection eSDir);
	void _restart(iterator & rIter,int nSingleIndex,SRADirection eSDir) const;
	void advanceHeading(iterator & riter,SRADirection eSDir) const;
private:
	static SRAMatrixElement * m_pNullElement; // signal end of iteration
	static const int	m_nHeadingsRow[NOFSRADirections];
	int					m_nDeadInds[NOFSRADirections];
	const int			*m_pnRecycledDeadInds;
	size_t				m_uValueSize;
	const byte *		m_pu8DfltInitial;
	size_t				m_uElementSize;
	ulong32				m_u32HashTableOrder;
	ulong32				m_u32MaxNofElements;
	byte *				m_pElementHashTable;
	CreateParams		m_xCreatePars;
	ContentStats		*m_pContentStats;
	mutable SRAMatrixElement	m_xTopLeftCorner;
friend MPR_API std::ostream & operator <<(std::ostream & os, const ISRAMatrix & mat);
};

#ifdef WIN32
#pragma warning(default:4251)
#endif// WIN32

inline bool ISRAMatrix::CreateParams::GetFlag(CreateFlag eFlag) const
{	return m_xFlags[eFlag]; }
inline void ISRAMatrix::CreateParams::SetFlag(CreateFlag eFlag,bool bValue/*=true*/)
{	m_xFlags.set(eFlag,bValue); }
inline bool ISRAMatrix::CreateParams::IsSymmetric() const
{	return m_xFlags[CFSymmetric]; }

inline 	ulong32	ISRAMatrix::ContentStats::GetNofElements() const
{	return m_u32NofElements; }
inline 	ulong32	ISRAMatrix::ContentStats::GetNofMarkedRemoved() const
{	return m_u32NofMarkedRemoved; }
inline 	ulong32	ISRAMatrix::ContentStats::GetTotalNofUsed() const
{	return m_u32NofElements+m_u32Added[SDCol]+m_u32Added[SDRow]; }
inline 	ulong32	ISRAMatrix::ContentStats::GetMaxHeadings() const
{
	if (m_u32MaxHead[SDCol] > m_u32MaxHead[SDRow])
		return m_u32MaxHead[SDCol];
	else
		return m_u32MaxHead[SDRow];
}
inline ulong32 ISRAMatrix::ContentStats::GetMaxHeading(SRADirection eDir) const
{	return m_u32MaxHead[eDir]; }
inline 	ulong32	ISRAMatrix::ContentStats::GetMaxNonZeroHeadings() const
{
	if (m_u32Added[SDCol] > m_u32Added[SDRow])
		return m_u32Added[SDCol];
	else
		return m_u32Added[SDRow];
}
inline 	ulong32	ISRAMatrix::ContentStats::GetTotalNonZeroHeadings() const
{
	int nDir;
	ulong32 u32Ret= 0;
	for (nDir= SDCol; nDir<= SDRow; nDir++)
		u32Ret += m_u32Added[nDir];

	return u32Ret;
}
inline ulong32 ISRAMatrix::ContentStats::GetNonZeroHeading(SRADirection eDir) const
{	return m_u32Added[eDir]; }

inline uint64 ISRAMatrix::ContentStats::GetMaintOps() const
{	return m_u64MaintOps; }

inline bool ISRAMatrix::ContentStats::IsSymmetric() const
{	return m_bIsSymmetric; }

inline ulong32 ISRAMatrix::GetMaxNofElements() const
{
	if (*this)
	{
		return m_u32MaxNofElements;
	}
	else
		return 0;
}

inline ISRAMatrix::operator const void *() const
{
	return (const void *)m_pElementHashTable;
}

inline const ISRAMatrix::CreateParams & ISRAMatrix::GetCreateParams() const
{
	return m_xCreatePars;
}

inline ulong32 ISRAMatrix::GetNofElements() const
{
	return m_pContentStats->GetNofElements();
}

inline const ISRAMatrix::ContentStats & ISRAMatrix::GetContentStats() const
{
	return *m_pContentStats;
}


inline bool ISRAMatrix::IsSymmetric() const
{
	return m_pContentStats->IsSymmetric();
}

inline int ISRAMatrix::getCurrentInd(const iterator & rIter) const
{
	const SRAMatrixElement *pCur= *rIter.ppNext;

	if (pCur)
	{
		switch (rIter.eSDirection)
		{
		case SDCol:
			return pCur->Row();
			break;
		case SDRow:
			return pCur->Column();
			break;
		default:
			throw 1;
		}
	}
	else
	{
		return 0;
	}

}

inline void ISRAMatrix::getCurrentInds(const iterator & rIter,int & rRow, int & rCol) const
{
	const SRAMatrixElement *pCur= *rIter.ppNext;

	rRow= pCur->Row();
	rCol= pCur->Column();
}

inline SRAMatrixElement * ISRAMatrix::protGetCurrent(const iterator & rIter)
{
	return *rIter.ppNext;
}

inline const SRAMatrixElement * ISRAMatrix::protGetCurrent(const iterator & rIter) const
{
	return *rIter.ppNext;
}

inline void ISRAMatrix::_restart(iterator &rIter, int nSingleIndex, SRADirection eSDir) const
{
	rIter.eSDirection= eSDir;
	SRAMatrixElement *pHeading;
	pHeading= &findHeadingElement(eSDir,nSingleIndex);
	if (*pHeading && pHeading->IsMatrixElement(eSDir))
		rIter.ppNext= &pHeading->m_pNext[eSDir];
	else
		rIter.ppNext= &m_pNullElement;
	rIter.u32SingleIndex= nSingleIndex;
}

inline void ISRAMatrix::restartIterator(iterator & riter,int nSingleIndex,SRADirection eSDir/*=SDCol*/) const
{
	MH_ASSERT(nSingleIndex>0);
	if (m_pContentStats->IsSymmetric())
	{
		int nDir;
		MH_ASSERT(eSDir==SDCol);
		for (nDir=SDCol; nDir <= SDRow; nDir++)
		{
			SRADirection eRestartDir= (SRADirection)nDir;
			_restart(riter,nSingleIndex,eRestartDir);
			if (*riter.ppNext)
				break;
		}
	}
	else
		_restart(riter,nSingleIndex,eSDir);

	riter.ppNextHeading= &m_pNullElement;
	riter.pCurHeading= 0;
}

inline bool ISRAMatrix::atEnd(const iterator & rIter) const
{
	return *rIter.ppNext==0;
}

inline void ISRAMatrix::advanceIterator(iterator & riter) const
{
	SRAMatrixElement *pCur;
	SRAMatrixElement **& rppNext=riter.ppNext;
	SRADirection eIterDir= riter.eSDirection;
	if ((pCur= *rppNext))
	{
		rppNext= &(pCur->m_pNext[eIterDir]);
		pCur= *rppNext;
	}
	if (!pCur && m_pContentStats->IsSymmetric() && riter.u32SingleIndex)
	{
		if(eIterDir==SDCol)
		{	// switch to horizontal iteration on rest of column
			_restart(riter,riter.u32SingleIndex,SDRow);
		}
		pCur= *rppNext;
	}
	if (!pCur && *riter.ppNextHeading)
	{
		advanceHeading(riter,eIterDir);
	}
}

// col iteration methods- for compatibility with ContMemSM
// start all Matrix iteration
inline void ISRAMatrix::restartColIterator(ColIterator & rIter) const
{
	restartIterator(rIter,SDCol);
}
// start Single col/row iteration
inline void ISRAMatrix::restartColIterator(ColIterator & rIter,int nSingleIndex) const
{
	restartIterator(rIter,nSingleIndex,SDCol);
}
// advance iterator to next col/row element (depending on
inline void ISRAMatrix::advanceColIterator(ColIterator & rIter) const
{
	advanceIterator(rIter);
}
inline int ISRAMatrix::getCurrentRowInd(const ColIterator & rIter) const
{
	return getCurrentInd(rIter);
}

// row iteration methods- for compatibility with ContMemSM
// start all Matrix iteration
inline void ISRAMatrix::restartRowIterator(RowIterator & rIter) const
{
	restartIterator(rIter,SDRow);
}
// start Single col/row iteration
inline void ISRAMatrix::restartRowIterator(RowIterator & rIter,int nSingleIndex) const
{
	restartIterator(rIter,nSingleIndex,SDRow);
}
// advance iterator to next col/row element (depending on
inline void ISRAMatrix::advanceRowIterator(RowIterator & rIter) const
{
	advanceIterator(rIter);
}
inline int ISRAMatrix::getCurrentColInd(const RowIterator & rIter) const
{
	return getCurrentInd(rIter);
}


/*
inline SRAMatrixElement * ISRAMatrix::findSymmetricHeading(int nHeadingInd) const
{
	int nDir;
	for (nDir=SDCol; nDir <= SDRow; nDir++)
	{
		SRADirection eSDir= (SRADirection)nDir;
		SRAMatrixElement *pHeading= &findHeadingElement(eSDir,nHeadingInd);
		if (*pHeading && pHeading->IsMatrixElement(eSDir))
			return pHeading;
	}

	return 0;
}
*/

template <class VAL,typename BaseSRAMatrix=ISRAMatrix>
class SRAMatrix : public BaseSRAMatrix {
private:
	struct SRAElement : public SRAMatrixElement {
		VAL m_xVal;
	};
public:
	typedef VAL UsrVAL;
	SRAMatrix(VAL dfltInitialValue=0);
	SRAMatrix(const SRAMatrix &src);
public:
	// random-access(hashed) accessors
	VAL & element(int row,int col);// if location is not set, allocate and initialize it, and return its reference
	const VAL & element(int row,int col) const;// returns 0 if value is not set
	const VAL & const_element(int row,int col) const;// returns 0 if value is not set // TODO: DROR - fix this (ask noam\ofir)
	bool isNull(int row,int col) const;// return true if value is not set
	VAL & operator()(int row,int col);
	const VAL & operator()(int row,int col) const;
	// A-symmetric heading value accessor
	//bool headingVal(SRADirection eSDir,int nHeadingInd,const VAL & rVal);
	//VAL headingVal(SRADirection eSDir,int nHeadingInd) const;
	// symmetric heading value accessor
	//bool headingVal(int nHeadingInd,const VAL & rVal);
	//VAL headingVal(int nHeadingInd) const;

	// column 'bulk' memory access
	//Creates a new column 'colNum' with 'count' values in 'vals' (invalidates RowsContMemStruct)
	void createNewCol(int colNum, VAL* pVals, int* pRow_idxs, ulong32 count);
	//Add to column (invalidates RowsContMemStruct)
	void addToCol(int colNum, VAL* pVals, int* pRow_idxs, ulong32 count);

	// iteration accessors
	// returns 0 if at end of iteration
	VAL * getCurrent(const ISRAMatrix::iterator & rIter);
	// returns 0 if at end of iteration
	const VAL * getCurrent(const ISRAMatrix::iterator & rIter) const;
protected:
	virtual void PrintVal(std::ostream & ost,const SRAMatrixElement & rElm) const;
	void IsDerivedInitVal(const VAL *);
private:
	VAL	m_xNullValue;
	VAL	m_xDfltInitialValue;
private:
	void SetDfltInitial(VAL dfltInitialValue);
};

template <class VAL,typename BaseSRAMatrix/*=ISRAMatrix*/>
SRAMatrix<VAL,BaseSRAMatrix>::SRAMatrix(VAL dfltInitialValue/*=0*/) :
m_xNullValue(dfltInitialValue)
{
	SetDfltInitial(dfltInitialValue);
}

template <class VAL,typename BaseSRAMatrix/*=ISRAMatrix*/>
SRAMatrix<VAL,BaseSRAMatrix>::SRAMatrix(const SRAMatrix &src) :
m_xNullValue(src.m_xNullValue)
{
	SetDfltInitial(src.m_xDfltInitialValue);
	operator=(src);
}

template <class VAL,typename BaseSRAMatrix/*=ISRAMatrix*/>
void SRAMatrix<VAL,BaseSRAMatrix>::SetDfltInitial(VAL dfltInitialValue)
{
	m_xDfltInitialValue= dfltInitialValue;
	// following test verifies that BaseRAMatrix::VAL is indeed a Base of VAL
	VAL *pDerived= 0;
	BaseSRAMatrix::IsDerivedInitVal(pDerived);
	ISRAMatrix::SetDfltInitial((byte *)&m_xDfltInitialValue,sizeof(VAL));
}

template <class VAL,typename BaseSRAMatrix/*=ISRAMatrix*/>
void SRAMatrix<VAL,BaseSRAMatrix>::IsDerivedInitVal(const VAL *)
{
}
/*
template <class VAL,typename BaseSRAMatrix=ISRAMatrix,
	typename BaseIter=ISRAMatrix::iterator>
SRAMatrix<VAL,BaseSRAMatrix,BaseIter> & SRAMatrix<VAL,BaseSRAMatrix,BaseIter>::operator =(const SRAMatrix &rhs)
{
	// Initialize same size matrix
	Init(rhs.GetMaxNofElements(),rhs.GetCreateParams());
	// copy elements
	iterator xIter;
	int nRow, nCol;
	const VAL *pCurVal;
	for (rhs.restartIterator(xIter);
		pCurVal=rhs.getCurrent(xIter);rhs.advanceIterator(xIter))
	{
		rhs.getCurrentInds(xIter,nRow,nCol);
		element(nRow,nCol)= *pCurVal;
	}

	return *this;
}
*/

template <class VAL,typename BaseSRAMatrix>
void SRAMatrix<VAL,BaseSRAMatrix>::createNewCol(int colNum, VAL* pVals, int* pRow_idxs, ulong32 count)
{
	BaseSRAMatrix::removeCol(colNum);
	addToCol(colNum, pVals, pRow_idxs, count);
}

template <class VAL,typename BaseSRAMatrix>
void SRAMatrix<VAL,BaseSRAMatrix>::addToCol(int colNum, VAL* pVals, int* pRow_idxs, ulong32 count)
{
	for(ulong32 i=0; i< count; i++)
	{
		element(pRow_idxs[i],colNum) = pVals[i];
	}
}

template <class VAL,typename BaseSRAMatrix/*=ISRAMatrix*/>
inline VAL & SRAMatrix<VAL,BaseSRAMatrix>::element(int row,int col)
{
	MH_ASSERT(row>0 && col>0);
	SRAElement & rElement= static_cast<SRAElement &>(
		ISRAMatrix::findOrAddElement(row,col));
	MH_ASSERT(rElement.IsMatrixElement());

	return rElement.m_xVal;
}

template <class VAL,typename BaseSRAMatrix/*=ISRAMatrix*/>
const VAL & SRAMatrix<VAL,BaseSRAMatrix>::const_element(int row,int col) const
{
	// Should be a call to const element but this is more safe and efficient.. 

	MH_ASSERT(row>0 && col>0);
	const SRAMatrixElement &rElement= ISRAMatrix::findElement(row,col,false);
	if (rElement)
	{
		MH_ASSERT(rElement.IsMatrixElement());
		const SRAElement & rValElement=static_cast<const SRAElement &>(rElement);

		return rValElement.m_xVal;
	}
	else
	{
		// returns 0 if value is not set
		return m_xNullValue;
	}
}

template <class VAL,typename BaseSRAMatrix/*=ISRAMatrix*/>
const VAL & SRAMatrix<VAL,BaseSRAMatrix>::element(int row,int col) const
{
	MH_ASSERT(row>0 && col>0);
	const SRAMatrixElement &rElement= ISRAMatrix::findElement(row,col);
	if (rElement)
	{
		MH_ASSERT(rElement.IsMatrixElement());
		const SRAElement & rValElement=static_cast<const SRAElement &>(rElement);

		return rValElement.m_xVal;
	}
	else
	{
		// returns 0 if value is not set
		return m_xNullValue;
	}
}

template <class VAL,typename BaseSRAMatrix/*=ISRAMatrix*/>
bool SRAMatrix<VAL,BaseSRAMatrix>::isNull(int row,int col) const
{// return true if value is not set
	return &element(row,col)==&m_xNullValue;
}

template <class VAL,typename BaseSRAMatrix/*=ISRAMatrix*/>
inline VAL & SRAMatrix<VAL,BaseSRAMatrix>::operator()(int row,int col)
{
	return element(row,col);
}

template <class VAL,typename BaseSRAMatrix/*=ISRAMatrix*/>
inline const VAL & SRAMatrix<VAL,BaseSRAMatrix>::operator()(int row,int col) const
{
	return element(row,col);
}

/*template <class VAL,typename BaseSRAMatrix//=ISRAMatrix,
	typename BaseIter//=ISRAMatrix::iterator>
inline bool SRAMatrix<VAL,BaseSRAMatrix,BaseIter>::headingVal(SRADirection eSDir,int nHeadingInd,const VAL & rVal)
{
	SRAMatrixElement & rHeading=
		ISRAMatrix::findHeadingElement(eSDir,nHeadingInd);
	if (rHeading && rHeading.IsMatrixElement(eSDir))
	{
		SRAElement & rElement= static_cast<SRAElement &>(rHeading);
		rElement.m_xVal= rVal;

		return true;
	}
	else
		return false;

}
template <class VAL,typename BaseSRAMatrix//=ISRAMatrix,
	typename BaseIter//=ISRAMatrix::iterator>
inline VAL SRAMatrix<VAL,BaseSRAMatrix,BaseIter>::headingVal(SRADirection eSDir,int nHeadingInd) const
{
	const SRAMatrixElement & rHeading=
		ISRAMatrix::findHeadingElement(eSDir,nHeadingInd);
	if (rHeading && rHeading.IsMatrixElement(eSDir))
	{
		const SRAElement & rElement= static_cast<const SRAElement &>(rHeading);

		return rElement.m_xVal;
	}
	else
		return m_xNullValue;

}
template <class VAL,typename BaseSRAMatrix//=ISRAMatrix,
	typename BaseIter//=ISRAMatrix::iterator>
inline bool SRAMatrix<VAL,BaseSRAMatrix,BaseIter>::headingVal(int nHeadingInd,const VAL & rVal)
{
	MH_ASSERT(false);// TODO: fix inconsistency between symmetric set/get
	SRAMatrixElement *pHeading= 0;// findSymmetricHeading(nHeadingInd);
	if (pHeading)
	{
		SRAElement & rElement= static_cast<SRAElement &>(*pHeading);
		rElement.m_xVal= rVal;

		return true;
	}
	else
		return false;
}
template <class VAL,typename BaseSRAMatrix//=ISRAMatrix,
	typename BaseIter//=ISRAMatrix::iterator>
inline VAL SRAMatrix<VAL,BaseSRAMatrix,BaseIter>::headingVal(int nHeadingInd) const
{
	MH_ASSERT(false);// TODO: fix inconsistency between symmetric set/get
	const SRAMatrixElement *pHeading= 0;// findSymmetricHeading(nHeadingInd);
	if (pHeading)
	{
		const SRAElement & rElement= static_cast<const SRAElement &>(*pHeading);

		return rElement.m_xVal;
	}
	else
		return m_xNullValue;
}*/

template <class VAL,typename BaseSRAMatrix/*=ISRAMatrix*/>
inline VAL * SRAMatrix<VAL,BaseSRAMatrix>::getCurrent(const ISRAMatrix::iterator & rIter)
{
	SRAMatrixElement *pElement= BaseSRAMatrix::protGetCurrent(rIter);
	if (pElement)
	{
		SRAElement & rElement=*static_cast<SRAElement *>(pElement);

		return &rElement.m_xVal;
	}
	else
	{
		// returns 0 if value is not set
		return 0;
	}
}

template <class VAL,typename BaseSRAMatrix/*=ISRAMatrix*/>
inline const VAL * SRAMatrix<VAL,BaseSRAMatrix>::getCurrent(const ISRAMatrix::iterator & rIter) const
{
	const SRAMatrixElement *pElement= BaseSRAMatrix::protGetCurrent(rIter);
	if (pElement)
	{
		const SRAElement & rElement=*static_cast<const SRAElement *>(pElement);

		return &rElement.m_xVal;
	}
	else
	{
		// returns 0 if value is not set
		return 0;
	}
}

template <class VAL,typename BaseSRAMatrix/*=ISRAMatrix*/>
void SRAMatrix<VAL,BaseSRAMatrix>::PrintVal(std::ostream & ost,const SRAMatrixElement & rElm) const
{
	const VAL & rVal= static_cast<const SRAElement &>(rElm).m_xVal;
	ost << rVal;
}

#endif // SRAMATRIX_H_
