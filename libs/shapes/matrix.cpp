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



#include "matrix.h"
#include <cmath>
#include <vector>
using std::vector;
#define TwentySeventh 0.037037037
#define Third 0.3333333333333
#define Sqrt3 1.732050808
#define epsilon 1e-5

#define debug false

namespace Shapes {

Matrix::Matrix() {
  if (debug)
    std::cout << "def const" << std::endl;
  ;
  M = N = 0;
  row_maj = NULL;
  if (debug)
    std::cout << "out def const" << std::endl;
  ;
}

Matrix::Matrix(const Matrix &P) {

  M = P.M;
  N = P.N;
  row_maj = NULL;
  row_maj = new double *[M];

  for (int i = 0; i < M; i++)
    row_maj[i] = new double[N];

  for (int i = 0; i < M; i++) {
    for (int j = 0; j < N; j++) {
      row_maj[i][j] = P.row_maj[i][j];
    }
  }
}

Matrix::Matrix(int rows, int cols) {

  if (debug)
    std::cout << "in const" << std::endl;
  ;
  M = rows;
  N = cols;
  row_maj = NULL;
  row_maj = new double *[M];

  for (int i = 0; i < M; i++)
    row_maj[i] = new double[N];

  for (int i = 0; i < M; i++)
    for (int j = 0; j < N; j++) {
      row_maj[i][j] = 0;
    }

  if (debug)
    std::cout << "out const" << std::endl;
  ;
}

double Matrix::Length() {

  double len = 0;

  for (int i = 0; i < M; i++) {
    for (int j = 0; j < N; j++) {
      len += (*this)[i][j] * (*this)[i][j];
    }
  }
  len = sqrt(len);

  return (len);
}

void Matrix::Identity() {

  for (int i = 0; i < M; i++)
    for (int j = 0; j < N; j++)
      row_maj[i][j] = 0;

  for (int i = 0; i < M && i < N; i++)
    row_maj[i][i] = 1;
}

void Matrix::RotateX(double theta) {

  assert(M == N && N >= 3);

  Identity();

  row_maj[1][1] = cos(theta);
  row_maj[1][2] = sin(theta);
  row_maj[2][1] = -row_maj[1][2];
  row_maj[2][2] = row_maj[1][1];
}

void Matrix::RotateY(double theta) {

  assert(M == N && N >= 3);

  Identity();

  row_maj[0][0] = cos(theta);
  row_maj[2][0] = sin(theta);
  row_maj[0][2] = -row_maj[2][0];
  row_maj[2][2] = row_maj[0][0];
}

void Matrix::RotateZ(double theta) {

  assert(M == N && N >= 2);

  Identity();

  row_maj[0][0] = cos(theta);
  row_maj[0][1] = sin(theta);
  row_maj[1][0] = -row_maj[0][1];
  row_maj[1][1] = row_maj[0][0];
}

Matrix::~Matrix() {
  if (debug)
    std::cout << "in dest" << std::endl;
  ;

  if (row_maj) {
    for (int i = 0; i < M; i++) {
      if (debug)
        std::cout << "deleting 1" << std::endl;
      ;
      delete (row_maj[i]);
    }
    if (debug)
      std::cout << "deleting 2" << std::endl;
    ;
    delete (row_maj);
  }

  M = N = 0;
  row_maj = NULL;

  if (debug)
    std::cout << "out dest" << std::endl;
  ;
}


void Matrix::Output() {

  for (int i = 0; i < M; i++) {
    for (int j = 0; j < N; j++) {
      std::cout << row_maj[i][j] << " ";
    }
    std::cout << std::endl;
    ;
  }
}

double *Matrix::operator[](unsigned int i) {

  assert(i < (unsigned int)M);
  return row_maj[i];
}

void Matrix::Resize(int rows, int cols) {

  if (debug)
    std::cout << "in Resize" << std::endl;
  ;
  for (int i = 0; i < M; i++)
    delete row_maj[i];

  delete row_maj;

  M = rows;
  N = cols;
  row_maj = new double *[M];

  for (int i = 0; i < M; i++)
    row_maj[i] = new double[N];

  for (int i = 0; i < N; i++)
    for (int j = 0; j < M; j++) {
      row_maj[j][i] = 0;
    }

  if (debug)
    std::cout << "out Resize" << std::endl;
  ;
}

Matrix &Matrix::operator=(const Matrix &P) {

  if (debug)
    std::cout << "in =operator" << std::endl;
  ;
  Resize(P.M, P.N);

  for (int i = 0; i < M; i++) {
    for (int j = 0; j < N; j++) {
      row_maj[i][j] = P.row_maj[i][j];
    }
  }

  if (debug)
    std::cout << "out =operator" << std::endl;
  ;
  return (*this);
}

Matrix Matrix::operator+(const Matrix &P) {

  assert(M == P.M && N == P.N);
  Matrix T(P.M, P.N);

  for (int i = 0; i < M; i++)
    for (int j = 0; j < N; j++)
      T[i][j] = row_maj[i][j] + P.row_maj[i][j];

  return T;
}

Matrix Matrix::operator-(const Matrix &P) {

  assert(M == P.M && N == P.N);

  assert(M == P.M && N == P.N);
  Matrix T(P.M, P.N);

  for (int i = 0; i < M; i++)
    for (int j = 0; j < N; j++)
      T[i][j] = row_maj[i][j] - P.row_maj[i][j];

  return T;
}

Matrix Matrix::operator*(const Matrix &P) {

  assert(N == P.M);
  Matrix T(M, P.N);

  for (int i = 0; i < M; i++) {
    for (int j = 0; j < P.N; j++) {
      T[i][j] = 0;
      for (int k = 0; k < N; k++)
        T[i][j] += row_maj[i][k] * P.row_maj[k][j];
    }
  }
  return T;
}

Point<double, 3> Matrix::operator*(const Point<double, 3> &p) {

  assert(N == 3);
  Point<double, 3> T;

  for (int i = 0; i < M; i++) {
    T[i] = 0;
    for (int k = 0; k < N; k++)
      T[i] += row_maj[i][k] * p.Get(k);
  }
  return T;
}

Matrix Matrix::operator*(const double &c) {

  Matrix T(M, N);

  for (int i = 0; i < M; i++) {
    for (int j = 0; j < N; j++) {
      T[i][j] = c;
      T[i][j] *= row_maj[i][j];
    }
  }
  return T;
}

Matrix Matrix::operator~() {

  Matrix T(N, M);

  for (int i = 0; i < M; i++)
    for (int j = 0; j < N; j++)
      T[j][i] = row_maj[i][j];

  return T;
}

Matrix Matrix::Inverse() {

  assert(N == M);
  Matrix inv(N, N);
  double denom;
  if (N == 2) {
    denom = Det();
    if (debug)
      std::cout << "good2" << std::endl;
    ;
    if (denom) {
      denom = 1 / denom;
      if (debug)
        std::cout << "good1" << std::endl;
      ;
      inv.row_maj[0][0] = denom * row_maj[1][1];
      inv.row_maj[0][1] = -row_maj[0][1] * denom;
      inv.row_maj[1][0] = -row_maj[1][0] * denom;
      inv.row_maj[1][1] = row_maj[0][0] * denom;
    }
    if (debug)
      std::cout << "good" << std::endl;
    ;
  } else if (N == 3) {

    for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
        inv[i][j] = (Minor(i, j).Det());
        if ((i + j) % 2)
          inv[i][j] = -1 * inv[i][j];
      }
    }
    denom = Det();
    if (denom) {

      denom = (1 / denom);
      inv = inv * denom;
      inv = ~inv;
    }

  } else {

    // implement gausian with full-pivoting
    if (debug)
      std::cout << "Gausian pivoting not implemented" << std::endl;
    ;
  }

  return inv;
}

