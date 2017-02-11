#pragma once
#include <iostream>
#include "Graph.h"
#include "MotifResults.h"

class UndirectedRAGECount
{
private:
	static inline void ColorNodes(int* arr, int len, int colors, int** CiPathCount);
// TODO Move to general math class
static inline long factorial(long rhs);
static inline int GroupSize(int Group);

	static const int PATH_LEN4_COLORGROUP_PER[16][2];
public:
	UndirectedRAGECount(void);
	static void CountGraphlets(Graph& SearchGraph,
					  MotifResults* pResults, 
					  bool bIsGraphSymetric,
					  bool bConvertToInduced = false);
	static void ConvertResultsToInduced(MotifResults* pResults);

public:
	~UndirectedRAGECount(void);
};

inline long UndirectedRAGECount::factorial(long rhs)
{
    long lhs = 1;
 
    for( long x = 1; x <= rhs; ++x )
    {
       lhs *= x;
    }
 
    return lhs;
}

inline int UndirectedRAGECount::GroupSize(int Group)
{
	int size = 0;
	while (Group > 0)
	{
		size += Group & 1;
		Group = Group>>1;
	}
	return size;
}
inline void UndirectedRAGECount::ColorNodes(int* arr, int len, int colors, int** CiPathCount)
{
	for (int i = 0; i < len; i++)
	{
		arr[i] = rand() % colors; 

		// Clear all 1 color paths count - This should be set 
		for (int j = 0; j < colors;j++)
			CiPathCount[1<<j][i] = 0;

		// Set only own color path 1 as 1
		CiPathCount[1<<arr[i]][i] = 1;
	}

}
