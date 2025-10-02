#ifndef __LINE_H__
#define __LINE_H__

template<typename f>
class WorldLine
{
public:
	WorldLine(WorldPoint<f> p0, WorldPoint<f> p1) :
	  _p0(p0), _p1(p1)
	{}
	f DistanceTo(WorldPoint<f> p) const // ignores Z coordinates
	{
		const f l = Length();
		if (0==l)
			return XYDistance(_p0, p);
		f u = 
			(
			(p.X()-_p0.X())*(_p1.X()-_p0.X()) +
			(p.Y()-_p0.Y())*(_p1.Y()-_p0.Y())
			)
			/ (l*l);
		if (u<=0.0f)
			return XYDistance(_p0, p);
		else if (u>=1.0f)
			return XYDistance(_p1, p);
		
		WorldPoint<f> intersection
			(
			_p0.X() + u*(_p1.X()-_p0.X()),
			_p0.Y() + u*(_p1.Y()-_p0.Y())
			);
		return XYDistance(intersection, p);
	}
	f Length() const
	{ return XYDistance(_p0, _p1); }
private:
	WorldPoint<f> _p0, _p1;
};


typedef WorldLine<float> WorldLinef;

#endif
