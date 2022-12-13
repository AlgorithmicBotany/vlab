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

class WorldPoint
{
friend double Distance(const WorldPoint&, const WorldPoint&);
public:
  WorldPoint() : _x(0), _y(0), _z(0) {}
  WorldPoint(double x, double y, double z) : _x(x), _y(y), _z(z) {}
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
    return *this;
  }
  void operator+=(const WorldPoint& src)
  {
    _x += src._x;
    _y += src._y;
    _z += src._z;
  }
  void operator-=(const WorldPoint& src)
  {
    _x -= src._x;
    _y -= src._y;
    _z -= src._z;
  }
  friend inline WorldPoint operator*(WorldPoint wp, double f)
  {
    WorldPoint res(wp.X() * f, wp.Y() * f, wp.Z() * f);
    return res;
  }
  friend inline WorldPoint operator/(WorldPoint wp, double f)
  {
    WorldPoint res(wp.X() / f, wp.Y() / f, wp.Z() / f);
    return res;
  }

  void operator*=(double s) {
    _x *= s;
    _y *= s;
    _z *= s;
  }
  void operator/=(double s) {
    _x /= s;
    _y /= s;
    _z /= s;
  }
  bool operator==(WorldPoint& wp) const {
    if (_x != wp._x) return false;
    if (_y != wp._y) return false;
    if (_z != wp._z) return false;
    return true;
  }
  bool operator!=(WorldPoint& wp) const {
    if (_x != wp._x) return true;
    if (_y != wp._y) return true;
    if (_z != wp._z) return true;
    return false;
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
  WorldPoint operator+(WorldPoint wp)
  {
    WorldPoint res(_x + wp._x, _y + wp._y, _z + wp._z);
    return res;
  }

  double magnitude() const {
    return sqrt(_x*_x + _y*_y + _z*_z);
  }

protected:
  double _x, _y, _z;
};

#endif
