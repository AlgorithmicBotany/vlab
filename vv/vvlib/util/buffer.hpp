#ifndef __UTIL__BUFFER_HPP__
#define __UTIL__BUFFER_HPP__

#include <stdexcept>
#include <iostream>

namespace util {
  /** @brief A ranged checked array.
      @param T    The type to store.
      @param size The size of the array.
  */
  template <class T, unsigned int size> class Buffer {
  public:
    Buffer(const T& val = T());
    virtual ~Buffer();

    T& operator[](unsigned int i) throw (std::out_of_range);
    T* c_data();

  private:
    T data[size];
  };

  /** @brief Stream input to the array.
      @param is The input stream.
      @param b  The Buffer object to write into.

      The following stream input reads 'size' elements, assuming that
      elements are seperated by whitespace as defined by std::ws.  For
      this to work, T must have an implemented input stream operator.
  */
  template <class T, unsigned int size>
  std::istream& operator>>(std::istream& is, Buffer<T, size>& b) {
    for (unsigned int i = 0; i < size; ++i)
      is >> std::ws >> b[i];
    return is;
  }

  /** @brief Stream output to the array.
      @param os The output stream.
      @param b  The Buffer object to read from.

      The following stream output writes 'size' elements, assuming
      that elements are seperated by a single space.  For this to
      work, T must have an implemented output stream operator.
  */
  template <class T, unsigned int size>
  std::ostream& operator<<(std::ostream& os, Buffer<T, size>& b) {
    for (unsigned int i = 0; i < size; ++i) {
      if (i) os << " ";
      os << b[i];
    }
    return os;
  }
}

/** @brief Constructor.
    @param val The inital value for the elements of Buffer.
*/
template <class T, unsigned int size>
util::Buffer<T, size>::Buffer(const T& val) {
  for (unsigned int i = 0; i < size; i++) data[i] = val;
}

/** @brief Destructor. */
template <class T, unsigned int size>
util::Buffer<T, size>::~Buffer() {}

/** @brief Index operator.
    @param i The index to reference.

    The index operator is range checked.  If i is out of range,
    std::out_of_range is thrown.
*/
template <class T, unsigned int size>
T& util::Buffer<T, size>::operator[](unsigned int i) throw (std::out_of_range) {
  if (i < size) return data[i];
  else throw std::out_of_range("Buffer index is out of range.");
}

/** @brief Return the data as a C-style array.

    This function allows the data in Buffer to be passed as a pointer
    to a C-style array.  Like all C-style arrays, it should be used
    with caution as access is not checked.
*/
template <class T, unsigned int size>
T* util::Buffer<T, size>::c_data() {
  return data;
}

#endif
