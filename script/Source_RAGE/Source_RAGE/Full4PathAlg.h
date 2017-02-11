#pragma once
#include <iostream>
#include "Graph.h"
#include "MotifResults.h"

class Full4PathAlg
{
public:
	Full4PathAlg(void);
	static void Path(Graph& SearchGraph, 
					 MotifResults* pResults,
					 bool bIsSymmetric = true);

public:
	~Full4PathAlg(void);
};
