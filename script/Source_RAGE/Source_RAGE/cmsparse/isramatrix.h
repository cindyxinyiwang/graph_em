/**
 * isramatrix.h
 * 
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

#ifndef ISRAMATRIX_H_
#define ISRAMATRIX_H_

#ifdef USE_CONTMEMSM
#include "contmemsm.h"
#define ISRAMatrix IContMemSM
#define SRAMatrix ContMemSRAMatrix
#else
#include "sramatrix.h"
#endif // USE_CONTMEMSM

class ISRABiIterator {
public:
	inline ISRABiIterator(ISRAMatrix & rMat);
	// iter accessors
	inline const ISRAMatrix::ColIterator & getColIter() const;
	inline const ISRAMatrix::RowIterator & getRowIter() const;
	// start all Matrix iteration
	inline void restart(SRADirection eSDir=SDCol);
	// start Single col/row iteration
	inline void restart(int nSingleIndex,SRADirection eSDir=SDCol);
	// advance iterator to next col/row element (depending on
	inline void advance();
	// Peer iteration index (row or col index if iterating in SDCol or SDRow direction respectively
	// returns 0 if at end of iteration
	inline int getCurrentInd() const;
	// current iteration row + col indexes
	// returns 0s if at end of iteration
	inline void getCurrentInds(int & rRow, int & rCol) const;
	// check end of iteration
	inline bool atEnd() const;
#ifdef USE_CONTMEMSM
	inline void prepareRowsContMemStruct();
	inline void setRowIterIgnoreDirtyFlag(bool ignoreDirtyFlag);
#endif
protected:
	// remove current col/row element
	inline void protRemoveCurrent();
protected:
	ISRAMatrix &	m_rMat;
	SRADirection	m_eDir;
	ISRAMatrix::ColIterator m_xColIter;
	ISRAMatrix::RowIterator m_xRowIter;
};

inline ISRABiIterator::ISRABiIterator(ISRAMatrix & rMat) :
m_rMat(rMat), m_eDir(NOFSRADirections){}

#ifdef USE_CONTMEMSM
inline void ISRABiIterator::setRowIterIgnoreDirtyFlag(bool ignoreDirtyFlag)
{
	m_xRowIter.setIgnoreFlag(ignoreDirtyFlag);
}
inline void ISRABiIterator::prepareRowsContMemStruct()
{
	m_rMat.prepareRowsContMemStruct();
}
#endif

// iter accessors
inline const ISRAMatrix::ColIterator & ISRABiIterator::getColIter() const
{	return m_xColIter; }
inline const ISRAMatrix::RowIterator & ISRABiIterator::getRowIter() const
{	return m_xRowIter; }
// start all Matrix iteration
inline void ISRABiIterator::restart(SRADirection eSDir/*=SDCol*/)
{
	if (eSDir==SDCol) {
		m_rMat.restartColIterator(m_xColIter);
		m_eDir= SDCol;
	}
	else if (eSDir==SDRow) {
		m_rMat.restartRowIterator(m_xRowIter);
		m_eDir= SDRow;
	}
	else
		m_eDir= NOFSRADirections;
}
// start Single col/row iteration
inline void ISRABiIterator::restart(int nSingleIndex,SRADirection eSDir/*=SDCol*/)
{
	if (eSDir==SDCol) {
		m_rMat.restartColIterator(m_xColIter,nSingleIndex);
		m_eDir= SDCol;
	}
	else if (eSDir==SDRow) {
		m_rMat.restartRowIterator(m_xRowIter,nSingleIndex);
		m_eDir= SDRow;
	}
	else
		m_eDir= NOFSRADirections;
}
// advance iterator to next col/row element (depending on
inline void ISRABiIterator::advance()
{
	if (m_eDir==SDCol)
		m_rMat.advanceColIterator(m_xColIter);
	else
		m_rMat.advanceRowIterator(m_xRowIter);
}
// Peer iteration index (row or col index if iterating in SDCol or SDRow direction respectively
// returns 0 if at end of iteration
inline int ISRABiIterator::getCurrentInd() const
{
	if (m_eDir==SDCol)
		return m_rMat.getCurrentRowInd(m_xColIter);
	else
		return m_rMat.getCurrentColInd(m_xRowIter);
}
// current iteration row + col indexes
// returns 0s if at end of iteration
inline void ISRABiIterator::getCurrentInds(int & rRow, int & rCol) const
{
	if (m_eDir==SDCol)
		m_rMat.getCurrentInds(m_xColIter,rRow,rCol);
	else
		m_rMat.getCurrentInds(m_xRowIter,rRow,rCol);
}
// check end of iteration
inline bool ISRABiIterator::atEnd() const
{
	if (m_eDir==SDCol)
		return m_rMat.atEnd(m_xColIter);
	else
		return m_rMat.atEnd(m_xRowIter);
}
// remove current col/row element
inline void ISRABiIterator::protRemoveCurrent()
{
	if (m_eDir==SDCol)
		m_rMat.removeCurrent(m_xColIter);
	else {
#ifdef USE_CONTMEMSM
		MH_ASSERT(m_eDir==SDCol);
#else
		m_rMat.removeCurrent(m_xRowIter);
#endif
	}
}

