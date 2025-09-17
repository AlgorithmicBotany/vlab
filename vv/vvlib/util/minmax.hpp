#ifndef __UTIL__MAXMIN_HPP__
#define __UTIL__MAXMIN_HPP__

#include <cstdarg>

namespace util {
  /** @brief Return the maximum of two values.
      @param a The first value.
      @param b The second value.
  */
  template <typename T> T maximum(const T& a, const T& b) {
    return (a > b) ? a : b;
  }

  /** @brief Return the minimum of two values.
      @param a The first value.
      @param b The second value.
  */
  template <typename T> T minimum(const T& a, const T& b) {
    return (a < b) ? a : b;
  }
}

#endif
