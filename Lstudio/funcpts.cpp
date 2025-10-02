#include <vector>

#include <fw.h>
#include <glfw.h>

#include "funcpts.h"


void FunctionPoints::Insert(size_t p, WorldPointf wp)
{
	iterator it = begin();

	while (p>0)
	{
		++it; 
		--p;
	}

	insert(it, wp);
}


void FunctionPoints::Delete(size_t p)
{
	assert(CanDelete(p));
	iterator it = begin();
	while (p>0)
	{
		++it;
		--p;
	}

	erase(it);
}



bool FunctionPoints::CanDelete(size_t i) const
{
	if (0 == i)
		return false;
	else if (size()-1 == i)
		return false;
	return (size()>4);
}


bool FunctionPoints::operator !=(const FunctionPoints& Rght) const
{
	if (size() != Rght.size())
		return true;
	for (size_t i=0; i<size(); ++i)
	{
		if (at(i) != Rght.at(i))
			return true;
	}
	return false;
}