double Matrix::Det() {

  assert(N == M);
  double d = 0;

  if (N == 2) {

    d = row_maj[0][0] * row_maj[1][1] - row_maj[0][1] * row_maj[1][0];

  } else if (N == 3) {

    d = row_maj[0][0] * row_maj[1][1] * row_maj[2][2];
    d += row_maj[0][1] * row_maj[1][2] * row_maj[2][0];
    d += row_maj[0][2] * row_maj[1][0] * row_maj[2][1];
    d -= row_maj[0][0] * row_maj[1][2] * row_maj[2][1];
    d -= row_maj[0][1] * row_maj[1][0] * row_maj[2][2];
    d -= row_maj[0][2] * row_maj[1][1] * row_maj[2][0];
  } else {

    // generalized det calculation
    if (debug)
      std::cout << "determinate for greater than 3X3 not implemented"
                << std::endl;
    ;
  }
  return d;
}

Matrix Matrix::Minor(int r, int c) {

  Matrix cmat(M - 1, N - 1);
  int ci, cj;
  ci = 0;
  for (int i = 0; i < M; i++) {
    cj = 0;
    if (r != i) {
      for (int j = 0; j < N; j++) {
        if (j != c) {
          cmat[ci][cj] = row_maj[i][j];
          cj++;
        }
      }
      ci++;
    }
  }
  return cmat;
}

