#ifndef __QUATERNION_HPP
#define __QUATERNION_HPP

#ifndef __LPFGALL_H__
#error quaternion.hpp must be included after lpfgall.h
#endif // __LPFGALL_H__

#ifndef DEG2RAD
#define DEG2RAD(x) ((x)*M_PI/180.)
#endif // DEG2RAD

#ifndef RAD2DEG
#define RAD2DEG(x) ((x)*180./M_PI)
#endif // RAD2DEG

#include <cmath>

// ***** Global constants
static const double EPSILON = 1e-6;

// Quaternion class is templated over the type of the scalar field;
// since it uses V3<T>, only float and double really make sense.
template<class T>
struct Quaternion
{
  // ***** Members
  T scalar;
  V3<T> bivec;

  // ***** Constructors
  Quaternion(void) : scalar(0) , bivec(0,0,0) {}
  Quaternion(const T& _scalar, const V3<T>& _bivec = V3<T>(0,0,0))
    : scalar(_scalar) , bivec(_bivec) {}
  //Quaternion(const T& w, const T& x, const T& y, const T& z)
  //  : scalar(w) , bivec(x,y,z) {}
  //Quaternion(const Quaternion& q)
  //  : scalar(q.scalar) , bivec(q.bivec) {}

  // ***** Operators
  // Assignment
  Quaternion& operator=(const Quaternion& q)
  {
    scalar = q.scalar;
    bivec = q.bivec;
    return (*this);
  }

  // Quaternion multiplication
  Quaternion operator*(const Quaternion& q) const
  {
    return Quaternion(scalar * q.scalar - bivec * q.bivec,
		      scalar * q.bivec + q.scalar * bivec + bivec % q.bivec);
  }
  Quaternion& operator*=(const Quaternion& q)
  {
    return (*this) = (*this) * q;
  }

  // Scalar multiplication
  Quaternion operator*(const T& mult) const
  {
    return Quaternion(scalar * mult , bivec * mult);
  }
  Quaternion& operator*=(const T& mult)
  {
    return (*this) = (*this) * mult;
  }
  friend Quaternion operator*(const T& mult, const Quaternion& q)
  {
    return Quaternion(mult * q.scalar , mult * q.bivec);
  }

  // Quaternion division
  Quaternion operator/(const Quaternion& q) const
  {
    return (*this) * q.inverse();
  }
  Quaternion& operator/=(const Quaternion& q)
  {
    return (*this) = (*this) / q;
  }

  // Scalar division
  Quaternion operator/(const T& mult) const
  {
    return Quaternion(scalar / mult , bivec / mult);
  }
  Quaternion& operator/=(const T& mult)
  {
    return (*this) = (*this) / mult;
  }

  friend Quaternion operator/(const T& mult,const Quaternion& q)
  {
    return mult * q.inverse();
  }

  // Addition
  Quaternion operator+(const Quaternion& q) const
  {
    return Quaternion(scalar + q.scalar , bivec + q.bivec);
  }
  Quaternion& operator+=(const Quaternion& q)
  {
    return (*this) = (*this) + q;
  }

  // Subtraction
  Quaternion operator-(const Quaternion& q) const
  {
    return Quaternion(scalar - q.scalar , bivec - q.bivec);
  }
  Quaternion& operator-=(const Quaternion& q)
  {
    return (*this) = (*this) - q;
  }

  // Negation
  Quaternion operator-(void) const
  {
    return Quaternion(-scalar , -bivec);
  }

  // ***** Other methods
  // Norm and norm^2
  T norm2(void) const
  {
    return scalar * scalar + bivec * bivec;
  }
  T norm(void) const
  {
    return std::sqrt(norm2());
  }

  // Reversion (negates bivector part)
  Quaternion reverse(void) const
  {
    return Quaternion(scalar,-bivec);
  }
  Quaternion& revert(void)
  {
    bivec = -bivec;
    return (*this);
  }

  // Inverse = reverse / norm^2
  Quaternion inverse(void) const
  {
    return reverse() / norm2();
  }
  Quaternion& invert(void)
  {
    return (*this) = this->inverse();
  }

  // Normalization
  Quaternion normalized(void) const
  {
    Quaternion q(*this);
    return q.normalize();
  }
  Quaternion& normalize(void)
  {
    T length = norm();
    if(length > EPSILON)
      (*this) /= length;
    return (*this);
  }

  // Rotate vector by quaternion
  V3<T> rotateVector(const V3<T>& vec) const
  {
    return ((*this) * Quaternion(0,vec) * this->reverse()).bivec;
  }

  // Get the angle of the rotation represented by this quaternion
  T getAngle(void) const
  {
    return 2 * atan2(bivec.Length(),scalar);
  }

  // Get the axis (normalized) of the rotation represented by this quaternion
  V3<T> getAxis(void) const
  {
    V3<T> ans(bivec);
    if(ans.Length() > EPSILON)
      ans.Normalize();
    return ans;
  }
};

// ***** Typedefs
typedef Quaternion<float> Qf;
typedef Quaternion<double> Qd;

// ***** Some other useful functions
// axisAngle - create the quaternion that rotates about the given axis
// by the given angle (in degrees)
Qf axisAngle(V3f axis, float angle)
{
  // If the axis is very short, no rotation takes place
  if(axis.Length() < EPSILON)
    return Qf(1);

  angle = 0.5 * DEG2RAD(angle);
  return Qf(cos(angle) , sin(angle) * axis.Normalize());
}

// axisAngle - same as the two-argument version, but angle is
// the length of the axis
Qf axisAngle(const V3f& axis)
{
  return axisAngle(axis,axis.Length());
}

// rotateTo - returns the shortest quaternion that rotates from to to
Qf rotateTo(V3f from, V3f to)
{
  from.Normalize();
  to.Normalize();
  return axisAngle(from % to, RAD2DEG(atan2((from%to).Length(),from*to)));
}

// slrp - performs spherical linear interpolation between two quaternions
Qf slrp(const Qf& q1, const Qf& q2, float t)
{
  // omega is the angle on the hypersphere subtended by q1 and q2
  Qf q = q1 * q2.reverse();
  q.normalize();
  float omega = acos(q.scalar);

  // if q1 = q2, omega is zero, and the interpolation will return zero; catch this
  if(fabs(omega) < EPSILON) return q1;

  Qf slrp = sin((1-t) * omega) * q1 + sin(t * omega) * q2;
  slrp.normalize();
  return slrp;
}

#endif // __QUATERNION_HPP
