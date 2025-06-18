#ifndef __UTIL__GEOMETRY_HPP__
#define __UTIL__GEOMETRY_HPP__

#include <cmath>
#include <util/point.hpp>
#include <util/range.hpp>

namespace util {
  const double pi      = 3.1415926535897932;
  const double half_pi = 1.5707963267948966;
  const double two_pi  = 6.2831853071795862;

  /* @brief Calculate the angle between two vectors.
     @param a The first vector.
     @param b The second vector.

     This routine takes two vectors, and returns the smaller of the
     two angles between them.  The calculation may degenerate if one
     of the vectors has a length close to zero.
  */
  template <class T>
  T angle(util::Point<T> a, util::Point<T> b) {
    a.normalise();
    b.normalise();
    return std::acos(a * b);
  }

  /*
    @brief Find the area of a triangle.
    @param a A vertex one the triangle.
    @param b A vertex one the triangle.
    @param c A vertex one the triangle.

    This routine finds the area of a triangle on the xy-plane.  The
    triangle does not need to be a right-triangle or axis-aligned.

    From http://www.btinternet.com/~se16/hgb/triangle.htm
  */
  template <class T>
  T planar_triangle_area(util::Point<T> a, util::Point<T> b, util::Point<T> c) {
    T ab = b.x() * a.y() - a.x() * b.y();
    T bc = c.x() * b.y() - b.x() * c.y();
    T ac = a.x() * c.y() - c.x() * a.y();

    T area = std::abs(ab + bc + ac) * 0.5;
    return area;
  }

  /* @brief Find the intersection of two lines.
     @param a1 A point on the first line.
     @param a2 Another point on the first line.
     @param b1 A point on the second line.
     @param b2 Another point on the second line.
     @param ia Set to true if the intersection is in the line segment a
     @param ib Set to true if the intersection is in the line segment b

     This routine finds the intersection of two lines in the xy-plane.
     It does not account for the case of parrallel or coincident
     lines.

     From htp://astronomy.swin.edu.au/~pbourke/geometry/lineline2d/
  */
  template <class T>
  util::Point<T> planar_line_intersection(util::Point<T> a1, util::Point<T> a2, util::Point<T> b1, util::Point<T> b2, bool& ia, bool& ib) {
    T x1 = a1.x();
    T x2 = a2.x();
    T x3 = b1.x();
    T x4 = b2.x();
    T y1 = a1.y();
    T y2 = a2.y();
    T y3 = b1.y();
    T y4 = b2.y();

    T d = (y4 - y3) * (x2 - x1) - (x4 - x3) * (y2 - y1);
    T ua = ((x4 - x3) * (y1 - y3) - (y4 - y3) * (x1 - x3)) / d;
    T ub = ((x2 - x1) * (y1 - y3) - (y2 - y1) * (x1 - x3)) / d;

    T x = x1 + ua * (x2 - x1);
    T y = y1 + ua * (y2 - y1);

    ia = util::range_open(0.0, 1.0, ua);
    ib = util::range_open(0.0, 1.0, ub);

    return util::Point<T>(x, y);
  }

  /* @brief Rotation of a vector in the xy-plane.
     @param v The vector from the origin to rotate.
     @param angle The angle to rotate by.

     This routine takes a vector from the origin in the xy-plane and
     rotates it by the supplied angle in the counter-clockwise
     direction.

     Sine, cosine and arctangent from the standard math library are
     used, so the type used in the template should be convertable to a
     plain floating-point type.

     This method came from the web, but I lost the url.
  */
  template <class T>
  util::Point<T> planar_rotation(const util::Point<T>& v, T angle) {
    T alpha = atan2(v.y(), v.x()) + angle;
    T length = v.length();
    return util::Point<T>(length * std::cos(alpha), length * std::sin(alpha));
  }
}

#endif
