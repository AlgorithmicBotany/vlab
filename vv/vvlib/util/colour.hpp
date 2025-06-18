#ifndef __UTIL__COLOUR_H__
#define __UTIL__COLOUR_H__

#include <util/vector.hpp>
#include <util/clamp.hpp>

namespace util {
  /** @brief A utility class to encapsulate colour data. */
  template<class T> class Colour : public Vector<4, T> {
  public:
    /** @brief Constructor.
	@param r Value for red.
	@param g Value for green.
	@param b Value for blue.
	@param a Value for alpha.
    */
    Colour(const T& r = T(), const T& g = T(), const T& b = T(), const T& a = T())
		: Vector<4,T>(r,g,b,a) { }

    /** @brief Return the red component. */
    inline const T& r() const {return this->x();}

    /** @brief Return the green component. */
    inline const T& g() const {return this->y();}

    /** @brief Return the blue component. */
    inline const T& b() const {return this->z();}

    /** @brief Return the alpha component. */
    inline const T& a() const {return this->t();}

    /** @brief Set the red component. */
    inline void r(const T& val) {this->x(val);}

    /** @brief Set the green component. */
    inline void g(const T& val) {this->y(val);}

    /** @brief Set the blue component. */
    inline void b(const T& val) {this->z(val);}

    /** @brief Set the alpha component. */
    inline void a(const T& val) {this->t(val);}

    /** @brief Set a minimum colour value.

        This is enforced only when clamp() is called
    */
    void setMaxValue(const T& val) {max_value = val;}

    /** @brief Set a maximum colour value.

        This is enforced only when clamp() is called
    */
    void setMinValue(const T& val) {min_value = val;}

    void clamp();

    Colour<T>& operator= (const Colour<T>& c);

  protected:
    T max_value;
    T min_value;
  };

  /** @brief Clamp the values of the colour to the specified range. */
  template <class T>
  void Colour<T>::clamp() {
    set(util::clamp<T>(r(), min_value, max_value),
		util::clamp<T>(g(), min_value, max_value),
		util::clamp<T>(b(), min_value, max_value),
		util::clamp<T>(a(), min_value, max_value));
  }

  /** @brief Assignment of colour data. */
  template <class T>
  Colour<T>& Colour<T>::operator=(const Colour<T>& c) {
	this->Vector<4,T>::operator=(c);
	return *this;
  }

  /** @brief Return a colour based on HSV values. */
  template <class T>
  Colour<T> convertHSVtoRGB(T h, T s, T v) {
    // based on Jo's code in medit

    Colour<T> rgb;
    rgb.a(1.0);

    while (h > 360.0) h -= 360.0;
    while (h < 0.0) h += 360.0;

    h /= 60.0;

    int i = int(h);

    double f = h - i;
    double p = v * (1 - s);
    double q = v * (1 - (s * f));
    double t = v * (1 - (s * (1 - f)));

    switch (i) {
    case 0: rgb.r(v); rgb.g(t); rgb.b(p); break;
    case 1: rgb.r(q); rgb.g(v); rgb.b(p); break;
    case 2: rgb.r(p); rgb.g(v); rgb.b(t); break;
    case 3: rgb.r(p); rgb.g(q); rgb.b(v); break;
    case 4: rgb.r(t); rgb.g(p); rgb.b(v); break;
    case 5: rgb.r(v); rgb.g(p); rgb.b(q); break;
    }

    return rgb;
  }
}

#endif
