#ifndef __SCREENPOINT_H__
#define __SCREENPOINT_H__

class ScreenPoint
{
public:
	ScreenPoint(int x, int y) : _x(x), _y(y) {}
	ScreenPoint() : _x(0), _y(0) {}

	void Set(int x, int y)
	{ _x = x; _y = y; }
	void Set(POINT pt)
	{ _x = pt.x; _y = pt.y; }
	void X(int x) { _x = x; }
	void Y(int y) { _y = y; }

	int X() const
	{ return _x; }
	int Y() const
	{ return _y; }

	friend bool operator==(const ScreenPoint& p1, const ScreenPoint& p2)
	{
		if (p1._x != p2._x)
			return false;
		if (p1._y != p2._y)
			return false;
		return true;
	}
private:
	int _x;
	int _y;
};

#endif
