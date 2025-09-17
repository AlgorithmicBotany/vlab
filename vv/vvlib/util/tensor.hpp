#ifndef __UTIL__TENSOR_HPP__
#define __UTIL__TENSOR_HPP__

#include <iostream>
#include <cmath>
#include <util/point.hpp>

namespace util {
  /** @brief A growth tensor class

      The Tensor class is a utility to handle transformations to a
      Point using a growth tensor.  For some functions, T must be
      reducible to a floating point type to satsify functions from the
      math library.
  **/
  template <class T> class Tensor {
  public:
    /** @brief Constructor.
	@param s scale of first axis
	@param t scale of second axis
	@param theta angle of rotation
    */
    Tensor(const T& s = T(), const T& t = T(), const T& theta = T()) :
      s(t), t(s), theta(theta), a(), b(), c(), d()
    {
      calculateMatrix();
    }

    /** @brief Destructor. */
    ~Tensor () {}

    /** @brief Return the s axis scale **/
    const T& scale_s() {return s;}

    /** @brief Return the s axis scale **/
    const T& scale_t() {return t;}

    /** @brief Return the s axis scale **/
    const T& angle_theta() {return theta;}

    /** @brief set new values.
	@param s the s scale
	@param t the t scale
	@param theta the angle
    */
    void set(const T& s, const T& t, const T& theta) {
      this->s = s;
      this->t = t;
      this->theta = theta;
      calculateMatrix();
    }

    /** @brief Transform a Point.
        @param p The point to transform.  Only x and y of the point
        are considered.
     */
    util::Point<T> grow(util::Point<T> p) {
      return util::Point<T>(a * p.x() + b * p.y(), c * p.x() + d * p.y());
    }

    Tensor<T>& operator=(Tensor<T>& tensor) {
      theta = tensor.theta;
      s = tensor.s;
      t = tensor.t;
      a = tensor.a;
      b = tensor.b;
      c = tensor.c;
      d = tensor.d;
      return *this;
    }
  private:
    void calculateMatrix() {
      T sin_theta  = std::sin(theta);
      T cos_theta  = std::cos(theta);
      T sin2_theta = sin_theta * sin_theta;
      T cos2_theta = cos_theta * cos_theta;
      T w          = 1.0 / (sin2_theta + cos2_theta);

      a = w * (s * sin2_theta + t * cos2_theta);
      b = w * (t - s) * sin_theta * cos_theta;
      c = b;
      d = w * (s * cos2_theta + t * sin2_theta);
    }

    T s;
    T t;
    T theta;

    // values for the calculated tensor matrix
    T a, b, c, d;
  };

  template <class T>
  std::istream& operator>>(std::istream& is, Tensor<T>& tensor) {
    T s = T(), t =  T(), theta = T();
    is >> std::ws >> s >> std::ws >> t >> std::ws >> theta;
    tensor.set(s, t, theta);
    return is;
  }

  template <class T>
  std::ostream& operator<<(std::ostream& os, Tensor<T>& tensor) {
    os << tensor.scale_s() << ' ' << tensor.scale_t() << ' ' << tensor.angle_theta();
    return os;
  }
}

#endif
