#ifndef __VIEWBOX_H__
#define __VIEWBOX_H__

class ViewBox
{
public:
	ViewBox();
	void SetX(float xmin, float xmax)
	{
		assert(xmin<xmax);
		_xmin = xmin;
		_xmax = xmax;
		_dx = _xmax-_xmin;
	}
	void SetCX(float cx, float xrange)
	{
		assert(xrange>0);
		_xmin = cx - xrange/2.0f;
		_xmax = cx + xrange/2.0f;
		_dx = xrange;
	}
	void SetY(float ymin, float ymax)
	{
		assert(ymin<ymax);
		_ymin = ymin;
		_ymax = ymax;
		_dy = _ymax - _ymin;
	}
	void SetCY(float cy, float yrange)
	{
		assert(yrange>0);
		_ymin = cy - yrange/2.0f;
		_ymax = cy + yrange/2.0f;
		_dy = yrange;
	}
	void SetZ(float zmin, float zmax)
	{
		assert(zmin<zmax);
		_zmin = zmin;
		_zmax = zmax;
		_dz = _zmax - _zmin;
	}
	void Apply() const
	{
		glOrtho(_xmin, _xmax, _ymin, _ymax, _zmin, _zmax);
	}
	void Frustum() const
	{ 
		glFrustum(_xmin, _xmax, _ymin, _ymax, _zmin, _zmax);
	}
	float MinX() const
	{ return _xmin; }
	float MaxX() const
	{ return _xmax; }
	float MidX() const
	{ return 0.5f*(_xmin + _xmax); }
	float MinY() const
	{ return _ymin; }
	float MaxY() const
	{ return _ymax; }
	float MidY() const
	{ return 0.5f*(_ymin + _ymax); }
	float MinZ() const
	{ return _zmin; }
	float MaxZ() const
	{ return _zmax; }
	float MidZ() const
	{ return 0.5f*(_zmin + _zmax); }
	float XRange() const
	{ return _dx; }
	float YRange() const
	{ return _dy; }
	float ZRange() const
	{ return _dz; }
	WorldPointf Center() const
	{ return WorldPointf(MidX(), MidY(), MidZ()); }
private:
	float _xmin, _xmax;
	float _ymin, _ymax;
	float _zmin, _zmax;
	float _dx, _dy, _dz;
};

#endif
