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



#ifndef __VIEWPOS_H__
#define __VIEWPOS_H__

#include "vector3d.h"

class ViewPos {
public:
  ViewPos()
      : _pos(0., 0., 0.), _head(0., 0., 0.), _left(0., 0., 0.), _up(0., 0., 0.),
        _clean(true) {}
  void Set(const Vector3d &pos, const Vector3d &head, const Vector3d &left,
           const Vector3d &up) {
    _pos = pos;
    _head = head;
    _left = left;
    _up = up;
    _clean = false;
  }
  Vector3d Pos(void) const { return _pos; }
  Vector3d Head(void) const { return _head; }
  Vector3d Left(void) const { return _left; }
  Vector3d Up(void) const { return _up; }
  bool IsInitialized(void) const { return !_clean; }

private:
  Vector3d _pos, _head, _left, _up;
  bool _clean;
};

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
