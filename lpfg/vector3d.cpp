/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#include "vector3d.h"
#include "quaternion.h"
#include "utils.h"

void Vector3d::RotateBy(Vector3d h, Vector3d l, Vector3d u) {
  Vector3d tmp(*this);
  X(tmp.X() * l.X() + tmp.Y() * h.X() + tmp.Z() * u.X());
  Y(tmp.X() * l.Y() + tmp.Y() * h.Y() + tmp.Z() * u.Y());
  Z(tmp.X() * l.Z() + tmp.Y() * h.Z() + tmp.Z() * u.Z());
}

void Vector3d::RotateBy(const Quaternion &q) {
  Quaternion qi(q);
  qi.Invert();
  Quaternion qv(*this);

  Quaternion qr = qi * qv;
  qv = qr * q;
  FromQuaternion(qv);
}

void Vector3d::RotateTo(Vector3d v) {
  Vector3d vec = *this % v;
  float len = vec.Length();
  if (this->operator*(v) < 0.0f)
    len = 1.0f;
  else if (len > 1.0f)
    len = 1.0f;
  float angle = len * 0.5f;
  float bendsin = sinf(angle);
  vec *= bendsin / len;

  Quaternion q(cosf(angle), vec);
  RotateBy(q);
}

void Vector3d::FromQuaternion(const Quaternion &q) {
  X(q.X());
  Y(q.Y());
  Z(q.Z());
}

void Vector3d::Dump() const {
  Utils::Message("x = %f\ty = %f\tz = %f\n", X(), Y(), Z());
}

OpenGLMatrix::OpenGLMatrix() { Identity(); }

OpenGLMatrix::OpenGLMatrix(const OpenGLMatrix &m) {
  for (int i = 0; i < 16; ++i)
    _arr[i] = m._arr[i];
}

OpenGLMatrix::OpenGLMatrix(const float *arr) {
  for (int i = 0; i < 16; ++i)
    _arr[i] = arr[i];
}

void OpenGLMatrix::Identity() {
  for (int i = 0; i < 16; ++i)
    _arr[i] = 0.0f;
  _arr[0] = _arr[5] = _arr[10] = _arr[15] = 1.0f;
}

void OpenGLMatrix::Transpose() {
  OpenGLMatrix m(*this);
  _arr[0] = m[0];
  _arr[1] = m[4];
  _arr[2] = m[8];
  _arr[3] = m[12];

  _arr[4] = m[1];
  _arr[5] = m[5];
  _arr[6] = m[9];
  _arr[7] = m[13];

  _arr[8] = m[2];
  _arr[9] = m[6];
  _arr[10] = m[10];
  _arr[11] = m[14];

  _arr[12] = m[3];
  _arr[13] = m[7];
  _arr[14] = m[11];
  _arr[15] = m[15];
}

void OpenGLMatrix::Translate(float x, float y, float z) {
  OpenGLMatrix m(*this);
  OpenGLMatrix t;
  t._arr[12] = x;
  t._arr[13] = y;
  t._arr[14] = z;
  Times(m, t);
}

void OpenGLMatrix::Multiply(const float *arr) {
  OpenGLMatrix m(*this);
  OpenGLMatrix t(arr);
  Times(m, t);
}

void OpenGLMatrix::Scale(float x, float y, float z) {
  OpenGLMatrix m(*this);
  OpenGLMatrix s;
  s._arr[0] = x;
  s._arr[5] = y;
  s._arr[10] = z;
  Times(m, s);
}

