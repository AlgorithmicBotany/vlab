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



#include "gencyldata.h"

GeneralizedCylinderData::GeneralizedCylinderData()
    : _isOn(false), _blender(0.0f), _bgnNormal(-1.f,0.f,0.f), _endNormal(-1.f,0.f,0.f), _isNormalChanged(false) {}

void GeneralizedCylinderData::Set(const Scale &scale, const Vector3d &position,
                                  const Vector3d &up, const Vector3d &left,
                                  const Vector3d &head, int contourId,
                                  int contourId2, float blender) {
  _scale = scale;
  _position = position;
  _up = up;
  _left = left;
  _head = head;
  _contourId = contourId;
  _contourId2 = contourId2;
  _blender = blender;
}

void GeneralizedCylinderData::SetNormal(const Vector3d &normal) {
  _bgnNormal = _endNormal;
  _endNormal = normal;
  _isNormalChanged = true;
}
