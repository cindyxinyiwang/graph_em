#pragma once
#include "IOutputResults.h"
#include "Graph.h"

class TextOutputParam : 
	public IOutputParam
{
public: string sourcefile;
	    Graph& G; 
		double Runtime;
};


class TextOutputResults :
	public IOutputResults
{
public:
	TextOutputResults(void);
	virtual int SaveResults(IOutputParam* params);
public:
	~TextOutputResults(void);
};