template <class VAL,typename theSRAMatrix>
class SRABiIterator : public ISRABiIterator {
public:
	inline SRABiIterator(theSRAMatrix & rMat);
	// mat accessor
	inline const theSRAMatrix & getMat() const;
	// iteration accessors
	// returns 0 if at end of iteration
	inline VAL * getCurrent();
	// returns 0 if at end of iteration
	inline const VAL * getCurrent() const;
	// remove current col/row element
	inline void removeCurrent();
};

template <class VAL,typename theSRAMatrix>
inline SRABiIterator<VAL,theSRAMatrix>::SRABiIterator(theSRAMatrix & rMat) :
ISRABiIterator(rMat)
{}

// mat accessor
template <class VAL,typename theSRAMatrix>
inline const theSRAMatrix &
SRABiIterator<VAL,theSRAMatrix>::getMat() const
{	return static_cast<const theSRAMatrix& >(m_rMat);	}

template <class VAL,typename theSRAMatrix>
inline VAL * SRABiIterator<VAL,theSRAMatrix>::getCurrent()
{
	theSRAMatrix & rMat= static_cast<theSRAMatrix& >(m_rMat);
	if (m_eDir==SDCol)
	{
		return rMat.getCurrent(m_xColIter);
	}
	else
	{
		return rMat.getCurrent(m_xRowIter);
	}
}


/*
template <class VAL,typename theSRAMatrix>
inline VAL * SRABiIterator<VAL,theSRAMatrix>::getCurrent()
{
	theSRAMatrix & rMat= static_cast<theSRAMatrix& >(m_rMat);
	if (m_eDir==SDCol)
		return rMat.getCurrent(m_xColIter);
	else {
#ifdef USE_CONTMEMSM
		MH_ASSERT(m_eDir==SDCol);
#else
		return rMat.getCurrent(m_xRowIter);
#endif
	}
}
*/

template <class VAL,typename theSRAMatrix>
inline const VAL * SRABiIterator<VAL,theSRAMatrix>::getCurrent() const
{
	if (m_eDir==SDCol)
		return getMat().getCurrent(m_xColIter);
	else
		return getMat().getCurrent(m_xRowIter);
}

template <class VAL,typename theSRAMatrix>
inline void SRABiIterator<VAL,theSRAMatrix>::removeCurrent()
{
	ISRABiIterator::protRemoveCurrent();
}

template <class VAL,typename theSRAMatrix>
class SRABiConstIterator : public ISRABiIterator {
public:
	inline SRABiConstIterator(const theSRAMatrix & rMat);
	// mat accessor
	inline const theSRAMatrix & getMat() const;
	// returns 0 if at end of iteration
	inline const VAL * getCurrent() const;
};

template <class VAL,typename theSRAMatrix>
inline SRABiConstIterator<VAL,theSRAMatrix>::SRABiConstIterator(const theSRAMatrix & rMat) :
ISRABiIterator(const_cast<theSRAMatrix &> (rMat))
{}

// mat accessor
template <class VAL,typename theSRAMatrix>
inline const theSRAMatrix  &
SRABiConstIterator<VAL,theSRAMatrix>::getMat() const
{	return static_cast<const theSRAMatrix& >(m_rMat);	}

template <class VAL,typename theSRAMatrix>
inline const VAL * SRABiConstIterator<VAL,theSRAMatrix>::getCurrent() const
{
	if (m_eDir==SDCol)
		return getMat().getCurrent(m_xColIter);
	else
		return getMat().getCurrent(m_xRowIter);
}

#endif /* ISRAMATRIX_H_ */
