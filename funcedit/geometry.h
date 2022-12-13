/* ******************************************************************** *
   Copyright (C) 1990-2022 University of Calgary
  
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
  
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * ******************************************************************** */




#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <cmath>
#include <cstdlib>
#include <cassert>

class WorldPoint
{
friend double Distance(const WorldPoint&, const WorldPoint&);
public:
  WorldPoint() : _x(0), _y(0), _z(0), _multiplicity(1) {}
  WorldPoint(double x, double y, double z) : _x(x), _y(y), _z(z), _multiplicity(1) {}
  WorldPoint(double x, double y, double z,int mult) : _x(x), _y(y), _z(z), _multiplicity(mult) {}
  double X() const
  { return _x; }
  double Y() const
  { return _y; }
  double Z() const
  { return _z; }
  void X(double x)
  { _x = x; }
  void Y(double y)
  { _y = y; }
  void Z(double z)
  { _z = z; }
  void Set(double x, double y, double z)
  {
    _x = x;
    _y = y;
    _z = z;
  }

  const WorldPoint& operator=(const WorldPoint& src)
  {
    _x = src._x;
    _y = src._y;
    _z = src._z;
    _multiplicity = src._multiplicity;
    return *this;
  }
  void operator+=(const WorldPoint& src)
  {
    _x += src._x;
    _y += src._y;
    _z += src._z;
  }
   void operator*=(const double f)
  {
    _x *= f;
    _y *= f;
    _z *= f;
  }

  friend inline WorldPoint operator*(WorldPoint wp, double f)
  {
    WorldPoint res(wp.X() * f, wp.Y() * f, wp.Z() * f);
    return res;
  }
  friend inline WorldPoint operator+(WorldPoint p0, WorldPoint p1) {
    return WorldPoint(p0.X() + p1.X(), p0.Y() + p1.Y(), p0.Z() + p1.Z());
  }
  double DistanceTo(WorldPoint wp) const
  {
    const double dx = _x - wp._x;
    const double dy = _y - wp._y;
    const double dz = _z - wp._z;
    return sqrt(dx*dx + dy*dy + dz*dz);
  }

  WorldPoint operator-(WorldPoint wp)
  {
    WorldPoint res(_x - wp._x, _y - wp._y, _z - wp._z);
    return res;
  }
  int getMultiplicity() { return _multiplicity; }
  void setMultiplicity(int m) {
    assert(m > 0);
    _multiplicity = m;
  }
  void incMultiplicity() { _multiplicity++; }

  friend double XYdot(WorldPoint p0, WorldPoint p1) {
    return (p0._x * p1._x + p0._y * p1._y);
  }

  
private:
  double _x, _y, _z;
  int _multiplicity;

};

inline double pow(double x, int n)
{
  if (0==n)
    return 1;
  double res = x;
  int iter = abs(n);
  for (int i=2; i<=iter; i++)
    res *= x;
  if (n<0)
    res = 1.0/res;
  return res;
}

#endif
