#ifndef __UTIL__CLAMP_HPP__
#define __UTIL__CLAMP_HPP__

namespace util {
  /** @brief A function to clamp a value to a range.
      @param val The start value.
      @param min The minimum value of the range.
      @param max The maximum value of the range.

      If min is more than max, the function returns max.
  */
  template <class T> T clamp(const T& val, const T& min, const T& max) {
    if (min >= max) return max;
    else if (val < min) return min;
    else if (val > max) return max;
    else return val;
  }
}

#endif
