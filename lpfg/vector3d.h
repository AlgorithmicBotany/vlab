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



#ifndef __VECTOR3D_H__
#define __VECTOR3D_H__

#include <cmath>
#include <iostream> 

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

class Quaternion;

class PolarPoint {
public:
  // Longitude is [-PI (west), PI (east)]
  // Lattutude is [-PI/2 (south), PI/2 (north)]
  PolarPoint(float lon, float lat) : _lon(lon), _lat(lat) {}
  float Longitute() const { return _lon; }
  float Lattitude() const { return _lat; }

private:
  float _lon;
  float _lat;
};

class Vector3d {
public:
  Vector3d() { _arr[0] = _arr[1] = _arr[2] = 0.0f; }
  Vector3d(float x, float y, float z) {
    _arr[0] = x;
    _arr[1] = y;
    _arr[2] = z;
  }
  Vector3d(const Vector3d src, const float rot[16]) {
    _arr[0] =
        rot[0] * src._arr[0] + rot[4] * src._arr[1] + rot[8] * src._arr[2];
    _arr[1] =
        rot[1] * src._arr[0] + rot[5] * src._arr[1] + rot[9] * src._arr[2];
    _arr[2] =
        rot[2] * src._arr[0] + rot[6] * src._arr[1] + rot[10] * src._arr[2];
  }
  Vector3d(const PolarPoint &pp) {
    // (1,0,0) --> PI/2 east, 0 north
    // (0,1,0) --> 0, PI/2 north
    // (0,0,1) --> 0, 0
    _arr[0] = sinf(pp.Longitute()) * cosf(pp.Lattitude());
    _arr[1] = sinf(M_PI - pp.Lattitude());
    _arr[2] = cosf(pp.Longitute()) * cosf(pp.Lattitude());
  }
  operator const float *() const { return _arr; }
  Vector3d operator*(float v) const {
    return Vector3d(_arr[0] * v, _arr[1] * v, _arr[2] * v);
  }
  // cross product
  Vector3d operator%(Vector3d v) const {
    return Vector3d(_arr[1] * v._arr[2] - _arr[2] * v._arr[1],
                    _arr[2] * v._arr[0] - _arr[0] * v._arr[2],
                    _arr[0] * v._arr[1] - _arr[1] * v._arr[0]);
  }
  // dot product
  float operator*(Vector3d v) const {
    return _arr[0] * v._arr[0] + _arr[1] * v._arr[1] + _arr[2] * v._arr[2];
  }
  friend Vector3d operator*(float x, Vector3d v) {
    return Vector3d(x * v._arr[0], x * v._arr[1], x * v._arr[2]);
  }
  void operator+=(const Vector3d &v) {
    _arr[0] += v._arr[0];
    _arr[1] += v._arr[1];
    _arr[2] += v._arr[2];
  }
  void operator-=(const Vector3d &v) {
    _arr[0] -= v._arr[0];
    _arr[1] -= v._arr[1];
    _arr[2] -= v._arr[2];
  }
  void operator*=(float v) {
    _arr[0] *= v;
    _arr[1] *= v;
    _arr[2] *= v;
  }
  void operator/=(float v) {
    _arr[0] /= v;
    _arr[1] /= v;
    _arr[2] /= v;
  }
  Vector3d operator-() const { return Vector3d(-_arr[0], -_arr[1], -_arr[2]); }
  float X() const { return _arr[0]; }
  float Y() const { return _arr[1]; }
  float Z() const { return _arr[2]; }
  void X(float x) { _arr[0] = x; }
  void Y(float y) { _arr[1] = y; }
  void Z(float z) { _arr[2] = z; }
  void Set(float x, float y, float z) {
    _arr[0] = x;
    _arr[1] = y;
    _arr[2] = z;
  }
  void Set(const float *a) {
    _arr[0] = a[0];
    _arr[1] = a[1];
    _arr[2] = a[2];
  }
  void Set(const PolarPoint &pp) {
    _arr[0] = sinf(pp.Longitute()) * cosf(pp.Lattitude());
    _arr[1] = sinf(M_PI - pp.Lattitude());
    _arr[2] = cosf(pp.Longitute()) * cosf(pp.Lattitude());
  }
  inline float &operator[](int idx) { return _arr[idx]; } // MC - Dec. 2020 - convenient to access vector elements
  inline float operator[](int idx) const { return _arr[idx]; } // e.g., Vector3d v; v[0] = x; v[1] = y; v[2] = z;
  inline Vector3d &operator=(Vector3d const &r) { // assignment operator
    _arr[0] = r._arr[0];
    _arr[1] = r._arr[1];
    _arr[2] = r._arr[2];
    return *this;
  }
  void FromQuaternion(const Quaternion &);
  void AddX(float v) { _arr[0] += v; }
  void AddY(float v) { _arr[1] += v; }
  void AddZ(float v) { _arr[2] += v; }
  Vector3d operator-(Vector3d r) const {
    return Vector3d(_arr[0] - r._arr[0], _arr[1] - r._arr[1],
                    _arr[2] - r._arr[2]);
  }
  Vector3d operator+(Vector3d r) const {
    return Vector3d(_arr[0] + r._arr[0], _arr[1] + r._arr[1],
                    _arr[2] + r._arr[2]);
  }
  bool operator==(Vector3d r) const {
    return (_arr[0] == r._arr[0]) && (_arr[1] == r._arr[1]) &&
           (_arr[2] == r._arr[2]);
  }
  float Length() const {
    return sqrtf(_arr[0] * _arr[0] + _arr[1] * _arr[1] + _arr[2] * _arr[2]);
  }
  float LengthSquared() const {
    return (_arr[0] * _arr[0] + _arr[1] * _arr[1] + _arr[2] * _arr[2]);
  }

