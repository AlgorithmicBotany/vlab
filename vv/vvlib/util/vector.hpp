#ifndef __VECTOR_H
#define __VECTOR_H

#include <iostream>
#include <cmath>
#include <cassert>
#include <cstdarg>
#include <util/static_assert.hpp>

namespace util
{
  /**
   * \class Vector vector.hpp <util/vector.hpp>
   *
   * Brief single line description.
   *
   * Complete description of the class
   */
  template <size_t dim,class T = double>
    class Vector
      {
    protected:
      T elems[dim];

    public:
      /**
       * Default constructor
       */
      Vector(void)
        {
        for(size_t i = 0 ; i < dim ; i++)
          elems[i] = 0;
        }

      Vector(const Vector& vec)
        {
        for(size_t i = 0 ; i < dim ; i++)
          elems[i] = vec.elems[i];
        }

      Vector(const T* el)
        {
        for(size_t i = 0 ; i < dim ; i++)
          elems[i] = el[i];
        }

      Vector( const T& x )
        {
        for( size_t i = 0 ; i < dim ; ++i )
          elems[ i ] = x;
        }

      Vector( const T& x, const T& y )
        {
        STATIC_ASSERT( dim == 2 );
        elems[ 0 ] = x;
        elems[ 1 ] = y;
        }

      Vector( const T& x, const T& y, const T& z )
        {
        STATIC_ASSERT( dim == 3 );
        elems[ 0 ] = x;
        elems[ 1 ] = y;
        elems[ 2 ] = z;
        }

      Vector( const T& x, const T& y, const T& z, const T& t )
        {
        STATIC_ASSERT( dim == 4 );
        elems[ 0 ] = x;
        elems[ 1 ] = y;
        elems[ 2 ] = z;
        elems[ 3 ] = t;
        }

      T* data() { return elems; }
      const T* c_data() const { return elems; }

      Vector operator-(void) const
        {
        Vector ans;
        for(size_t i = 0 ; i < dim ; i++)
          ans[i] = -elems[i];

        return ans;
        }

      Vector operator+(const Vector& vec) const
        {
        Vector ans;
        for(size_t i = 0 ; i < dim ; i++)
          ans[i] = elems[i] + vec.elems[i];

        return ans;
        }

      Vector operator-(const Vector& vec) const
        {
        Vector ans;
        for(size_t i = 0 ; i < dim ; i++)
          ans[i] = elems[i] - vec.elems[i];

        return ans;
        }

      Vector operator*(const T& scalar) const
        {
        Vector ans;
        for(size_t i = 0 ; i < dim ; i++)
          ans[i] = elems[i] * scalar;

        return ans;
        }

      Vector operator/(const T& scalar) const
        {
        Vector ans;
        for(size_t i = 0 ; i < dim ; i++)
          ans[i] = elems[i] / scalar;

        return ans;
        }

      friend Vector operator*(const T& scalar,const Vector& vec)
        {
        Vector ans;
        for(size_t i = 0 ; i < dim ; i++)
          ans[i] = scalar * vec.elems[i];

        return ans;
        }

      T operator*(const Vector& vec) const
        {
        T ans = 0;
        for(size_t i = 0 ; i < dim ; i++)
          ans += elems[i] * vec.elems[i];

        return ans;
        }

      Vector& operator=(const Vector& vec)
        {
        for(size_t i = 0 ; i < dim ; i++)
          elems[i] = vec.elems[i];

        return (*this);
        }

      Vector& operator+=(const Vector& vec)
        {
        return ((*this) = (*this) + vec);
        }

      Vector& operator-=(const Vector& vec)
        {
        return ((*this) = (*this) - vec);
        }

      Vector& operator*=(const T& scalar)
        {
        return ((*this) = (*this) * scalar);
        }

      Vector& operator/=(const T& scalar)
        {
        return ((*this) = (*this) / scalar);
        }

      bool operator==(const Vector& vec) const
        {
        for(size_t i = 0 ; i < dim ; i++)
          if(elems[i] != vec.elems[i])
            return false;

        return true;
        }

      bool operator!=(const Vector& vec) const
        {
        return (!((*this) == vec));
        }

      T& operator[](size_t idx)
        {
        return elems[idx];
        }

      T operator[](size_t idx) const
        {
        return elems[idx];
        }

      T norm() const
        {
        return std::sqrt(normsq());
        }

      T normsq() const
        {
        T ans = 0;
        for(size_t i = 0 ; i < dim ; i++)
          ans += elems[i] * elems[i];

        return ans;
        }

      Vector& normalize(void)
        {
        T sz = norm();
        return ((*this) /= sz);
        }

      Vector normalized(void) const
        {
        Vector ans(*this);
        return ans.normalize();
        }

      bool iszero(void)
        {
        for(size_t i = 0 ; i < dim ; i++)
          if(elems[i] != 0)
            return false;
        return true;
        }

      Vector& zero(void)
        {
        for(size_t i = 0 ; i < dim ; i++)
          elems[i] = 0;
        return (*this);
        }

      void set( const T& x )
        {
        STATIC_ASSERT( dim == 1 );
        elems[ 0 ] = x;
        }

