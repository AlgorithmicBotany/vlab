#include <cmath>
#include <fstream>
#include "contour.hpp"

using namespace std;

/** @brief Default constructor. */
util::Contour::Contour() {}

/** @brief Constructor from file.
    @bug   If the specified contains an incomplete or improper
           specification, the result of construction will be
           unknown.  Most probably, the object will unuseable;
           however, this is not currently reported.
    @param filename The contour file to load.
*/
util::Contour::Contour(string filename) :
  filename(filename)
{
  reread();
}

void util::Contour::reread() {
  int num = 0, dimensions = 0;
  bool checkz = false;
  closed = false;

  ifstream in(filename.c_str());
  if (!in || !in.good() || in.eof()) return;

  if (in.peek() == 'c') {
    // check cver 1 1
    string buff, name;
    int major = 0, minor = 0;
    in >> buff >> major >> minor >> ws;

    if (!(major == minor == 1)) return;

    in >> buff >> ws;
    getline(in, name);

    int num_total;
    in >> buff >> num >> num_total >> ws;

    string type;
    in >> buff >> type >> ws;

    if (type == string("closed")) closed = true;
    else closed = false;

    for (int i = 0; i < num; i++) {
      double x = 0.0, y = 0.0, z = 0.0;
      int m = 1;

      in >> x >> y >> z >> m >> ws;

      if (i == 0) {
	min.x(x);
	max.x(x);
	min.y(y);
	max.y(y);
      }
      else {
	if (x < min.x()) min.x(x);
	if (x > max.x()) max.x(x);
	if (y < min.y()) min.y(y);
	if (y > max.y()) max.y(y);
      }

      for (int j = 0; j < m; j++)
	pts.push_back(Point<double>(x, y)); 
    }
  }
  else if (in.peek() == 'v') {
    // check version 1.4
    string buff;

    in >> buff >> buff >> ws;
    if (buff != string("1.4")) return;

    in
      >> buff >> buff >> buff >> buff // contact
      >> buff >> buff >> buff >> buff // end
      >> buff >> buff >> buff >> buff // heading
      >> buff >> buff >> buff >> buff // up
      >> buff >> buff                 // size
      >> buff >> num                  // points
      >> buff >> buff >> buff         // range
      >> buff >> buff                 // dimension
      >> buff >> buff >> ws;          // type

    for (int i = 0; i < num; i++) {
      double x = 0.0, y = 0.0, z = 0.0;
      int m = 1;

      in >> x >> y >> z >> m >> ws;

      if (i == 0) {
	min.x(x);
	max.x(x);
	min.y(y);
	max.y(y);
      }
      else {
	if (x < min.x()) min.x(x);
	if (x > max.x()) max.x(x);
	if (y < min.y()) min.y(y);
	if (y > max.y()) max.y(y);
      }

      for (int j = 0; j < m; j++)
	pts.push_back(Point<double>(x, y)); 
    }
  }
  else {
    // original version
    string c;

    in >> num >> dimensions >> c >> ws;
    if (c == string("closed")) closed = true;

    if (dimensions == 3) checkz = true;

    for (int i = 0; i < num; i++) {
      double x = 0.0, y = 0.0, z = 0.0;
      int m = 1;
      in >> x >> y >> ws;

      in >> x >> y >> z >> m >> ws;

      if (i == 0) {
	min.x(x);
	max.x(x);
	min.y(y);
	max.y(y);
      }
      else {
	if (x < min.x()) min.x(x);
	if (x > max.x()) max.x(x);
	if (y < min.y()) min.y(y);
	if (y > max.y()) max.y(y);
      }

      pts.push_back(Point<double>(x, y)); 
      
      if (checkz) {
	double z;
	in >> z >> ws;
      }
    }
  }

  if (closed) {
    pts.push_back(pts[0]);
    pts.push_back(pts[1]);
    pts.push_back(pts[2]);
  }
}

