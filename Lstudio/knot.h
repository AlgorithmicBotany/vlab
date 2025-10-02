#ifndef __KNOT_H__
#define __KNOT_H__


class CtrlPoint : public WorldPointf
{
public:
	CtrlPoint()
	{ _multiplicity = 1; }
	CtrlPoint(float x, float y, float z) : WorldPointf(x, y, z)
	{ _multiplicity = 1; }
	void SetMultiplicity(int m)
	{
		assert(m>=1);
		assert(m<=3);
		_multiplicity = m;
	}
	int GetMultiplicity() const
	{ return _multiplicity; }
	void IncMultiplicity()
	{
		_multiplicity++;
		if (4 == _multiplicity)
			_multiplicity = 1;
		assert(_multiplicity>=1);
		assert(_multiplicity<=3);
	}
	void operator=(CtrlPoint src)
	{
		_arr[0] = src._arr[0];
		_arr[1] = src._arr[1];
		_arr[2] = src._arr[2];
		_multiplicity = src._multiplicity;
		assert(_multiplicity>=1);
		assert(_multiplicity<=3);
	}
	void Set(WorldPointf wp)
	{
		_arr[0] = wp.X();
		_arr[1] = wp.Y();
		_arr[2] = wp.Z();
	}
	void Set(float x, float y, float z)
	{ WorldPointf::Set(x, y, z); }
	bool operator !=(const CtrlPoint& r) const
	{ 
		if (_arr[0] != r._arr[0])
			return true;
		if (_arr[1] != r._arr[1])
			return true;
		if (_arr[2] != r._arr[2])
			return true;
		if (_multiplicity != r._multiplicity)
			return true;
		return false;
	}
private:
	int _multiplicity;
	CtrlPoint(WorldPointf);
	void operator=(WorldPointf);
};


#else
	#error File already included
#endif