      void set( const T& x, const T& y )
        {
        STATIC_ASSERT( dim == 2 );
        elems[ 0 ] = x;
        elems[ 1 ] = y;
        }

      void set( const T& x, const T& y, const T& z )
        {
        STATIC_ASSERT( dim == 3 );
        elems[ 0 ] = x;
        elems[ 1 ] = y;
        elems[ 2 ] = z;
        }

      void set( const T& x, const T& y, const T& z, const T& t )
        {
        STATIC_ASSERT( dim == 4 );
        elems[ 0 ] = x;
        elems[ 1 ] = y;
        elems[ 2 ] = z;
        elems[ 3 ] = t;
        }

      Vector& operator=( const T& value )
        {
        for( size_t i = 0 ; i < dim ; ++i )
          {
          elems[ i ] = value;
          }
        return *this;
        }

      /**
       * Compute the cross product as \c this x \c other
       */
      Vector cross( const Vector& other ) const
        {
        STATIC_ASSERT( dim == 3 );
        return ( *this ) ^ other;
        }


      void x(const T& v) { STATIC_ASSERT( dim > 0 ); elems[0] = v; }
      void y(const T& v) { STATIC_ASSERT( dim > 1 ); elems[1] = v; }
      void z(const T& v) { STATIC_ASSERT( dim > 2 ); elems[2] = v; }
      void t(const T& v) { STATIC_ASSERT( dim > 3 ); elems[3] = v; }
      T& x(void) { STATIC_ASSERT( dim > 0 ); return elems[0]; }
      T& y(void) { STATIC_ASSERT( dim > 1 ); return elems[1]; }
      T& z(void) { STATIC_ASSERT( dim > 2 ); return elems[2]; }
      T& t(void) { STATIC_ASSERT( dim > 3 ); return elems[3]; }
      const T& x(void) const { STATIC_ASSERT( dim > 0 ); return elems[0]; }
      const T& y(void) const { STATIC_ASSERT( dim > 1 ); return elems[1]; }
      const T& z(void) const { STATIC_ASSERT( dim > 2 ); return elems[2]; }
      const T& t(void) const { STATIC_ASSERT( dim > 3 ); return elems[3]; }

      Vector<2,T> projectXY(void)
        {
        STATIC_ASSERT( dim>1 );
        return Vector<2,T>(elems[0],elems[1]);
        }

      friend T norm( const Vector& v )
        {
        return v.norm();
        }

      friend T normsq( const Vector& v )
        {
        return v.normsq();
        }

      friend std::ostream& operator<<(std::ostream& out,const Vector& vec)
        {
        for(size_t i = 0 ; i < dim ; i++)
          {
          out << vec.elems[i];
          if(i != (dim - 1))
            out << " ";
          }
        return out;
        }

      friend std::istream& operator>>(std::istream& in,Vector& vec)
        {
        in >> vec[ 0 ];
        for(size_t i = 1 ; i < dim && in ; i++)
          in >> std::ws >> vec[i];
        return in;
        }

      };

  /**
   * Cross product \c v1 x \c v2
   */
  template <class T>
    T operator%( const Vector<2,T>& v1, const Vector<2,T>& v2 )
      {
      return v1^v2;
      }

  template <class T>
    T operator^( const Vector<2,T>& v1, const Vector<2,T>& v2 )
      {
      return ((v1[0] * v2[1]) -
              (v1[1] * v2[0]));
      }

  template <class T>
    T operator^( const Vector<1,T>& v1, const Vector<1,T>& v2 )
      {
      return 0;
      }

  template <class T>
    Vector<3,T> operator%(const Vector<3,T>& v1,const Vector<3,T>& v2)
      {
      return v1^v2;
      }

  template <class T>
    Vector<3,T> operator^(const Vector<3,T>& v1,const Vector<3,T>& v2)
      {
      Vector<3,T> ans;
      ans[0] = v1[1]*v2[2] - v1[2]*v2[1];
      ans[1] = v1[2]*v2[0] - v1[0]*v2[2];
      ans[2] = v1[0]*v2[1] - v1[1]*v2[0];

      return ans;
      }

  template <class T>
    double angle( const Vector<2,T>& v )
      {
      return atan2( v.y(), v.x() );
      }

  template <class T>
    double angle( const Vector<3,T>& v1, const Vector<3,T>& v2 )
      {
      double x = v1*v2;
      double y = norm( v1^v2 );
      return atan2( y, x );
      }

  template <class T>
    double angle( const Vector<2,T>& v1, const Vector<2,T>& v2 )
      {
      double x = v1*v2;
      double y = v1^v2;
      return atan2( y, x );
      }

  template <class T>
    double angle( const Vector<1,T>& v1, const Vector<1,T>& v2 )
      {
      return ( v1*v2 < 0 )? -1 : 1;
      }


  template <class T>
    double angle( const Vector<3,T>& v1, const Vector<3,T>& v2, const Vector<3,T>& ref )
      {
      double x = v1*v2;
      Vector<3,T> n = v1^v2;
      double y = norm( n );
      if( n*ref < 0 )
        return atan2( -y, x );
      else
        return atan2( y, x );
      }
}

#endif // __VECTOR_H
