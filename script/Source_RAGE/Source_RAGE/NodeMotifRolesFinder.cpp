/** 
 * Copyright 2010 Dror Marcus. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY DROR MARCUS ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DROR MARCUS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those of the
 * authors and should not be interpreted as representing official policies, either expressed
 * or implied, of Dror Marcus.
 */

#include "NodeMotifRolesFinder.h"

#include <queue>
using namespace std;
const int NodeMotifRolesFinder::IDs[199] = {14, 28, 30, 74, 76, 78, 90, 92, 94, 204, 206, 222, 280, 282, 286, 
			  328, 330, 332, 334, 344, 346, 348, 350, 390, 392, 394, 396, 398, 
			  404, 406, 408, 410, 412, 414, 454, 456, 458, 460, 462, 468, 470, 
			  472, 474, 476, 478, 856, 858, 862, 904, 906, 908, 910, 922, 924, 
			  926, 972, 974, 990, 2184, 2186, 2190, 2202, 2204, 2206, 2252, 2254, 
			  2270, 2458, 2462, 2506, 2510, 2524, 2526, 3038, 4370, 4374, 4382, 
			  4418, 4420, 4422, 4424, 4426, 4428, 4430, 4434, 4436, 4438, 4440, 
			  4442, 4444, 4446, 4546, 4548, 4550, 4556, 4558, 4562, 4564, 4566, 
			  4572, 4574, 4678, 4682, 4686, 4692, 4694, 4698, 4700, 4702, 4740, 
			  4742, 4748, 4750, 4758, 4764, 4766, 4812, 4814, 4830, 4946, 4950, 
			  4952, 4954, 4958, 4994, 4998, 5002, 5004, 5006, 5010, 5012, 5014, 
			  5016, 5018, 5020, 5022, 5058, 5062, 5064, 5066, 5068, 5070, 5074, 
			  5076, 5078, 5080, 5082, 5084, 5086, 6342, 6348, 6350, 6356, 6358, 
	   		  6364, 6366, 6550, 6552, 6554, 6558, 6598, 6602, 6604, 6606, 6614, 
			  6616, 6618, 6620, 6622, 6854, 6858, 6862, 6870, 6874, 6876, 6878, 
			  7126, 7128, 7130, 7134, 13142, 13146, 13148, 13150, 13260, 13262, 
			  13278, 14678, 14686, 14790, 14798, 14810, 14812, 14814, 15258, 
   			  15262, 15310, 15326, 31710 };

int NodeMotifRolesFinder::permutations[24][4] = {
	{0, 1, 2, 3}, {0, 1, 3, 2}, {0, 2, 1, 3}, {0, 2, 3, 1}, {0, 3, 1, 2}, {0, 3, 2, 1},
	{1, 0, 2, 3}, {1, 0, 3, 2}, {1, 2, 0, 3}, {1, 2, 3, 0}, {1, 3, 0, 2}, {1, 3, 2, 0},
	{2, 0, 1, 3}, {2, 0, 3, 1}, {2, 1, 0, 3}, {2, 1, 3, 0}, {2, 3, 0, 1}, {2, 3, 1, 0}, 
	{3, 0, 1, 2}, {3, 0, 2, 1}, {3, 1, 0, 2}, {3, 1, 2, 0}, {3, 2, 0, 1}, {3, 2, 1, 0}
};

GraphletS4::GraphletS4(int id)
{
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
		{
			Matrix[i][j] = id & 1;
			id >>= 1;
		}

}

int GraphletS4::GetMatrix() const
{
	int Mat = 0;
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
		{
			Mat = Mat << 1;
			if (Matrix[3-i][3-j] == 1)
				Mat |= 1;
		}
	
	return Mat;
}

void GraphletS4::Permutate(int Perm[4])
{
	char Mat[4][4];
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
		{
			Mat[i][j] = Matrix[Perm[i]][Perm[j]];
		}
	memcpy(Matrix, Mat, sizeof(Mat));
}

int GraphletS4::GetPermutation(int Perm[4]) const
{
	GraphletS4 tmp(GetMatrix());
	tmp.Permutate(Perm);
	return tmp.GetMatrix();
}

int GraphletS4::GetPermutation(unsigned int first, unsigned int second, bool PermOtherPair) const
{
	GraphletS4 tmp(GetMatrix());
	tmp.Permutate(first,second, PermOtherPair);
	return tmp.GetMatrix();
}

