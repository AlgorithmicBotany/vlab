#ifndef __BOUNDINGBOX_H__
#define __BOUNDINGBOX_H__


class BoundingBox
{
public:
	BoundingBox(WorldPointf);
	void Reset(WorldPointf);
	void Adapt(WorldPointf);
	float minX() const
	{ return _minPoint.X(); }
	float maxX() const
	{ return _maxPoint.X(); }
	float minY() const
	{ return _minPoint.Y(); }
	float maxY() const
	{ return _maxPoint.Y(); }
	float minZ() const
	{ return _minPoint.Z(); }
	float maxZ() const
	{ return _maxPoint.Z(); }
	WorldPointf Center() const;
	float XSize() const;
	float YSize() const;
	float ZSize() const;
private:
	WorldPointf _minPoint;
	WorldPointf _maxPoint;
};


#endif
