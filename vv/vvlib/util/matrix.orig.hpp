#ifndef __UTIL__MATRIX_HPP__
#define __UTIL__MATRIX_HPP__

#include <cassert>
#include <iostream>

namespace util {
  template <typename T, unsigned int rows = 2, unsigned int cols = rows>
  class Matrix {
  public:
    Matrix(T value = T()) {
      for (unsigned int row = 0; row < rows; ++row)
	for (unsigned int col = 0; col < cols; ++col)
	  r[row][col] = value;
    }

    struct Row {
      T entries[cols];
      T& operator[](unsigned int col) {
	assert(col < cols);
	return entries[col];
      }
    };

    Row& operator[](unsigned int row) {
      assert(row < rows);
      return r[row];
    }

    Matrix operator=(Matrix& m) {
      for (unsigned int row = 0; row < rows; ++row)
	for (unsigned int col = 0; col < cols; ++col)
	  r[row][col] = m[row][col];
    }

    Matrix operator+(Matrix& m) {
      Matrix x;
      for (unsigned int row = 0; row < rows; ++row)
	for (unsigned int col = 0; col < cols; ++col)
	  x[row][col] = r[row][col] + m[row][col];
      return x;
    }

    Matrix operator-(Matrix& m) {
      Matrix x;
      for (unsigned int row = 0; row < rows; ++row)
	for (unsigned int col = 0; col < cols; ++col)
	  x[row][col] = r[row][col] - m[row][col];
      return x;
    }

    Matrix operator*(T s) {
      Matrix x;
      for (unsigned int row = 0; row < rows; ++row)
	for (unsigned int col = 0; col < cols; ++col)
	  x[row][col] = r[row][col] * s;
      return x;
    }

    Matrix operator/(T s) {
      Matrix x;
      for (unsigned int row = 0; row < rows; ++row)
	for (unsigned int col = 0; col < cols; ++col)
	  x[row][col] = r[row][col] / s;
      return x;
    }

  private:
    Row r[rows];
  };

  template <typename T, unsigned int lr, unsigned int lc, unsigned int rc>
  Matrix<T, lr, rc> operator*(Matrix<T, lr, lc>& l, Matrix<T, lc, rc> r) {
    Matrix<T, lr, rc> x;

    return x;
  }

  template <typename T, unsigned int rows, unsigned int cols>
  std::ostream& operator<<(std::ostream& os, Matrix<T, rows, cols> m) {
    os << '{';
    for (unsigned int row = 0; row < rows; ++row) {
      os << '{';
      for (unsigned int col = 0; col < cols; ++col) {
	os << m[row][col];
	if (col != cols - 1) os << ", ";
      }
      os << '}';
      if (row != rows - 1) os << ", ";
    }
    os << '}';
    return os;
  }
}

#endif
