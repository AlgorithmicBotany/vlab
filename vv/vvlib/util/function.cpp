#include "function.hpp"

#include <iostream>
#include <fstream>
#include <cassert>

using namespace std;

/** @brief Initialization function that should be called from every constructor
 */
void util::Function::init()
{
  samples = 100000;
  cache_valid = false;
  error_occured = false;
}

/** @brief Default constructor. */
util::Function::Function() :
  pts(),
  max(),
  min()
{
  init();
}

/** @brief Constructor with initialisation from file.
    @bug   If the specified file contains an incomplete or improper
           specification, the result of construction will be
           unknown.  Most probably, the object will unuseable;
           however, this is not currently reported.
    @param filename The function file to load.
*/
util::Function::Function(std::string filename) :
  filename(filename),
  pts(),
  max(),
  min()
{
  init();
  reread();
}

/** @brief Copy constructor. */
const util::Function& util::Function::operator=(const util::Function& f) {
  filename = f.filename;
  pts = f.pts;
  max = f.max;
  min = f.min;
  samples = f.samples;
  cache_valid = f.cache_valid;
  cache = f.cache;
  error_occured = f.error_occured;
  return *this;
}

bool util::Function::error() {
  return error_occured;
}

void util::Function::reread() {
  ifstream in(filename.c_str());
  string buffer;

  if (!in || !in.good() || in.eof()) {
    error_occured = true;
    std::cerr << "util::Function - Cannot open file '" << filename << "'\n";
    throw (std::string ("Cannot open file ") + filename);
    return;
  }
  error_occured = false;

  in >> ws >> buffer;

  if (buffer == string("range:")) {
    // old version

    double rmin, rmax;
    in >> rmin >> rmax;
  }
  else if (buffer == string("fver")) {
    int major, minor;
    in >> major >> minor >> ws;

    if (major == 1 && minor == 1) {
      getline(in, buffer); // name
      getline(in, buffer); // samples
      getline(in, buffer); // flip
    }
  }

  unsigned int num;
  in >> buffer >> num;

  pts.reserve(num);

  bool first = true;
  for (unsigned int i = 0; i < num; i++) {
    double x, y;
    in >> x >> y;

    if (first) {
      max.set(x, y, 0);
      min.set(x, y, 0);
      first = false;
    }
    else {
      if (x > max.x()) max.x(x);
      if (y > max.y()) max.y(y);
      if (x < min.x()) min.x(x);
      if (y < min.y()) min.y(y);
    }

    util::Point<double> p(x, y);
    pts.push_back(p);
  }

  cache_valid = false;
}

/** @brief sets the number of samples to be used
    @param n The number of samples to use
    
    This sets the new number of samples and invalidates the cache.
*/
void util::Function::setSamples (size_t n)
{
  samples = (unsigned int)n;
  cache_valid = false;
}

/** @brief Function operator.
    @param x The position in the function.

    This returns the y-value of the function for a given x-value.  If x is outside the domain of
    the function, the value of the function start or end point is returned. Dynamic caching is
    used to speed things up. Basically, the function is only evaluated at a number of places
    (specified in 'samples'), and otherwise the values are linearly interpolated inbetween. The
    real values are obtained by calling getVal ().
*/
double util::Function::operator()(double x) {
  if (x <= min.x()) return pts[0].y();
  if (x >= max.x()) return pts[pts.size() - 1].y();

  // check for cache
  if (! cache_valid) {
    cache_valid = true;
    cache.resize (samples+1);
    for (size_t i = 0 ; i < samples ; i ++)
      cache[i].valid = false;
  }
  // lower index
  double dx = (max.x() - min.x()) / samples;
  size_t lo = (size_t) ((x - min.x()) / dx);
  size_t hi = lo + 1;
  // make sure we have both lo and hi
  double lox = min.x() + dx * lo;
  double hix = lox + dx;
  if (! cache [lo].valid) {
    cache [lo].valid = true;
    cache [lo].val = getVal (lox);
  }
  if (! cache [hi].valid) {
    cache [hi].valid = true;
    cache [hi].val = getVal (hix);
  }
  // linearly interpolate
  double r = (x - lox) / dx;
  double ret = (1-r) * cache[lo].val + r * cache[hi].val;

  return ret;
}

/** @brief Function operator.
    @param x The position in the function.

    This returns the y-value of the function for a given x-value. The parameter x is assumed to be
    between minx and maxx.
*/
double util::Function::getVal (double x) const {
  const double tofind = x;
  const double MaxError = 0.00001; // TBD: this should be user controlled
  double low = 0.0;
  double high = 1.0;
  double check = 0.5;
  util::Point<double> tst;
  int counter = 0;
  do {
    check = (low + high) / 2.0;
    tst = P(check);
    if (tst.x() > tofind)
      high = check;
    else 
      low = check;
    counter++;
  } while (fabs(tst.x() - tofind) > MaxError);
  return tst.y();
}

/** @brief P basis function */
util::Point<double> util::Function::P(double x) const {
  const int n = (unsigned int)pts.size() - 1;
  const int t = 4;
  double u = x * (double(n - t) + 2.0);

  util::Point<double> sum;

  for (int k = 0; k <= n; k++) {
    double coeff = N(k, t, u);
    sum += pts[k] * coeff;
  }

  return sum;
}

/** @brief A basis function. */
double util::Function::N(int k, int t, double u) const {
  double res = 0.0;
  if (1==t)
    res = Nk1(k, u);
  else
    res = Nkt(k, t, u);
  return res;
}


/** @brief A basis function. */
double util::Function::Nk1(int k, double u) const {
  if (Uk(k) <= u)
    {
      if (u < Uk(k + 1))
	return 1.0;
    }
  return 0.0;
}

/** @brief A basis function. */
double util::Function::Nkt(int k, int t, double u) const {
  double sum = 0.0;
  int div = Uk(k + t - 1) - Uk(k);
  if (0 != div)
    sum = (u - Uk(k)) / div * N(k, t - 1, u);

  div = Uk(k + t) - Uk(k + 1);
  if (0 != div)
    sum += (Uk(k + t) - u) / div * N(k + 1, t - 1, u);

  return sum;
}

/** @brief A basis function. */
int util::Function::Uk(int j) const {
  const int n = (unsigned int)pts.size() - 1;
  const int t = 4;
  if (j < t)
    return 0;
  if (j > n)
    return n - t + 2;
  return j - t + 1;
}

/** @brief Return the maximum x and y values. */
const util::Point<double>& util::Function::getMax() const {
  return max;
}

/** @brief Return the minimum x and y values. */
const util::Point<double>& util::Function::getMin() const {
  return min;
}
