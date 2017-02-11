#pragma once
#include <iostream>

template <class T>
class SortedLinkedList
{
private:
	class LinkedListElement
	{
	public:
		T Value;
		LinkedListElement* Next;
	};

	LinkedListElement* Anchor;
	int ElementCount;
	int DefaultVal;
public:
	
	class Iterator
	{
		SortedLinkedList* m_list;
		LinkedListElement* m_CurrElement;
	public:
		Iterator(SortedLinkedList<T>* list){m_list = list; m_CurrElement = m_list->Anchor;}
		Iterator(const Iterator& it){m_list = it.m_list; m_CurrElement = it.m_CurrElement;}
		inline T GetNextValue();
		inline T Value();
		inline void AdvanceIterator();
		bool EndOfList() {return (m_CurrElement == NULL);};
		void RestartIterator(){ m_CurrElement = m_list->Anchor;};	
		void RestartIterator(const Iterator& it){m_list = it.m_list; m_CurrElement = it.m_CurrElement;}
		void RestartIterator(SortedLinkedList<T>* list){m_list = list; ; m_CurrElement = m_list->Anchor;};
	};

	~SortedLinkedList(){Clear();}
	SortedLinkedList(){Anchor = NULL; ElementCount=0; DefaultVal =-1;};
	int GetCount() {return ElementCount;};
	inline T operator[](int index);

	inline bool operator!=(const SortedLinkedList& other) const
	{
		return !(*this == other);
	}
	inline bool operator==(const SortedLinkedList& other) const
	{
		if (ElementCount != other.ElementCount)
			return false;

		bool res = true;
		LinkedListElement* pA = Anchor;
		LinkedListElement* pB = other.Anchor;

		for (int i = 0; res && i < ElementCount; i++)
		{
			if (pA==NULL || pB==NULL || pA->Value!=pB->Value)
				res = false;
			pA = pA->Next;
			pB= pB->Next;
		}
		
		return res;
	}

	inline bool IsInList(T element);
	inline bool AddToList(T element);
	inline bool RemoveFromList(T element);
	inline void Clear();
};

template <class T>
inline void SortedLinkedList<T>::Clear() 
{
	LinkedListElement* ptr = Anchor;
	while (Anchor != NULL)
	{
		Anchor = ptr->Next;
		delete ptr;
		ptr = Anchor;
	}
	ElementCount = 0;
}

template <class T>
inline T SortedLinkedList<T>::operator[](int index)
{
	LinkedListElement* ptr = Anchor;
	T res = DefaultVal;
	for (int i =0; i < index && ptr!=NULL;i++) 
		ptr = ptr->Next;

	if (ptr != NULL)
		res = ptr->Value;

	return res;
}

template <class T>
inline bool SortedLinkedList<T>::IsInList(T element) 
{
	LinkedListElement* ptr = Anchor;
	while (ptr != NULL && ptr->Value < element)
	{
		ptr = ptr->Next;
	}
	if (ptr!=NULL && ptr->Value == element)
		return true;
	else
		return false;
}

template <class T>
inline bool SortedLinkedList<T>::AddToList(T element) 
{
	LinkedListElement* ptr = Anchor;
	if (Anchor == NULL)
	{
		Anchor = new SortedLinkedList<T>::LinkedListElement();
		Anchor->Next = NULL;
		Anchor->Value = element;
		ptr = Anchor;
	}
	else if (Anchor->Value >= element)
	{
		ptr = new SortedLinkedList<T>::LinkedListElement();
		ptr->Value = element;
		ptr->Next = Anchor;
		Anchor = ptr;
	}
	else
	{
		while (ptr->Next != NULL && ptr->Next->Value < element)
		{
			ptr = ptr->Next;
		}
		LinkedListElement* tmp = new SortedLinkedList<T>::LinkedListElement();
		tmp->Next = ptr->Next;
		tmp->Value = element;
		ptr->Next = tmp;
	}
	ElementCount++;
	return true;
}

template <class T>
inline bool SortedLinkedList<T>::RemoveFromList(T element) 
{
	LinkedListElement* ptr = Anchor;
	if (Anchor == null)
	{
		return false;
	}
	else if (Anchor->Value == element)
	{
		Anchor = Anchor->Next;
		delete ptr;
	}
	else
	{
		while (ptr->Next != NULL && ptr->Next->Value < element)
		{
			ptr = ptr->Next;
		}
		if (ptr->Next == NULL || ptr->Next->Value > element)
		{
			return false;
		}
		else
		{
			LinkedListElement* tmp = ptr->Next;
			ptr->Next = ptr->Next->Next;

			delete tmp;
		}
	}

	ElementCount--;
	return true;
}

/************************************************************
 * SortedLinkedList<T>::Iterator inline functions
 ************************************************************/

template <class T>
inline T SortedLinkedList<T>::Iterator::GetNextValue() 
{
	if (m_CurrElement != NULL) 
	{
		T res = m_CurrElement->Value; 
		m_CurrElement = m_CurrElement->Next;
		return res;
	} else 
	{
		return m_list->DefaultVal;
	}
};

template <class T>
inline T SortedLinkedList<T>::Iterator::Value()
{
	if (m_CurrElement != NULL) 
	{
		return m_CurrElement->Value; 
	} else 
	{
		return m_list->DefaultVal;
	}
}

template <class T>
inline void SortedLinkedList<T>::Iterator:: AdvanceIterator()
{
	if (m_CurrElement != NULL) 
		m_CurrElement = m_CurrElement->Next;
}