void GraphletS4::Permutate(unsigned int first, unsigned int second, bool PermOtherPair)
{
	if (first > 3 || second > 3)
	{
		return;
	}

	char tmp[4];
	memcpy(tmp, Matrix[first], sizeof(char[4]));
	memcpy(Matrix[first], Matrix[second], sizeof(char[4]));
	memcpy(Matrix[second], tmp, sizeof(char[4]));

	for (int i = 0; i < 4; i++)
	{
		tmp[0] = Matrix[i][first];
		Matrix[i][first] = Matrix[i][second];
		Matrix[i][second] = tmp[0];
	}

	if (PermOtherPair)
	{
		static int MatOtherPair[4][4][2] = 
		// sec=0  sec=1  sec=2  sec=3
		{
		{{-1,-1}, {2,3}, {1,3}, {1,2}}, // first == 0
		{{2,3}, {-1,-1}, {0,3}, {0,2}}, // first == 1
		{{1,3}, {0,3}, {-1,-1}, {0,1}}, // first == 2
		{{1,2}, {0,2}, {0,1}, {-1,-1}}}; // first == 3		
		
		Permutate(MatOtherPair[first][second][0],MatOtherPair[first][second][1], false);
	}
}

NodeMotifRolesFinder::NodeMotifRolesFinder(void)
{
}

NodeMotifRolesFinder::~NodeMotifRolesFinder(void)
{
}

void NodeMotifRolesFinder::LoadRoleID(int id)
{
	queue<int> nums;
	nums.push(0);
	nums.push(1);
	nums.push(2);
	nums.push(3);

	unsigned char RoleCount = 0;
	unsigned char Roles[4];

	GraphletS4 motif(id);
	int first;
	int second;
	
	int size;
	int PermutateMatA = 0;
	int PermutateMatB = 0;
	while (!nums.empty())
	{
		first = nums.front();
		nums.pop();
		size = (int)nums.size();
		Roles[first] = RoleCount;
		for (int i = 0; i < size; i++)
		{
			second = nums.front();
			nums.pop();
			PermutateMatA = motif.GetPermutation(first, second);
			PermutateMatB = motif.GetPermutation(first,second, true);
			if (PermutateMatA == id ||
				PermutateMatB == id)
			{
				Roles[second] = RoleCount;
			}
			else
			{
				nums.push(second);
			}
		}
		RoleCount++;
	}

	HashElement& elem = MotifRoles.GetElement(id);
	elem.motifID = id;
	elem.RoleCount = (unsigned char)RoleCount;
	memcpy(elem.Roles, Roles, sizeof(elem.Roles));

	for (int j = 1; j < 24; j++)
	{
		PermutateMatA = motif.GetPermutation(permutations[j]);
		if (PermutateMatA != id)
		{
			HashElement& newElem = MotifRoles.GetElement(PermutateMatA);
			newElem.motifID = id;
			newElem.RoleCount = RoleCount;

			for (int x = 0; x < 4; x++)
				newElem.Roles[x] = Roles[permutations[j][x]];
		}
	}
}

void NodeMotifRolesFinder::LoadRoles()
{
	for (int i = 0; i < 199; i++)
	{	
		LoadRoleID(IDs[i]);
	}
}

/****
int main( int argc, char* argv[] )
{

NodeMotifRolesFinder tmp;
tmp.LoadRoles();

ofstream motifs;

motifs.open("MotifRoles.txt");
int offsets[4096] = {0};
if (motifs.is_open())
{
motifs << "Roles for motifs MOTIFID : [node1, node2, node3, node4]" << endl;
for (int i = 0; i < 199; i++)
{
	motifs << (int)tmp.IDs[i] << ": [" <<  flush; 
	motifs << (int)tmp.GetRoleHash().GetElement(tmp.IDs[i]).Roles[0] << ", "
		   << (int)tmp.GetRoleHash().GetElement(tmp.IDs[i]).Roles[1] << ", "
		   << (int)tmp.GetRoleHash().GetElement(tmp.IDs[i]).Roles[2] << ", "
		   << (int)tmp.GetRoleHash().GetElement(tmp.IDs[i]).Roles[3]
		   << flush;
	motifs << "]" <<  endl; 
}

//motifs << endl << endl;
motifs.close();
}
return 0;
}
***/