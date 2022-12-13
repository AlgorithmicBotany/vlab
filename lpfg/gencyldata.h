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



#ifndef __GENCYLDATA_H__
#define __GENCYLDATA_H__

#include "vector3d.h"

class GeneralizedCylinderData {
public:
  GeneralizedCylinderData();

  struct Scale {
    Scale() : p(1.0f), q(1.0f) {}
    float p, q;
  };

  void Set(const Scale &,    // width
           const Vector3d &, // position
           const Vector3d &, // up
           const Vector3d &, // left
           const Vector3d &, // head
           int,              // contourId
           int,              // contourId2
           float             // blender
  );
  bool On() const { return _isOn; }
  void Start() { _isOn = true; }
  void End() { _isOn = false; }
  Vector3d Position() const { return _position; }
  Vector3d Left() const { return _left; }
  Vector3d Up() const { return _up; }
  Vector3d Heading() const { return _head; }
  float ScaleP() const { return _scale.p; }
  float ScaleQ() const { return _scale.q; }
  int ContourId() const { return _contourId; }
  int ContourId2() const { return _contourId2; }
  float Blender() const { return _blender; }
  bool Blended() const { return _blender > 0.0f; }

private:
  bool _isOn;
  Scale _scale;
  Vector3d _position;
  Vector3d _up;
  Vector3d _left;
  Vector3d _head;
  int _contourId;
  int _contourId2;
  float _blender;
};

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
