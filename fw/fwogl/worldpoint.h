#ifndef __WORLDPOINT_H__
#define __WORLDPOINT_H__


#ifndef _INC_MATH
#include <math.h>
#endif


template<class f>
class WorldPoint
{
public:
	WorldPoint()
	{ _arr[0] = _arr[1] = _arr[2] = 0.0f; }
	WorldPoint(f x, f y, f z = 0.0f)
	{
		_arr[0] = x;
		_arr[1] = y;
		_arr[2] = z;
	}

	void Set(f x, f y, f z)
	{
		_arr[0] = x;
		_arr[1] = y;
		_arr[2] = z;
	}

	void X(f x) { _arr[0] = x; }
	void Y(f y) { _arr[1] = y; }
	void Z(f z) { _arr[2] = z; }

	f X() const
	{ return _arr[0]; }
	f Y() const
	{ return _arr[1]; }
	f Z() const
	{ return _arr[2]; }

	const f* Vertex() const
	{ return _arr; }
	void IncX(f x)
	{ _arr[0] += x; }
	void IncY(f y)
	{ _arr[1] += y; }

	void Scale(f s)
	{ 
		_arr[0] *= s;
		_arr[1] *= s;
		_arr[2] *= s;
	}

	const WorldPoint<f>& operator=(const WorldPoint<f>& src)
	{ 
		_arr[0] = src._arr[0];
		_arr[1] = src._arr[1];
		_arr[2] = src._arr[2];
		return *this;
	}
	WorldPoint<f>& operator+=(const WorldPoint<f>& src)
	{
		_arr[0] += src._arr[0];
		_arr[1] += src._arr[1];
		_arr[2] += src._arr[2];
		return *this;
	}
	WorldPoint<f>& operator-=(const WorldPoint<f>& src)
	{
		_arr[0] -= src._arr[0];
		_arr[1] -= src._arr[1];
		_arr[2] -= src._arr[2];
		return *this;
	}
	WorldPoint<f>& operator/=(f div)
	{
		_arr[0] /= div;
		_arr[1] /= div;
		_arr[2] /= div;
		return *this;
	}
	WorldPoint<f> operator-(const WorldPoint<f>& wp) const
	{
		WorldPoint res;
		res.X(_arr[0] - wp._arr[0]);
		res.Y(_arr[1] - wp._arr[1]);
		res.Z(_arr[2] - wp._arr[2]);
		return res;
	}
	WorldPoint<f> operator+(const WorldPoint<f>& wp) const
	{
		WorldPoint res;
		res.X(_arr[0] + wp._arr[0]);
		res.Y(_arr[1] + wp._arr[1]);
		res.Z(_arr[2] + wp._arr[2]);
		return res;
	}

	friend WorldPoint<f> operator*(const WorldPoint<f> l, f r)
	{
		WorldPoint res(l._arr[0]*r, l._arr[1]*r, l._arr[2]*r);
		return res;
	}
	friend f Distance(const WorldPoint<f>& p1, const WorldPoint<f>& p2)
	{
		const f dx = p1._arr[0] - p2._arr[0];
		const f dy = p1._arr[1] - p2._arr[1];
		const f dz = p1._arr[2] - p2._arr[2];
		return sqrtf(dx*dx + dy*dy + dz*dz);
	}
	friend f XYDistance(const WorldPoint<f>& p1, const WorldPoint<f>& p2)
	{
		const f dx = p1._arr[0] - p2._arr[0];
		const f dy = p1._arr[1] - p2._arr[1];
		return sqrtf(dx*dx + dy*dy);
	}
	friend WorldPoint<f> Product(const WorldPoint<f>& p1, const WorldPoint<f>& p2)
	{
		WorldPoint<f> res(
			p1._arr[1]*p2._arr[2] - p2._arr[1]*p1._arr[2],
			p1._arr[2]*p2._arr[0] - p2._arr[2]*p1._arr[0],
			p1._arr[0]*p2._arr[1] - p2._arr[0]*p1._arr[1]);
		return res;
	}
	friend f DotProduct(WorldPoint<f> p1, WorldPoint<f> p2)
	{ return p1._arr[0]*p2._arr[0] + p1._arr[1]*p2._arr[1] + p1._arr[2]*p2._arr[2]; }
	f Len() const
	{ return sqrtf(_arr[0]*_arr[0] + _arr[1]*_arr[1] + _arr[2]*_arr[2]); }
	void Normalize()
	{
		const f d = Len();
		_arr[0] /= d;
		_arr[1] /= d;
		_arr[2] /= d;
	}
	void Minus()
	{
		_arr[0] = -_arr[0];
		_arr[1] = -_arr[1];
		_arr[2] = -_arr[2];
	}
	bool operator != (const WorldPoint<f>& r) const
	{
		if (_arr[0] != r._arr[0])
			return true;
		if (_arr[1] != r._arr[1])
			return true;
		if (_arr[2] != r._arr[2])
			return true;
		return false;
	}
protected:
	f _arr[3];
};

typedef WorldPoint<float> WorldPointf;

#endif
