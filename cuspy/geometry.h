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
#include <iostream>

class WorldPoint {
  friend double Distance(const WorldPoint &, const WorldPoint &);

public:
  WorldPoint() : _x(0), _y(0), _z(0), _multiplicity(1) {}
  WorldPoint(double x, double y, double z)
      : _x(x), _y(y), _z(z), _multiplicity(1) {}
  double X() const { return _x; }
  double Y() const { return _y; }
  double Z() const { return _z; }
  void X(double x) { _x = x; }
  void Y(double y) { _y = y; }
  void Z(double z) { _z = z; }
  void Set(double x, double y, double z) {
    _x = x;
    _y = y;
    _z = z;
  }

  friend double XYDistance(const WorldPoint &p1, const WorldPoint &p2) {
    const double dx = p1._x - p2._x;
    const double dy = p1._y - p2._y;
    return sqrt(dx * dx + dy * dy);
  }

  bool operator==(const WorldPoint &wp) const {
    if (_x != wp._x)
      return false;
    if (_y != wp._y)
      return false;
    if (_z != wp._z)
      return false;
    return true;
  }

  const WorldPoint &operator=(const WorldPoint &src) {
    _x = src._x;
    _y = src._y;
    _z = src._z;
    _multiplicity = src._multiplicity;
    return *this;
  }

  void operator+=(const WorldPoint &src) {
    _x += src._x;
    _y += src._y;
    _z += src._z;
  }
  friend inline WorldPoint operator*(WorldPoint wp, double f) {
    WorldPoint res(wp.X() * f, wp.Y() * f, wp.Z() * f);
    return res;
  }
  double DistanceTo(WorldPoint wp) const {
    const double dx = _x - wp._x;
    const double dy = _y - wp._y;
    const double dz = _z - wp._z;
    return sqrt(dx * dx + dy * dy + dz * dz);
  }

  WorldPoint operator-(WorldPoint wp) const {
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

  double XYmagnitude() { return sqrt(_x * _x + _y * _y); }

private:
  double _x, _y, _z;
  int _multiplicity;
};

class WorldLine {
public:
  WorldLine(WorldPoint p0, WorldPoint p1) : _p0(p0), _p1(p1) {}
  double Length() const { return XYDistance(_p0, _p1); }
  double DistanceTo(WorldPoint p) const {
    // see http://astronomy.swin.edu.au/pbourke/geometry/pointline/
    // for the description of the algorithm
    const double l = Length();
    if (0 == l)
      return XYDistance(_p0, p);
    double u = ((p.X() - _p0.X()) * (_p1.X() - _p0.X()) +
                (p.Y() - _p0.Y()) * (_p1.Y() - _p0.Y())) /
               (l * l);
    if (u <= 0.0)
      return XYDistance(_p0, p);
    else if (u >= 1.0)
      return XYDistance(_p1, p);

    WorldPoint intersection(_p0.X() + u * (_p1.X() - _p0.X()),
                            _p0.Y() + u * (_p1.Y() - _p0.Y()), 0.0);
    return XYDistance(intersection, p);
  }

private:
  WorldPoint _p0;
  WorldPoint _p1;
};

#endif
