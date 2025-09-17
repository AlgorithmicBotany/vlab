#ifndef __UTIL_RANGE_HPP__
#define __UTIL_RANGE_HPP__

namespace util {
  /** @brief Check if a value is in an closed range.
      @param min The minimum range value.
      @param max The maximum range value.
      @param val The value to check.
  */
  template <typename T> bool range_closed(const T& min, const T& max, const T& val) {
    return (val >= min && val <= max);
  }

  /** @brief Check if a value is in an open range.
      @param min The minimum range value.
      @param max The maximum range value.
      @param val The value to check.
  */
  template <typename T> bool range_open(const T& min, const T& max, const T& val) {
    return (val > min && val < max);
  }

  /** @brief Check if a value is in a C-style range (closed on lower bound and open on the upper)
      @param min The minimum range value.
      @param max The maximum range value.
      @param val The value to check.
  */
  template <typename T> bool range_c(const T& min, const T& max, const T& val) {
    return (val >= min && val < max);
  }
}

#endif
