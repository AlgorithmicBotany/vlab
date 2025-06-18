#ifndef _VVLIB_UTIL_MATRIX_H
#define _VVLIB_UTIL_MATRIX_H

#include <util/vector.hpp>
#include <util/static_assert.hpp>
#include <cmath>

namespace util
{
  template <size_t nRows,size_t nCols,typename T = double>
    class Matrix
      {
    private:
      Vector<nCols,T> rows[nRows];

    public:
      Matrix(void)
        {
        for( size_t i = 0 ; i < nRows ; i++ )
          for( size_t j = 0 ; j < nCols ; j++ )
            rows[ i ][ j ] = 0;
        }

      Matrix(const Matrix& mat)
        {
        for(size_t i = 0 ; i < nRows ; i++)
          rows[i] = mat.rows[i];
        }

      Matrix(const Vector<nCols,T>* vecs)
        {
        for(size_t i = 0 ; i < nRows ; i++)
          rows[i] = vecs[i];
        }

      Matrix( const T* values, bool c_style = true)
        {
        if(c_style)
          {
          for(size_t i = 0 ; i < nRows ; i++)
            {
            rows[i] = Vector<nCols,T>(values[i*nCols]);
            }
          }
        else
          {
          for(size_t i = 0 ; i < nRows ; i++)
            for(size_t j = 0 ; j < nCols ; j++)
              {
              rows[i][j] = values[i+j*nRows];
              }
          }
        }

      Matrix( const T& value)
        {
        for(size_t i = 0 ; i < nRows ; i++)
          for(size_t j = 0 ; j < nCols ; j++)
            rows[i][j] = (i==j)?value:0;
        }

      Matrix operator-(void) const
        {
        Matrix ans;

        for(size_t i = 0 ; i < nRows ; i++)
          ans.rows[i] = -rows[i];

        return ans;
        }

      Matrix operator+(const Matrix& mat) const
        {
        Matrix ans;

        for(size_t i = 0 ; i < nRows ; i++)
          ans.rows[i] = rows[i] + mat.rows[i];

        return ans;
        }

      Matrix operator-(const Matrix& mat) const
        {
        Matrix ans;

        for(size_t i = 0 ; i < nRows ; i++)
          ans.rows[i] = rows[i] - mat.rows[i];

        return ans;
        }

      Matrix operator*(const T& scalar) const
        {
        Matrix ans;

        for(size_t i = 0 ; i < nRows ; i++)
          ans[i] = rows[i] * scalar;

        return ans;
        }

      Matrix operator/(const T& scalar) const
        {
        Matrix ans;

        for(size_t i = 0 ; i < nRows ; i++)
          ans[i] = rows[i] / scalar;

        return ans;
        }

      friend Matrix operator*(const T& scalar,const Matrix& mat)
        {
        Matrix ans;

        for(size_t i = 0 ; i < nRows ; i++)
          ans[i] = scalar * mat.rows[i];

        return ans;
        }

      Vector<nRows,T> operator*(const Vector<nRows,T>& vec) const
        {
        Matrix<nRows,1,T> mat;
        for(size_t i = 0; i < nRows; i++)
          mat[i][0] = vec[i];
        mat = (*this) * mat;
        Vector<nRows, T> ans;
        for(size_t i = 0; i < nRows; i++)
          ans[i] = mat[i][0];

        return ans;
        }

      Matrix& operator=(const Matrix& mat)
        {
        for(size_t i = 0 ; i < nRows ; i++)
          rows[i] = mat.rows[i];

        return (*this);
        }

      Matrix& operator+=(const Matrix& mat)
        {
        return ((*this) = (*this) + mat);
        }

      Matrix& operator-=(const Matrix& mat)
        {
        return ((*this) = (*this) - mat);
        }

      Matrix& operator*=(const T& scalar)
        {
        return ((*this) = (*this) * scalar);
        }

      Matrix& operator/=(const T& scalar)
        {
        return ((*this) = (*this) / scalar);
        }

      Matrix& operator*=(const Matrix& mat)
        {
        return ((*this) = (*this) * mat);
        }

      bool operator==(const Matrix& mat) const
        {
        for(size_t i = 0 ; i < nRows ; i++)
          if(rows[i] != mat.rows[i])
            return false;

        return true;
        }

      bool operator!=(const Matrix& mat) const
        {
        return (!((*this) == mat));
        }

