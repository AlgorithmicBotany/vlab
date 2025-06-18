#ifndef __UTIL__FUNCTION_HPP__
#define __UTIL__FUNCTION_HPP__

#include "point.hpp"

#include <string>
#include <vector>

namespace util {
  /** @brief A utility class for functions.

  This class is a function object that encapsulates functions
  in the VLAB function formats (original and fver 1 1).
  */
  class Function {
  public:
    Function();
    Function(std::string filename);
    const Function& operator=(const Function&);
    double operator()(double x);
    const util::Point<double>& getMax() const;
    const util::Point<double>& getMin() const;
    void reread();
    void setSamples (size_t n);
    bool error();

    const std::string& getFilename() const { return filename; }

  private:
    std::string filename;
    std::vector<util::Point<double> > pts;
    util::Point<double> max;
    util::Point<double> min;
    unsigned int samples;

    util::Point<double> P(double x) const;
    double N(int, int, double) const;
    double Nk1(int, double) const;
    double Nkt(int, int, double) const;
    int    Uk(int) const;
    double getVal(double x) const; // computes the actual value for a given x

    struct CacheVal {
      bool valid;
      double val;
    };
    bool cache_valid;              // whether the cache vector is resized properly or not
    std::vector <CacheVal> cache;  // stores the cached values
    void init();                   // should be executed by every constructor
    bool error_occured;            // when loading this indicates whether there was an error
  };
}

#endif