/** @brief Copy constructor. */
const util::Contour& util::Contour::operator=(const util::Contour& c) {
  pts = c.pts;
  max = c.max;
  min = c.min;
  closed = c.closed;

  return *this;
}

/** @brief Function operator.
    @param t Time along the curve, between zero and one.

    This calculates the 3-dimensional coordinates of the curve
    at time t, where t is between zero and one.  If t is less then
    zero, it is set to zero.  If it is larger than one, it is set to one.
*/
util::Point<double> util::Contour::operator()(double t) const {
  if (t < 0.0) t = 0.0;
  else if (t > 1.0) t = 1.0;

  double k = 1.0 + double(pts.size() - 3) * t;
  int i = int(k) + 2;
  double p = k - floor(k);

  return
      pts[i - 3] * Basis0(p)
    + pts[i - 2] * Basis1(p)
    + pts[i - 1] * Basis2(p)
    + pts[i]     * Basis3(p);
}

/** @brief Return the maximum x, y and z coordinates. */
const util::Point<double>& util::Contour::getMax() const {
  return max;
}

/** @brief Return the minimum x, y and z coordinates. */
const util::Point<double>& util::Contour::getMin() const {
  return min;
}

/** @brief Return the arc length for a parameter interval.
    @param a Parameter for the start of the interval.
    @param b Parameter for the end of the interval.
    @param dt The step used for forward differencing.

    It is expected that a and b are both in the interval of [0, 1] and
    that dt is less than b - a.
*/
double util::Contour::length(double a, double b, double dt) {
  if (a < 0.0) a = 0.0;
  if (a > 1.0) a = 1.0;
  if (b > 1.0) b = 1.0;
  if (b < a + dt) return 0.0;

  double l = 0.0;
  util::Point<double> p = (*this)(a), q;
  while (a <= b) {
    a += dt;
    q = (*this)(a);
    l += p.distance(q);
    p = q;
  }
  return l;
}

/** @brief Return the paramter after traveling a given arc length.
    @param t Parameter for the start of the interval.
    @param l The arc length to travel.
    @param dt The step used for forward differencing.

    It is expected that t is in the interval of [0, 1] and that l is
    positive.
*/
double util::Contour::travel(double t, double l, double dt) {
  if (t < 0.0) t = 0.0;
  if (l < 0.0) return 0.0;
  if (l < dt) return t;

  util::Point<double> start = (*this)(t);
  double t_length = 0.0;
  double u = t + dt;
  while (t_length < l && t < 1.0) {
    t_length += this->length(t, u, dt);
    t = u;
    u += dt;
  }
  return t;
}

/** @brief Finds the tangent vector at a given parameter value.
    @param t Parameter value for where to take the tangent.
    @param dt The interval size used for the numerical evaluation.
*/
util::Point<double> util::Contour::tangent(double t, double dt) {
  util::Point<double> r = (*this)(t + dt) - (*this)(t - dt);
  r.normalise();
  return r;
}

/** @brief Finds the normal vector at a given parameter value.
    @param t Parameter value for where to take the normal.
    @param dt The interval size used for the numerical evaluation.
*/
util::Point<double> util::Contour::normal(double t, double dt) {
  util::Point<double> tvec = tangent(t, dt);
  return util::Point<double>(tvec.y(), tvec.x());
}

/** @brief A basis function. */
double util::Contour::Basis0(double t) const {
  return 1.0/6.0 * (-pow(t, 3) + 3 * pow(t, 2) - 3 * t + 1);
}

/** @brief A basis function. */
double util::Contour::Basis1(double t) const {
  return 1.0/6.0 * (3 * pow(t, 3) - 6 * pow(t, 2) + 4);
}

/** @brief A basis function. */
double util::Contour::Basis2(double t) const {
  return 1.0/6.0 * (-3 * pow(t, 3) + 3 * pow(t, 2) + 3 * t + 1);
}

/** @brief A basis function. */
double util::Contour::Basis3(double t) const {
  return 1.0/6.0 * (pow(t, 3));
}