bool OpenGLMatrix::Inverse()
// from MESA implementation of the GLU library
{
  double inv[16], det;
  int i;

  inv[0] = _arr[5] * _arr[10] * _arr[15] - _arr[5] * _arr[11] * _arr[14] -
           _arr[9] * _arr[6] * _arr[15] + _arr[9] * _arr[7] * _arr[14] +
           _arr[13] * _arr[6] * _arr[11] - _arr[13] * _arr[7] * _arr[10];

  inv[4] = -_arr[4] * _arr[10] * _arr[15] + _arr[4] * _arr[11] * _arr[14] +
           _arr[8] * _arr[6] * _arr[15] - _arr[8] * _arr[7] * _arr[14] -
           _arr[12] * _arr[6] * _arr[11] + _arr[12] * _arr[7] * _arr[10];

  inv[8] = _arr[4] * _arr[9] * _arr[15] - _arr[4] * _arr[11] * _arr[13] -
           _arr[8] * _arr[5] * _arr[15] + _arr[8] * _arr[7] * _arr[13] +
           _arr[12] * _arr[5] * _arr[11] - _arr[12] * _arr[7] * _arr[9];

  inv[12] = -_arr[4] * _arr[9] * _arr[14] + _arr[4] * _arr[10] * _arr[13] +
            _arr[8] * _arr[5] * _arr[14] - _arr[8] * _arr[6] * _arr[13] -
            _arr[12] * _arr[5] * _arr[10] + _arr[12] * _arr[6] * _arr[9];

  inv[1] = -_arr[1] * _arr[10] * _arr[15] + _arr[1] * _arr[11] * _arr[14] +
           _arr[9] * _arr[2] * _arr[15] - _arr[9] * _arr[3] * _arr[14] -
           _arr[13] * _arr[2] * _arr[11] + _arr[13] * _arr[3] * _arr[10];

  inv[5] = _arr[0] * _arr[10] * _arr[15] - _arr[0] * _arr[11] * _arr[14] -
           _arr[8] * _arr[2] * _arr[15] + _arr[8] * _arr[3] * _arr[14] +
           _arr[12] * _arr[2] * _arr[11] - _arr[12] * _arr[3] * _arr[10];

  inv[9] = -_arr[0] * _arr[9] * _arr[15] + _arr[0] * _arr[11] * _arr[13] +
           _arr[8] * _arr[1] * _arr[15] - _arr[8] * _arr[3] * _arr[13] -
           _arr[12] * _arr[1] * _arr[11] + _arr[12] * _arr[3] * _arr[9];

  inv[13] = _arr[0] * _arr[9] * _arr[14] - _arr[0] * _arr[10] * _arr[13] -
            _arr[8] * _arr[1] * _arr[14] + _arr[8] * _arr[2] * _arr[13] +
            _arr[12] * _arr[1] * _arr[10] - _arr[12] * _arr[2] * _arr[9];

  inv[2] = _arr[1] * _arr[6] * _arr[15] - _arr[1] * _arr[7] * _arr[14] -
           _arr[5] * _arr[2] * _arr[15] + _arr[5] * _arr[3] * _arr[14] +
           _arr[13] * _arr[2] * _arr[7] - _arr[13] * _arr[3] * _arr[6];

  inv[6] = -_arr[0] * _arr[6] * _arr[15] + _arr[0] * _arr[7] * _arr[14] +
           _arr[4] * _arr[2] * _arr[15] - _arr[4] * _arr[3] * _arr[14] -
           _arr[12] * _arr[2] * _arr[7] + _arr[12] * _arr[3] * _arr[6];

  inv[10] = _arr[0] * _arr[5] * _arr[15] - _arr[0] * _arr[7] * _arr[13] -
            _arr[4] * _arr[1] * _arr[15] + _arr[4] * _arr[3] * _arr[13] +
            _arr[12] * _arr[1] * _arr[7] - _arr[12] * _arr[3] * _arr[5];

  inv[14] = -_arr[0] * _arr[5] * _arr[14] + _arr[0] * _arr[6] * _arr[13] +
            _arr[4] * _arr[1] * _arr[14] - _arr[4] * _arr[2] * _arr[13] -
            _arr[12] * _arr[1] * _arr[6] + _arr[12] * _arr[2] * _arr[5];

  inv[3] = -_arr[1] * _arr[6] * _arr[11] + _arr[1] * _arr[7] * _arr[10] +
           _arr[5] * _arr[2] * _arr[11] - _arr[5] * _arr[3] * _arr[10] -
           _arr[9] * _arr[2] * _arr[7] + _arr[9] * _arr[3] * _arr[6];

  inv[7] = _arr[0] * _arr[6] * _arr[11] - _arr[0] * _arr[7] * _arr[10] -
           _arr[4] * _arr[2] * _arr[11] + _arr[4] * _arr[3] * _arr[10] +
           _arr[8] * _arr[2] * _arr[7] - _arr[8] * _arr[3] * _arr[6];

  inv[11] = -_arr[0] * _arr[5] * _arr[11] + _arr[0] * _arr[7] * _arr[9] +
            _arr[4] * _arr[1] * _arr[11] - _arr[4] * _arr[3] * _arr[9] -
            _arr[8] * _arr[1] * _arr[7] + _arr[8] * _arr[3] * _arr[5];

  inv[15] = _arr[0] * _arr[5] * _arr[10] - _arr[0] * _arr[6] * _arr[9] -
            _arr[4] * _arr[1] * _arr[10] + _arr[4] * _arr[2] * _arr[9] +
            _arr[8] * _arr[1] * _arr[6] - _arr[8] * _arr[2] * _arr[5];

  det = _arr[0] * inv[0] + _arr[1] * inv[4] + _arr[2] * inv[8] +
        _arr[3] * inv[12];

  if (det == 0)
    return false;

  det = 1.0 / det;

  for (i = 0; i < 16; i++)
    _arr[i] = inv[i] * det;

  return true;
}

void OpenGLMatrix::Dump() const {
  for (int i = 0; i < 16; ++i) {
    Utils::Message("%f\t", _arr[i]);
    if (3 == i % 4)
      Utils::Message("\n");
  }
}

void OpenGLMatrix::Times(const OpenGLMatrix &l, const OpenGLMatrix &r) {
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      float sum = 0.0f;
      for (int k = 0; k < 4; ++k) {
        sum += l.Get(i, k) * r.Get(k, j);
      }
      Set(i, j, sum);
    }
  }
}
