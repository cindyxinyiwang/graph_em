#pragma once

#include <iostream>

class GraphletS4
{
private:
	char Matrix[4][4];
public:
	GraphletS4(int id);
	int GetMatrix() const;
	void Permutate(unsigned int first, unsigned int second, bool PermOtherPair = false );
	void Permutate(int Perm[4]);
	int GetPermutation(int Perm[4]) const;
	int GetPermutation(unsigned int first, unsigned int second, bool PermOtherPair = false ) const;
};

struct HashElement
{
	int motifID;
	unsigned char RoleCount;
	unsigned char Roles[4];
};

class RoleHash
{
private:
	HashElement GraphletIDs[4096];
public:
	static int ParseID(unsigned int UriID)
	{
		UriID =   ((UriID>>1) & 0x7) | 
		      (((UriID>>1) & 1<<3) | ((UriID>>2) & 0x6<<3) ) | 
			  (((UriID>>2) & 3<<6) | ((UriID>>3) & 0x4<<6) ) |
			  ((UriID>>3) & (7<<9));
		return UriID;
	}
	HashElement& GetElement(unsigned int UriID)
	{
		UriID =   ParseID(UriID);
		if (UriID > 4095)
		{
			// TODO : ASSERT
			return GraphletIDs[0];
		}
		if (UriID == 7)
		{
			return GraphletIDs[7];
		}
 		return GraphletIDs[UriID];
	}
};

class NodeMotifRolesFinder
{
private:
	static int permutations[24][4];
	RoleHash MotifRoles;
	void LoadRoleID(int id);

public:
	static const int IDs[199];
	void LoadRoles();
	RoleHash& GetRoleHash(){return MotifRoles;}
	NodeMotifRolesFinder(void);
	~NodeMotifRolesFinder(void);
};