      friend std::ostream& operator<<(std::ostream& out,const Matrix& mat)
        {
        for(size_t i = 0 ; i < nRows ; i++)
          {
          out << mat.rows[i];
          if(i != (nRows - 1))
            out << " ";
          }

        return out;
        }

      friend std::istream& operator>>(std::istream& in,Matrix& mat)
        {
        for(size_t i = 0 ; i < nRows && in ; i++)
          in >> mat.rows[i];
        return in ;
        }

      Vector<nCols,T>& operator[](size_t idx)
        {
        return rows[idx];
        }

      Vector<nCols,T> operator[](size_t idx) const
        {
        return rows[idx];
        }

      T& operator()(size_t i,size_t j)
        {
        return rows[i][j];
        }

      T operator()(size_t i,size_t j) const
        {
        return rows[i][j];
        }

      static Matrix identity()
        {
        Matrix mat;
        for(size_t i = 0 ; i < nRows ; i++)
          for(size_t j = 0 ; j < nCols ; j++)
            if(i == j)
              mat(i,j) = 1.0;
            else
              mat(i,j) = 0.0;
        return mat;
        }

      Matrix& zero(void)
        {
        for(size_t i = 0 ; i < nRows ; i++)
          for(size_t j = 0 ; j < nCols ; j++)
            rows[i][j] = 0.0;
        return (*this);
        }

      Matrix& operator=( const T& value )
        {
        for( size_t i = 0 ; i < nRows ; ++i )
          {
          for( size_t j = 0 ; j < nCols ; ++j )
            {
            if( i == j )
              rows[ i ][ j ] = value;
            else
              rows[ i ][ j ] = 0;
            }
          }
        return *this;
        }

      Matrix operator~()
        {
        Matrix<nCols,nRows,T> t;
        for( size_t i = 0 ; i < nRows ; ++i )
          for( size_t j = 0 ; j < nCols ; ++j )
            t[ i ][ j ] = rows[ j ][ i ];
        return t;
        }

      static Matrix<3,3,T> rotation( const Vector<3, T>& direction, T angle )
        {
        T ca = std::cos( angle );
        T sa = std::sin( angle );
        Matrix<3,3,T> r;
        double x = direction.x();
        double y = direction.y();
        double z = direction.z();
        r[ 0 ].set( ca+(1-ca)*x*x,   (1-ca)*x*y-sa*z, (1-ca)*z*x+sa*y );
        r[ 1 ].set( (1-ca)*y*x+sa*z, ca+(1-ca)*y*y,   (1-ca)*z*y-sa*x );
        r[ 2 ].set( (1-ca)*x*z-sa*y, (1-ca)*y*z+sa*x, ca+(1-ca)*z*z );
        return r;
        }

      };

  template<size_t nRows,size_t size_tSize,size_t nCols,typename T>
    Matrix<nRows,nCols,T> operator*(const Matrix<nRows,size_tSize,T>& mat1,
                                    const Matrix<size_tSize,nCols,T>& mat2);

  template<typename T> T det(const Matrix<1,1,T>& mat);
  template<typename T> T det(const Matrix<2,2,T>& mat);
  template<typename T> T det(const Matrix<3,3,T>& mat);
  template <size_t nRows, typename T> T det(const Matrix<nRows,nRows,T>& mat);

  template <size_t nRows, typename T>
    T matrix_minor(const Matrix<nRows,nRows,T>& mat, size_t i, size_t j);

  template <size_t nRows, typename T>
    T cofactor(const Matrix<nRows,nRows,T>& mat, size_t i, size_t j);

  template <typename T>
    Matrix<1,1,T> inverse(const Matrix<1,1,T>& mat);
  template <typename T>
    Matrix<2,2,T> inverse(const Matrix<2,2,T>& mat);
  template <typename T>
    Matrix<3,3,T> inverse(const Matrix<3,3,T>& mat);
  template <size_t nRows, typename T>
    Matrix<nRows,nRows,T> inverse(const Matrix<nRows,nRows,T>& mat);

  template <size_t nRows, size_t nCols, typename T>
    Matrix<nCols,nRows,T> transpose( const Matrix<nRows, nCols, T>& mat);

  template <size_t nRows, size_t nCols, typename T>
    T norm(const Matrix<nRows,nCols,T>& mat);

  template <size_t nRows, size_t nCols, typename T>
    T normsq(const Matrix<nRows,nCols,T>& mat);

#include <util/matrix_impl.hpp>

}

#endif // __MATRIX_H
