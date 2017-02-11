#pragma once
#include <iostream>
#include <map>
#include "IDictionary.h"

class IIntDictionary:public IDictionary
{
private: 
	map<int, int> m_ValueToKey;
	map<int, int> m_KeyToValue;
	int m_TotalElements;
public:
	virtual int Init(int max_elements){m_TotalElements =0;max_elements++;return 0;}; // to avoid warnings};
	virtual int Init(){m_TotalElements =0; return 0;};
	virtual IDictionary::ValueTypes GetValueType(){return IDictionary::INT;};
	virtual int GetIntValue(int key, int& outValue);
	virtual int GetStringValue(int key, string& outValue);
	virtual int GetKey(int value, int& outKey);
	virtual int GetValueCount() {return m_TotalElements;};
};

// Inline functions
inline int IIntDictionary::GetIntValue(int key, int& outValue) 
{
	if (key > m_TotalElements)
		return DICTIONARY_KEY_NOT_FOUND;

	map<int, int>::iterator iter = m_KeyToValue.find(key);
	
	if (iter  == m_KeyToValue.end())
	{
		return DICTIONARY_KEY_NOT_FOUND;
	}
	else
	{
		return iter->second;
	}

	return 0;
}

inline int IIntDictionary::GetStringValue(int key, string& outValue) 
{
	if (key > m_TotalElements)
		return DICTIONARY_KEY_NOT_FOUND;

	map<int, int>::iterator iter = m_KeyToValue.find(key);
	
	if (iter  == m_KeyToValue.end())
	{
		return DICTIONARY_KEY_NOT_FOUND;
	}
	else
	{
		char strnum[30];

		sprintf(strnum,"%d",iter->second);
		strnum[29]=0;
		outValue = strnum;
	}

	return DICTIONARY_SUCCESS;
}

inline int IIntDictionary::GetKey(int value, int& outKey) 
{
	map<int, int>::iterator iter = m_ValueToKey.find(value);
	
	if (iter  == m_ValueToKey.end())
	{
		m_ValueToKey[value] = m_TotalElements + 1;
		m_KeyToValue[m_TotalElements + 1] = value;
		m_TotalElements++;
		outKey = m_TotalElements;
		return DICTIONARY_NEW_ELEMENT;
	}
	else
	{
		outKey = iter->second;
	}

	return DICTIONARY_SUCCESS;
}

