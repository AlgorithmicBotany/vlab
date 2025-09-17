#ifndef __UTIL__POINT_H__
#define __UTIL__POINT_H__

#include <iostream>
#include <cmath>
#include <util/vector.hpp>

namespace util {
  /** @brief A 3D point/vector class.

      The Point class is a utility class tp handle operations on
      three-dimensional xyz-points and vectors.  For some functions,
      T must be reducible to a floating point type to satisfy the
      functions used in the math library.
  */
  template <class T> class Point : public Vector<3,T> {
  public:
    /** @brief Constructor. */
    Point( const Vector<3,T>& v ) : Vector<3,T>( v ) {}
    Point(const T& x = T(), const T& y = T(), const T& z = T())
     : Vector<3,T>( x,y,z ) { }
    /** @brief Destructor. */
    virtual ~Point() {}

    Point<T> cross(const Point<T>& p) const { return this->Vector<3,T>::cross( Vector<3,T>(p) ) ;}
    Point<T> proj(const Point<T>& p) const;
    T        proj_length(const Point<T>& p) const;
    void normalise() { this->normalize(); }
    void normalise(T l) { this->normalize(); ( *this ) *= l; }
    T distance(const Point<T>& p) const { return ( *this - p ).norm(); }
    T distance_sq(const Point<T>& p) const { return ( *this - p ).normsq(); }
    T length() const { return this->norm(); }
    T length_sq() const { return this->normsq(); }

    Point& operator+=( const Point& p ) { this->Vector<3,T>::operator+=( p ); return *this; }
    Point& operator-=( const Point& p ) { this->Vector<3,T>::operator-=( p ); return *this; }
    Point& operator*=( const T& p ) { this->Vector<3,T>::operator*=( p ); return *this; }
    Point& operator/=( const T& p ) { this->Vector<3,T>::operator/=( p ); return *this; }
    Point operator+( const Point& p ) const { return this->Vector<3,T>::operator+( p ); }
    Point operator-( const Point& p ) const { return this->Vector<3,T>::operator-( p ); }
    Point operator/( const T& p ) const { return this->Vector<3,T>::operator/( p ); }
    Point operator-() const { return this->Vector<3,T>::operator-(); }
    /** @brief Return the data as a C-style array. */
    inline const T* c_data() const {return this->elems;}
  };

  /** @brief Scalar multiplication operator.
      @param p The point to scale.
      @param s The scaling value.
  */
  template <class T>
  Point<T> operator* (const Point<T>& p, const T& s) {
    Point<T> r( p.Vector<3,T>::operator*( s ) );
    return r;
  }

  template <class T>
  Point<T> operator* ( const T& s, const Point<T>& p) {
    return p * s;
  }


  /** @brief Vector projection.
      @param p The vector projected onto.
  */
  template <class T>
  Point<T> Point<T>::proj(const Point<T>& p) const {
    return ((*this * p) / p.length_sq()) * p;
  }

  /** @brief Vector projection length.
      @param p The vector projected onto.
  */
  template <class T>
  T Point<T>::proj_length(const Point<T>& p) const {
    return fabs(*this * p) / p.length();
  }

}

#endif
