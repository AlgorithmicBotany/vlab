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



#ifndef __MATRIX_H__
#define __MATRIX_H__

#include "asrt.h"
#include "vector3d.h"

template <int p, int q> class Matrix {
public:
  Matrix(const float *arr = 0) {
    if (0 != arr) {
      int ix = 0;
      for (int i = 0; i < p; ++i) {
        for (int j = 0; j < q; ++j) {
          _arr[i][j] = arr[ix];
          ++ix;
        }
      }
    }
  }
  Matrix(const Matrix<p, q> &src) {
    for (int i = 0; i < p; ++i) {
      for (int j = 0; j < q; ++j)
        _arr[i][j] = src._arr[i][j];
    }
  }
  void Set(int x, int y, float v) {
    ASSERT(x >= 0);
    ASSERT(x < p);
    ASSERT(y >= 0);
    ASSERT(y < q);
    _arr[x][y] = v;
  }
  float Get(int x, int y) const {
    ASSERT(x >= 0);
    ASSERT(x < p);
    ASSERT(y >= 0);
    ASSERT(y < q);
    return _arr[x][y];
  }
  int Rows() const { return p; }
  int Cols() const { return q; }
  template <class Ml, class Mr> void Product(const Ml &l, const Mr &r) {
    ASSERT(Rows() == l.Rows());
    ASSERT(Cols() == r.Cols());
    ASSERT(l.Cols() == r.Rows());
    const int R = l.Cols();
    for (int i = 0; i < Rows(); ++i) {
      for (int j = 0; j < Cols(); ++j) {
        float sum = 0.0f;
        for (int k = 0; k < R; ++k)
          sum += l.Get(i, k) * r.Get(k, j);
        Set(i, j, sum);
      }
    }
  }

protected:
  float _arr[p][q];
};

class Matrix4x3 : public Matrix<4, 3> {
public:
  Matrix4x3(Vector3d v0, Vector3d v1, Vector3d v2, Vector3d v3) {
    Set(0, 0, v0.X());
    Set(0, 1, v0.Y());
    Set(0, 2, v0.Z());

    Set(1, 0, v1.X());
    Set(1, 1, v1.Y());
    Set(1, 2, v1.Z());

    Set(2, 0, v2.X());
    Set(2, 1, v2.Y());
    Set(2, 2, v2.Z());

    Set(3, 0, v3.X());
    Set(3, 1, v3.Y());
    Set(3, 2, v3.Z());
  }
  Matrix4x3(const Matrix<4, 4> &l, const Matrix<4, 3> &r) { Product(l, r); }
};

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
