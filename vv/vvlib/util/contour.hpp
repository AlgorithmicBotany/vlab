#ifndef __UTIL__CONTOUR_HPP__
#define __UTIL__CONTOUR_HPP__

#include <string>
#include <vector>
#include "point.hpp"

namespace util {
  /** @brief Contour utility class.

      The Contour class encapsulates b-spline contours sepcified in
      the VLAB contour formats.  Currently all versions are supported
      (original, cver 1 1 and 1.4).  Instances of Countour behave as
      function objects.
  */
  class Contour {
  public:
    Contour();
    Contour(std::string filename);
    const Contour& operator=(const Contour&);
    util::Point<double> operator()(double t) const;
    const util::Point<double>& getMax() const;
    const util::Point<double>& getMin() const;

    double length(double a, double b, double dt = 0.01);
    double travel(double t, double l, double dt = 0.01);

    util::Point<double> tangent(double t, double dt = 0.01);
    util::Point<double> normal(double t, double dt = 0.01);

    void reread();

    const std::string& getFilename() const { return filename; }

  private:
    std::string filename;

    double Basis0(double t) const;
    double Basis1(double t) const;
    double Basis2(double t) const;
    double Basis3(double t) const;

    std::vector<util::Point<double> > pts;
    util::Point<double> max;
    util::Point<double> min;

    bool closed;
  };
}

#endif