Matrix Matrix::EigenVec(double *e_val) {

  Matrix E(N, N);
  vector<Matrix> e_vec;

  if (symmetric && M == N) {

    if (M > 3) {

      // jacobi transformation, does not work
      Matrix D = (*this);
      Matrix T(N, N);
      Matrix V(N, N);
      T.Identity();
      V.Identity();

      for (int k = 0; k < 3; k++) {
        for (int i = 0; i < N; i++) {
          for (int j = i + 1; j < N; j++) {

            if (D[i][j] && i != j) {

              double theta = (D[j][j] - D[i][i]) / (2 * D[i][j]);
              double t = 1 / (fabs(theta) + sqrt(theta * theta + 1));
              double c = 1 / (sqrt(theta * theta + 1));
              double s = t * c;

              if (theta < 0)
                t = -t;
              double tau = s / (1 + c);

              Matrix R = D;
              for (int r = 0; r < N; r++) {

                if (r == i)
                  D[i][i] = R[i][i] - t * R[i][j];
                else
                  D[i][r] /*=D[r][i]*/ =
                      R[r][i] - s * (R[r][j] + tau * R[r][i]);
                if (r == j)
                  D[j][j] = R[j][j] + t * R[i][j];
                else
                  D[j][r] /*=D[r][j]*/ =
                      R[r][j] + s * (R[r][i] - tau * R[r][j]);
              }
              R = V;
              for (int r = 0; r < N; r++) {
                V[r][i] = c * V[r][i] - s * V[r][j];
                V[r][j] = s * V[r][i] + c * V[r][j];
              }
            }

            D.Output();
            std::cout << std::endl;
          }
        }
      }

      (*this).Output();
      std::cout << std::endl;
      D.Output();
      std::cout << std::endl;
      V.Output();
      std::cout << std::endl;
      ((V)*D * (~V)).Output();
      std::cout << std::endl;
      std::cout << std::endl;
    }

    if (M == 3) {
      double temp;
      e_val = EigenVal();

      if (e_val[0] < e_val[2]) {
        temp = e_val[0];
        e_val[0] = e_val[2];
        e_val[2] = temp;
      }
      if (e_val[0] < e_val[1]) {
        temp = e_val[0];
        e_val[0] = e_val[1];
        e_val[1] = temp;
      } else if (e_val[1] < e_val[2]) {
        temp = e_val[1];
        e_val[1] = e_val[2];
        e_val[2] = temp;
      }

      if (debug)
        std::cout << e_val[0] << e_val[1] << e_val[2] << std::endl;

      if (e_val[0] != e_val[1]) {

        Matrix K;
        Matrix I(3, 3);
        I.Identity();
        K = (*this) - (I * e_val[0]);
        Matrix k1(N, 1);
        Matrix k2(N, 1);
        Matrix k3(N, 1);

        for (int m = 0; m < 3; m++) {
          k1[m][0] = K[m][0];
          k2[m][0] = K[m][1];
          k3[m][0] = K[m][2];
        }

        Matrix e_1;
        e_1 = k1.Cross(k2);

        if (e_1.Length() < epsilon) {
          e_1 = k1.Cross(k3);
        }
        e_1 = e_1 * (1 / e_1.Length());
        e_vec.push_back(e_1);

        if (e_val[1] != e_val[2]) {

          K = (*this) - (I * e_val[1]);
          Matrix k1(N, 1);
          Matrix k2(N, 1);
          Matrix k3(N, 1);

          for (int m = 0; m < 3; m++) {
            k1[m][0] = K[m][0];
            k2[m][0] = K[m][1];
            k3[m][0] = K[m][2];
          }

          Matrix e_2;
          e_2 = k1.Cross(k2);

          if (e_2.Length() < epsilon) {
            e_2 = k1.Cross(k3);
          }
          e_2 = e_2 * (1 / e_2.Length());

          Matrix e_3;
          e_3 = e_2.Cross(e_1);
          e_3 = e_2 * (1 / e_3.Length());

          e_vec.push_back(e_2);
          e_vec.push_back(e_3);
        } else {
          K = (*this) - (I * e_val[1]);
          Matrix k1(N, 1);
          Matrix k2(N, 1);
          Matrix k3(N, 1);

          for (int m = 0; m < 3; m++) {
            k1[m][0] = K[m][0];
            k2[m][0] = K[m][1];
            k3[m][0] = K[m][2];
          }

          k1 = k1 - (e_1) * ((((~k1) * e_1)[0][0]));

          if (k1.Length() > epsilon)
            k1 = k1 * (1 / k1.Length());
          else
            k1 = k1 * 0;

          k2 = k2 - (k1) * ((((~k2) * k1)[0][0])) -
               (e_1) * ((((~k2) * e_1)[0][0]));

          if (k2.Length() > epsilon)
            k2 = k2 * (1 / k2.Length());
          else
            k2 = k2 * 0;

          k3 = k3 - (k1) * ((((~k3) * k1)[0][0])) -
               (k2) * ((((~k2) * k3)[0][0])) - (e_1) * ((((~k3) * e_1)[0][0]));

          if (k1.Length() > epsilon)
            e_vec.push_back(k1);

          if (k2.Length() > epsilon)
            e_vec.push_back(k2);

          if (k3.Length() > epsilon) {
            k3 = k3 * (1 / k3.Length());
            e_vec.push_back(k3);
          }
        }
      } else if (e_val[1] != e_val[2]) {
        Matrix K;
        Matrix I(3, 3);
        I.Identity();
        K = (*this) - (I * e_val[0]);
        Matrix k1(N, 1);
        Matrix k2(N, 1);
        Matrix k3(N, 1);

        for (int m = 0; m < 3; m++) {
          k1[m][0] = K[m][0];
          k2[m][0] = K[m][1];
          k3[m][0] = K[m][2];
        }

        if (k1.Length() > epsilon)
          k1 = k1 * (1 / k1.Length());
        else
          k1 = k1 * 0;

        k2 = k2 - (k1) * ((((~k2) * k1)[0][0]));

        if (k2.Length() > epsilon)
          k2 = k2 * (1 / k2.Length());
        else
          k2 = k2 * 0;

        k3 = k3 - (k1) * ((((~k3) * k1)[0][0])) - (k2) * ((((~k2) * k3)[0][0]));


        if (k2.Length() > epsilon)
          e_vec.push_back(k2);

        if (k1.Length() > epsilon)
          e_vec.push_back(k1);

        if (k3.Length() > epsilon) {
          k3 = k3 * (1 / k3.Length());
          e_vec.push_back(k3);
        }

        Matrix e_3;

        if (e_vec.size() == 1) {

          Matrix K;
          Matrix I(3, 3);
          I.Identity();
          K = (*this) - (I * e_val[2]);
          Matrix k1(N, 1);
          Matrix k2(N, 1);
          Matrix k3(N, 1);

          for (int m = 0; m < 3; m++) {
            k1[m][0] = K[m][0];
            k2[m][0] = K[m][1];
            k3[m][0] = K[m][2];
          }

          e_3 = k1.Cross(k2);

          if (e_3.Length() < epsilon) {
            e_3 = k1.Cross(k3);
          }
          e_3 = e_3 * (1 / e_3.Length());

          Matrix e_2;
          e_2 = e_vec[0].Cross(e_3);
          e_2 = e_2 * (1 / e_2.Length());
          e_vec.push_back(e_2);
          e_vec.push_back(e_3);

        } else if (e_vec.size()) {
          Matrix e_3;
          e_3 = e_vec[0].Cross(e_vec[1]);
          e_3 = e_3 * (1 / e_3.Length());
          e_vec.push_back(e_3);

          if (e_val[1] == e_val[2])
            std::cout << "trouble" << std::endl;
        }
      }

      Matrix f(N, 1);

      f[0][0] = 0;
      e_vec.push_back(f);
      f[1][0] = 0;
      e_vec.push_back(f);
      f[2][0] = 0;
      e_vec.push_back(f);

  
    }
  } else {
    std::cout << "eigen value and vector determination for non-symmetric "
                 "matricies is unimplemented"
              << std::endl;
  }

  if (debug) {
    std::cout << "The eigenvectors are: " << std::endl;
    for (int i = 0; i < N; i++)
      e_vec[i].Output();
  }

  for (int i = 0; i < N; i++)
    for (int j = 0; j < N; j++)
      E[j][i] = (e_vec[i][j][0]);

  return E;
}

