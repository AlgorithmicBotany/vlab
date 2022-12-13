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





#ifndef MATRIX_H
#define MATRIX_H

#include "point.h"
#include <iostream>

namespace Shapes
{

class Matrix{

 public:

  Matrix(int rows, int cols);
  Matrix();
  Matrix(const Matrix& P);
  ~Matrix();
  // ostream& operator<<(const ostream &);
  void Output();
  void Set(int i, double val);
  double Get(int i)const;
  int GetRows(){return M;};
  int GetCols(){return N;};
  double* operator[](unsigned int i);
  Matrix  LeastSquares(Matrix b);
  Matrix& operator=(const Matrix &t);
  Matrix operator+(const Matrix &t);
  Matrix operator-(const Matrix &t);
  Matrix operator*(const Matrix &t);
  Matrix operator*(const double &c);
  Point<double, 3> operator*(const Point<double, 3> &p);
  Matrix operator~();

  Matrix Inverse();
  void Identity();
  void RotateX(double theta);
  void RotateY(double theta);
  void RotateZ(double theta);
  void ProjectionOrthog(int i);

  double* EigenVal();
  Matrix EigenVec(double *e_val);  
  Matrix Minor(int i,int j);
  double Det();
  void Resize(int rows, int cols);
  void SetSymmetric(bool s){symmetric=s;};
  bool IsSymmetric(){return symmetric;};
  double Length();
  Matrix Cross(Matrix p);

 private:
  bool symmetric;
  int M,N;
  double** row_maj;
  //  double** col_maj;
};
}
#endif
