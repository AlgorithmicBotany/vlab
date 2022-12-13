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

class WorldPoint {
public:
  WorldPoint() : _x(0), _y(0), _z(0),_multiplicity(1) {}
  WorldPoint(double x, double y, double z) : _x(x), _y(y), _z(z),_multiplicity(1) {}
  WorldPoint(double x, double y, double z, int m) : _x(x), _y(y), _z(z),_multiplicity(m) {}
  WorldPoint(const WorldPoint& p) : _x(p.GetX()), _y(p.GetY()), _z(p.GetZ()),_multiplicity(p.GetMultiplicity()) {}
  double GetX() const { return _x; }
  double GetY() const { return _y; }
  double GetZ() const { return _z; }
  double GetMultiplicity() const { return _multiplicity; }
  void SetX(double x) { _x = x; }
  void SetY(double y) { _y = y; }
  void SetZ(double z) { _z = z; }
  void SetMultiplicity(int m) { _multiplicity = m; }
  void Set(double x, double y, double z) {
    _x = x;
    _y = y;
    _z = z;
  }
  void Set(double x, double y, double z, int m) {
    _x = x;
    _y = y;
    _z = z;
    _multiplicity = m;
  }
  void Set(const WorldPoint &p){
    _x = p.GetX();
    _y = p.GetY();
    _z = p.GetZ();
    _multiplicity = p.GetMultiplicity();
  }
  const WorldPoint &operator=(const WorldPoint &src) {
    _x = src._x;
    _y = src._y;
    _z = src._z;
    return *this;
  }
  void operator+=(const WorldPoint &src) {
    _x += src._x;
    _y += src._y;
    _z += src._z;
  }

private:
  double _x, _y, _z;
  int _multiplicity;
};

inline WorldPoint operator*(WorldPoint wp, double f) {
  WorldPoint res(wp.GetX() * f, wp.GetY() * f, wp.GetZ() * f);
  return res;
}

inline double Distance(const WorldPoint &a, const WorldPoint &b) {
  const double dx = a.GetX() - b.GetX();
  const double dy = a.GetY() - b.GetY();
  const double dz = a.GetZ() - b.GetZ();
  return sqrt(dx * dx + dy * dy + dz * dz);
}

#else
#error File already included
#endif
