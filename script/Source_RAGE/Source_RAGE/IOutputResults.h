#pragma once
#include <iostream>
#include "string.h"
#include "IDictionary.h"
#include "DirectedMotifResults.h"
#include <map>

using namespace std;

typedef  map<int, string> ASTypesMap;

class IOutputParam
{
public:
	IOutputParam() {outUndir = false;}
	string SourceFile;
	string SaveFilename;
	IDictionary* pDictionary;
	DirectedMotifResults* rawResults;
	ASTypesMap* pAsTypes;
	bool outUndir; // Print only undirected motifs
};

class IOutputResults
{
protected:
	void SetFileType(const char* strOriginalFilename, char* strOutNewFilename, int maxLen, const char* EndType);

public:
	virtual int SaveResults(IOutputParam* params) = 0;	
};

