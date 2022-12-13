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



#ifndef __VOLUME_H__
#define __VOLUME_H__

#include "vector3d.h"
#include "asrt.h"

class Volume {
public:
  Volume();
  Volume(const Vector3d &);
  void Adapt(const Vector3d &);
  void Adapt(const Volume &);
  void Scale(float s);
  float CX() const { return 0.5f * (_max.X() + _min.X()); }
  float CY() const { return 0.5f * (_max.Y() + _min.Y()); }
  float CZ() const { return 0.5f * (_max.Z() + _min.Z()); }
  float MaxX() const { return _max.X(); }
  float MinX() const { return _min.X(); }
  float MaxY() const { return _max.Y(); }
  float MinY() const { return _min.Y(); }
  float MaxZ() const { return _max.Z(); }
  float MinZ() const { return _min.Z(); }
  const Vector3d &Min() const { return _min; }
  const Vector3d &Max() const { return _max; }
  Vector3d Center() const { return 0.5f * (_max + _min); }
  float Xrange() const { return _max.X() - _min.X(); }
  float Yrange() const { return _max.Y() - _min.Y(); }
  float Zrange() const { return _max.Z() - _min.Z(); }
  void Translate(Vector3d v) {
    _min += v;
    _max += v;
  }

  void Set(float x1, float x2, float y1, float y2, float z1, float z2) {
    ASSERT(x1 <= x2);
    _min.X(x1);
    _max.X(x2);
    ASSERT(y1 <= y2);
    _min.Y(y1);
    _max.Y(y2);
    ASSERT(z1 <= z2);
    _min.Z(z1);
    _max.Z(z2);
    _clean = false;
  }
  bool IsInitialized() const { return !_clean; }

private:
  Vector3d _min, _max;
  bool _clean;
};

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
