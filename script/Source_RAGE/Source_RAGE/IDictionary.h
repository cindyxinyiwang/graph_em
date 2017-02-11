#pragma once
#include <string>
using namespace std;

class IDictionary
{
public:
	static const int DICTIONARY_SUCCESS = 0;
	static const int DICTIONARY_NEW_ELEMENT = 1;
	static const int DICTIONARY_KEY_NOT_FOUND = -1;
	static const int DICTIONARY_UNKNOWN_ERROR = -2;

public:
	enum ValueTypes {INT, STRING, IP, UNKNOWN};
	virtual IDictionary::ValueTypes GetValueType() = 0;
	virtual int Init(int max_elements) = 0;
	virtual int Init() = 0;
	virtual int GetValueCount() = 0;

	virtual int GetIntValue(int key, int& outValue)= 0;
	virtual int GetStringValue(int key, string& outValue)= 0;

	virtual int GetKey(int value, int& outKey) = 0;

	inline int _GetKey(int value);
	inline int _GetIntValue(int Key);
	inline string _GetStringValue(int Key);

};

////////////////////////////////////////////////////////////////////
// INLINE FUNCTIONS
////////////////////////////////////////////////////////////////////

inline int IDictionary::_GetKey(int value)
{
	int key;
	if (GetKey(value, key) < 0)
		return key;
	else
		return DICTIONARY_UNKNOWN_ERROR;
};

inline int IDictionary::_GetIntValue(int Key)
{
	int value;
	if (GetIntValue(Key, value) < 0)
		return value;
	else
		return DICTIONARY_UNKNOWN_ERROR;
};

inline string IDictionary::_GetStringValue(int Key)
{
	string value;
	if (GetStringValue(Key, value) < 0)
		return "";
	else
		return value;
};