double *Matrix::EigenVal() {
  double *e_val;
  e_val = new double[3];
  if (N == M) {
    if (N == 3) {
      if (symmetric) {
        double coeff[3];
        double Root[3];
        coeff[0] = (*this)[0][1] * (*this)[0][1] * (*this)[2][2] +
                   (*this)[0][2] * (*this)[0][2] * (*this)[1][1] +
                   (*this)[1][2] * (*this)[1][2] * (*this)[0][0] -
                   2 * (*this)[0][2] * (*this)[0][1] * (*this)[1][2] -
                   (*this)[0][0] * (*this)[2][2] * (*this)[1][1];
        coeff[1] =
            -(*this)[0][1] * (*this)[0][1] - (*this)[0][2] * (*this)[0][2] -
            (*this)[2][1] * (*this)[2][1] + (*this)[1][1] * (*this)[0][0] +
            (*this)[0][0] * (*this)[2][2] + (*this)[1][1] * (*this)[2][2];

        coeff[2] = (*this)[0][0] + (*this)[1][1] + (*this)[2][2];

         coeff[2] = -coeff[2];
        double fA = Third * (3.0 * coeff[1] - coeff[2] * coeff[2]);
        double fB =
            TwentySeventh * (2.0 * coeff[2] * coeff[2] * coeff[2] -
                             9.0 * coeff[1] * coeff[2] + 27.0 * coeff[0]);
        double fOffset = Third * coeff[2];

        double fDiscr = 0.25 * fB * fB + TwentySeventh * fA * fA * fA;
        if (fabs(fDiscr) <= epsilon)
          fDiscr = 0.0;

        double fHalfB = 0.5 * fB;
        if (fDiscr < 0.0) {
          double fDist = sqrt(-Third * fA);
          double fAngle = Third * atan2(sqrt(-fDiscr), -fHalfB);
          double fCos = cos(fAngle);
          double fSin = sin(fAngle);
          Root[0] = 2.0 * fDist * fCos - fOffset;
          Root[1] = -fDist * (fCos + Sqrt3 * fSin) - fOffset;
          Root[2] = -fDist * (fCos - Sqrt3 * fSin) - fOffset;

        } else {
          double fTemp;
          if (fHalfB >= 0.0f)
            fTemp = -pow((double)fHalfB, (double)Third);
          else
            fTemp = pow((double)-fHalfB, (double)Third);
          Root[0] = 2.0f * fTemp - fOffset;
          Root[1] = -fTemp - fOffset;
          Root[2] = Root[1];
        }

        e_val[0] = Root[0];
        e_val[1] = Root[1];
        e_val[2] = Root[2];
        if (debug)
          std::cout << "Roots of char equation:" << Root[0] << " " << Root[1]
                    << " " << Root[2] << std::endl;
        if (debug)
          std::cout << "Coeffs for char equation:" << coeff[0] << " "
                    << coeff[1] << " " << coeff[2] << std::endl;
      }
    } else {
      std::cout << "eigen val for N!=3 is not implemented" << std::endl;
    }
  }
  return e_val;
}

Matrix Matrix::Cross(Matrix p) {

  Matrix k(M, 1);
  k[0][0] = (*this)[1][0] * p[2][0] - (*this)[2][0] * p[1][0];
  k[1][0] = (*this)[2][0] * p[0][0] - (*this)[0][0] * p[2][0];
  k[2][0] = (*this)[0][0] * p[1][0] - (*this)[1][0] * p[0][0];

  return k;
}

Matrix Matrix::LeastSquares(Matrix b) {

  Matrix temp;
  temp = (*this);

  temp = (((~(*this)) * (*this)).Inverse());
  temp = temp * (~(*this));

  temp = temp * b;

  return temp;
}

void Matrix::ProjectionOrthog(int i) {

  for (int j = 0; j < N; j++)
    row_maj[j][i - 1] = 0;
}

} // namespace Shapes
