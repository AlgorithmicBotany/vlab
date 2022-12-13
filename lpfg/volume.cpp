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



#include "volume.h"

Volume::Volume() : _min(0.0, 0.0, 0.0), _max(0.0, 0.0, 0.0) { _clean = true; }

Volume::Volume(const Vector3d &v) : _min(v), _max(v) { _clean = false; }

void Volume::Adapt(const Vector3d &v) {
  if (_clean) {
    _min = v;
    _max = v;
    _clean = false;
  } else {
    if (v.X() < _min.X())
      _min.X(v.X());
    else if (v.X() > _max.X())
      _max.X(v.X());

    if (v.Y() < _min.Y())
      _min.Y(v.Y());
    else if (v.Y() > _max.Y())
      _max.Y(v.Y());

    if (v.Z() < _min.Z())
      _min.Z(v.Z());
    else if (v.Z() > _max.Z())
      _max.Z(v.Z());
  }
}

void Volume::Adapt(const Volume &v) {
  if (_clean) {
    _min = v._min;
    _max = v._max;
    _clean = false;
  } else {
    if (v._min.X() < _min.X())
      _min.X(v._min.X());
    if (v._min.Y() < _min.Y())
      _min.Y(v._min.Y());
    if (v._min.Z() < _min.Z())
      _min.Z(v._min.Z());

    if (v._max.X() > _max.X())
      _max.X(v._max.X());
    if (v._max.Y() > _max.Y())
      _max.Y(v._max.Y());
    if (v._max.Z() > _max.Z())
      _max.Z(v._max.Z());
  }
}

void Volume::Scale(float s) {
  Vector3d center(CX(), CY(), CZ());
  Vector3d extent(Xrange(), Yrange(), Zrange());
  extent *= s;
  _min.Set(center.X() - 0.5f * extent.X(), center.Y() - 0.5f * extent.Y(),
           center.Z() - 0.5f * extent.Z());
  _max.Set(center.X() + 0.5f * extent.X(), center.Y() + 0.5f * extent.Y(),
           center.Z() + 0.5f * extent.Z());
}
