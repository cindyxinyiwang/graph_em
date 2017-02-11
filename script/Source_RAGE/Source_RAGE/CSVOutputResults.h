#pragma once
#include "ioutputresults.h"


class CSVOutputParam : 
	public IOutputParam
{
public: 
};

class CSVOutputResults :
	public IOutputResults
{
public:
	CSVOutputResults(void);
	virtual int SaveResults(IOutputParam* params);
public:
	~CSVOutputResults(void);
};