  Vector3d &Normalize(float n = 1.0) {
    float d = n / Length();
    operator*=(d);
    return (*this);
  }
  void RotateBy(Vector3d, Vector3d, Vector3d);
  void RotateBy(const Quaternion &);
  void RotateTo(Vector3d);
  bool IsNull() const {
    return _arr[0] == 0.0f && _arr[1] == 0.0f && _arr[2] == 0.0f;
  }
  Vector3d &Scale(float x, float y, float z) {
    _arr[0] *= x;
    _arr[1] *= y;
    _arr[2] *= z;
    return (*this);
  }
  Vector3d &Abs(void) {
    _arr[0] = std::abs(_arr[0]);
    _arr[1] = std::abs(_arr[1]);
    _arr[2] = std::abs(_arr[2]);
    return (*this);
  }

  // v is an array of 3 floats, mtrx is array of 16 floats
  // v is treated as if the fourth component (w) was 1
  // mtrx is assumed to have the last row 0, 0, 0, 1
  // so that it can be effectively ignored.
  // This means that it won't work properly
  // with a matrix that includes Frustum transformation
  // the matrix is in the column-major order
  // (as returned by glGetFloatv)
  void Transform(const float *v, const float *mtrx) {
    _arr[0] = mtrx[0] * v[0] + mtrx[4] * v[1] + mtrx[8] * v[2] + mtrx[12];
    _arr[1] = mtrx[1] * v[0] + mtrx[5] * v[1] + mtrx[9] * v[2] + mtrx[13];
    _arr[2] = mtrx[2] * v[0] + mtrx[6] * v[1] + mtrx[10] * v[2] + mtrx[14];
  }

  template <class M> void Transform(const M &mtrx) {
    float tmp[3];
    tmp[0] =
        mtrx[0] * _arr[0] + mtrx[4] * _arr[1] + mtrx[8] * _arr[2] + mtrx[12];
    tmp[1] =
        mtrx[1] * _arr[0] + mtrx[5] * _arr[1] + mtrx[9] * _arr[2] + mtrx[13];
    tmp[2] =
        mtrx[2] * _arr[0] + mtrx[6] * _arr[1] + mtrx[10] * _arr[2] + mtrx[14];
    _arr[0] = tmp[0];
    _arr[1] = tmp[1];
    _arr[2] = tmp[2];
  }

  void Dump() const;

private:
  float _arr[3];
};

const Vector3d eX(1.0f, 0.0f, 0.0f);
const Vector3d eY(0.0f, 1.0f, 0.0f);
const Vector3d eZ(0.0f, 0.0f, 1.0f);

inline float Distance(Vector3d v1, Vector3d v2) {
  Vector3d rel = v2 - v1;
  return rel.Length();
}

inline float Deg2Rad(float deg) { return deg * M_PI / 180.0f; }
inline float Rad2Deg(float rad) { return rad * 180.0f / M_PI; }

inline double Deg2Rad(double deg) { return deg * M_PI / 180.0f; }
inline double Rad2Deg(double rad) { return rad * 180.0f / M_PI; }

class OpenGLMatrix {
public:
  OpenGLMatrix();
  OpenGLMatrix(const OpenGLMatrix &);
  OpenGLMatrix(const float *);
  void Identity();
  void Transpose();
  void Translate(float, float, float);
  void Translate(Vector3d v) { Translate(v.X(), v.Y(), v.Z()); }
  void Multiply(const float *);
  void Scale(float, float, float);
  bool Inverse();
  void Dump() const;
  const float *Buf() const { return _arr; }
  float Get(int id) const { return _arr[id]; }
  float operator[](int id) const { return _arr[id]; }

private:
  float Get(int r, int c) const { return _arr[r + 4 * c]; }
  void Set(int r, int c, float v) { _arr[r + 4 * c] = v; }
  void Times(const OpenGLMatrix &, const OpenGLMatrix &);
  float _arr[16];
};

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
