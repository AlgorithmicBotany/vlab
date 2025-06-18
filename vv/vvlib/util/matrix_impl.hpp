#ifndef VVLIB_UTIL_MATRIX_IMPL_HPP
#define VVLIB_UTIL_MATRIX_IMPL_HPP

  template<size_t nRows,size_t intSize,size_t nCols,typename T>
Matrix<nRows,nCols,T> operator*(const Matrix<nRows,intSize,T>& mat1,
                                const Matrix<intSize,nCols,T>& mat2)
{
  Matrix<nRows,nCols,T> ans;
  for(size_t i = 0 ; i < nRows ; i++)
    for(size_t j = 0 ; j < nCols ; j++)
      {
      T acc = 0;
      for(size_t k = 0 ; k < intSize ; k++)
        acc += mat1[i][k] * mat2[k][j];
      ans[i][j] = acc;
      }
  return ans;
}

  template<typename T>
T det(const Matrix<1,1,T>& mat)
{
  return mat(0,0);
}

  template<typename T>
T det(const Matrix<2,2,T>& mat)
{
  return mat(0,0)*mat(1,1) - mat(0,1)*mat(1,0);
}

  template<typename T>
T det(const Matrix<3,3,T>& mat)
{
  return mat(0,0)*mat(1,1)*mat(2,2) + mat(0,1)*mat(1,2)*mat(2,0) + mat(0,2)*mat(1,0)*mat(2,1) -
    mat(0,0)*mat(1,2)*mat(2,1) - mat(0,1)*mat(1,0)*mat(2,2) - mat(0,2)*mat(1,1)*mat(2,0);
}

  template <size_t nRows, typename T>
T det(const Matrix<nRows,nRows,T>& mat)
{
  STATIC_ASSERT(nRows > 3);
  T acc = 0;
  for(size_t i = 0 ; i < nRows ; i++)
    {
    acc += mat(i,0) * cofactor(mat,i,0);
    }
  return acc;
}

  template <size_t nRows, typename T>
T matrix_minor(const Matrix<nRows,nRows,T>& mat, size_t i, size_t j)
{
  STATIC_ASSERT(nRows>0);
  Matrix<nRows-1,nRows-1,T> ans;
  for(size_t i1 = 0, i2 = 0 ; i1 < nRows ; i1++, i2++)
    {
    if( i1 == i )
      {
      i2--;
      }
    else
      {
      for(size_t j1=0, j2 = 0 ; j1 < nRows ; j1++, j2++)
        {
        if(j1 == j)
          {
          j2--;
          }
        else
          {
          ans(i2,j2) = mat(i1,j1);
          }
        }
      }
    }
  return det(ans);
}

  template <size_t nRows, typename T>
T cofactor(const Matrix<nRows,nRows,T>& mat, size_t i, size_t j)
{
  T inv = 1;
  if((i+j) % 2)
    {
    inv = -1;
    }
  return inv*matrix_minor(mat, i, j);
}

  template <typename T>
Matrix<1,1,T> inverse(const Matrix<1,1,T>& mat)
{
  Matrix<1,1,T> ans;
  ans[0][0] = 1/mat(0,0);
  return ans;
}

  template <typename T>
Matrix<2,2,T> inverse(const Matrix<2,2,T>& mat)
{
  Matrix<2,2,T> ans;
  T d;
  d = det(mat);
  if(d == 0)
    return ans;
  ans(0,0) = mat(1,1) / d;
  ans(0,1) = mat(0,1) / -d;
  ans(1,0) = mat(1,0) / -d;
  ans(1,1) = mat(0,0) / d;
  return ans;
}

  template <size_t nRows, typename T>
Matrix<nRows,nRows,T> inverse(const Matrix<nRows,nRows,T>& mat)
{
  Matrix<nRows,nRows,T> ans;
  T d = det(mat);
  if(d == 0)
    return ans;
  for(size_t i = 0 ; i < nRows ; ++i)
    for(size_t j = 0 ; j < nRows ; ++j)
      {
      ans(i,j) = cofactor(mat,j,i)/d;
      }
  return ans;
}

  template <size_t nRows, size_t nCols, typename T>
Matrix<nCols,nRows,T> transpose( const Matrix<nRows, nCols, T>& mat)
{
  Matrix<nCols,nRows,T> ans;
  for(size_t i = 0 ; i < nRows ; i++)
    for(size_t j = 0 ; j < nCols ; j++)
      ans(j,i) = mat(i,j);
  return ans;
}

template <size_t nRows, size_t nCols, typename T>
T normsq(const Matrix<nRows,nCols,T>& mat)
{
  T acc = 0;
  for(size_t i = 0 ; i < nRows ; i++)
    for(size_t j = 0 ; j < nCols ; j++)
      {
      const T& v = mat(i,j);
      acc += v*v;
      }
  return acc;
}

  template <size_t nRows, size_t nCols, typename T>
T norm(const Matrix<nRows,nCols,T>& mat)
{
  return std::sqrt(normsq(mat));
}

#endif // VVLIB/UTIL/MATRIX_IMPL_HPP

