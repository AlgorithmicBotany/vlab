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



#ifndef __QUATERNION_H__
#define __QUATERNION_H__

#include "vector3d.h"

class Quaternion {
public:
  Quaternion(float a, Vector3d v) {
    _arr[0] = a;
    _arr[1] = v.X();
    _arr[2] = v.Y();
    _arr[3] = v.Z();
  }
  Quaternion(float a, float x, float y, float z) {
    _arr[0] = a;
    _arr[1] = x;
    _arr[2] = y;
    _arr[3] = z;
  }
  Quaternion(Vector3d v) {
    _arr[0] = 0.0f;
    _arr[1] = v.X();
    _arr[2] = v.Y();
    _arr[3] = v.Z();
  }
  void Invert() {
    float sz = 1.0f / DotProd(*this, *this);
    _arr[0] *= sz;
    _arr[1] *= -sz;
    _arr[2] *= -sz;
    _arr[3] *= -sz;
  }
  void Revert() {
    _arr[1] = -_arr[1];
    _arr[2] = -_arr[1];
    _arr[3] = -_arr[1];
  }
  Quaternion &Normalize(void) {
    float d = 1.0f / DotProd(*this, *this);
    operator*=(d);
    return (*this);
  }
  Quaternion Inverse() {
    float sz = 1.0f / DotProd(*this, *this);
    return Quaternion(_arr[0] * sz, _arr[1] * -sz, _arr[2] * -sz,
                      _arr[3] * -sz);
  }
  Quaternion Reverse() {
    return Quaternion(_arr[0], -_arr[1], -_arr[2], -_arr[3]);
  }
  friend inline float DotProd(Quaternion q1, Quaternion q2) {
    return q1._arr[0] * q2._arr[0] + q1._arr[1] * q2._arr[1] +
           q1._arr[2] * q2._arr[2] + q1._arr[3] * q2._arr[3];
  }
  friend inline Quaternion operator*(const Quaternion &q1,
                                     const Quaternion &q2) {
    return Quaternion(q1._arr[0] * q2._arr[0] -
                          (q1._arr[1] * q2._arr[1] + q1._arr[2] * q2._arr[2] +
                           q1._arr[3] * q2._arr[3]),
                      q1._arr[0] * q2._arr[1] + q2._arr[0] * q1._arr[1] +
                          q1._arr[2] * q2._arr[3] - q1._arr[3] * q2._arr[2],
                      q1._arr[0] * q2._arr[2] + q2._arr[0] * q1._arr[2] +
                          q1._arr[3] * q2._arr[1] - q1._arr[1] * q2._arr[3],
                      q1._arr[0] * q2._arr[3] + q2._arr[0] * q1._arr[3] +
                          q1._arr[1] * q2._arr[2] - q1._arr[2] * q2._arr[1]);
  }
  inline Quaternion &operator*=(const Quaternion &q1) {
    return ((*this) = (*this) * q1);
  }
  inline Quaternion &operator*=(float s) {
    return (*this) =
               Quaternion(_arr[0] * s, _arr[1] * s, _arr[2] * s, _arr[3] * s);
  }
  float A() const { return _arr[0]; }
  float X() const { return _arr[1]; }
  float Y() const { return _arr[2]; }
  float Z() const { return _arr[3]; }

private:
  float _arr[4];
};

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
