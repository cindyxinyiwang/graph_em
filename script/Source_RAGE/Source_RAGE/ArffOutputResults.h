#pragma once
#include "ioutputresults.h"
#include "Graph.h"

class ArffOutputParam : 
	public IOutputParam
{
public: 
};

class ArffOutputResults :
	public IOutputResults
{
public:
	ArffOutputResults(void);
	virtual int SaveResults(IOutputParam* params);
public:
	~ArffOutputResults(void);
};
