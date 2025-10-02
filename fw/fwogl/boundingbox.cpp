#include <fw.h>

#include "worldpoint.h"
#include "boundingbox.h"


BoundingBox::BoundingBox(WorldPointf p)
{
	_minPoint = p;
	_maxPoint = p;
}

void BoundingBox::Reset(WorldPointf p)
{
	_minPoint = p;
	_maxPoint = p;
}


void BoundingBox::Adapt(WorldPointf p)
{
	if (p.X()<_minPoint.X())
		_minPoint.X(p.X());
	else if (p.X()>_maxPoint.X())
		_maxPoint.X(p.X());
	if (p.Y()<_minPoint.Y())
		_minPoint.Y(p.Y());
	else if (p.Y()>_maxPoint.Y())
		_maxPoint.Y(p.Y());
	if (p.Z()<_minPoint.Z())
		_minPoint.Z(p.Z());
	else if (p.Z()>_maxPoint.Z())
		_maxPoint.Z(p.Z());
}


WorldPointf BoundingBox::Center() const
{
	WorldPointf res = (_minPoint+_maxPoint)*0.5f;
	return res;
}

float BoundingBox::XSize() const
{
	return _maxPoint.X()-_minPoint.X();
}


float BoundingBox::YSize() const
{
	return _maxPoint.Y()-_minPoint.Y();
}

float BoundingBox::ZSize() const
{
	return _maxPoint.Z()-_minPoint.Z();
}
