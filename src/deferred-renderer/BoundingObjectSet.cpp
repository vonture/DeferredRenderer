#include "PCH.h"
#include "BoundingObjectSet.h"

BoundingObjectSet::BoundingObjectSet()
{
}

void BoundingObjectSet::Merge(BoundingObjectSet* set)
{
	for (UINT i = 0; i < set->_spheres.size(); i++)
	{
		_spheres.push_back(set->_spheres[i]);
	}

	for (UINT i = 0; i < set->_aabbs.size(); i++)
	{
		_aabbs.push_back(set->_aabbs[i]);
	}

	for (UINT i = 0; i < set->_obbs.size(); i++)
	{
		_obbs.push_back(set->_obbs[i]);
	}

	for (UINT i = 0; i < set->_frustums.size(); i++)
	{
		_frustums.push_back(set->_frustums[i]);
	}
}
