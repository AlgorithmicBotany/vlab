/* ******************************************************************** *
   Copyright (C) 1990-2022 University of Calgary
  
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
  
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * ******************************************************************** */



#include "contour.h"
#include "matrix.h"
#include "file.h"
#include "exception.h"
#include "utils.h"
#include <string.h>

static const float epsilon = 0.0001f;
static const float Spline[16] = {
    -1.0f / 6.0f, 3.0f / 6.0f,  -3.0f / 6.0f, 1.0f / 6.0f,
    3.0f / 6.0f,  -6.0f / 6.0f, 3.0f / 6.0f,  0.0f / 6.0f,
    -3.0f / 6.0f, 0.0f / 6.0f,  3.0f / 6.0f,  0.0f / 6.0f,
    1.0f / 6.0f,  4.0f / 6.0f,  1.0f / 6.0f,  0.0f / 6.0f};

static const float SplineNormal[16] = {
    0.0f / 2.0f,  0.0f / 6.0f, 0.0f / 6.0f,  0.0f / 6.0f,
    -3.0f / 6.0f, 9.0f / 6.0f, -9.0f / 6.0f, 3.0f / 6.0f,
    6.0f / 6.0f,  -2.0f,       6.0f / 6.0f,  0.0f / 6.0f,
    -3.0f / 6.0f, 0.0f / 6.0f, 3.0f / 6.0f,  0.0f / 6.0f};

const Matrix<4, 4> _SplineMatrix(Spline);
const Matrix<4, 4> _SplineNormal(SplineNormal);

Contour::Contour() { _Default(); }

/*
        _Default: Creates a default contour, that consists of Divisions() points
   uniformly distributed around a circle.

        Author (code): ???
        Author (comment): Adam Kromm
*/
void Contour::_Default() {
  defaultLoaded = true;

  _Closed = true;
  _type = btRegular;
  _vertex = std::vector<Vector3d>(Divisions());
  _normal = std::vector<Vector3d>(Divisions());
  for (size_t i = 0; i < Divisions() - 1; ++i) {
    _vertex[i].Set(sinf(M_PI / (0.5f * (Divisions() - 1)) * i),
                   cosf(M_PI / (0.5f * (Divisions() - 1)) * i), 0.0f);
    _normal[i] = _vertex[i];
  }
  _vertex[Divisions() - 1] = _vertex[0];
  _normal[Divisions() - 1] = _normal[0];
  strcpy(_Name, "def_contour");
  // MC - Sept. 2016 - default max and min point
  _maxPt = Vector3d(1.f, 1.f, 0.0);
  _minPt = Vector3d(-1.f, -1.f, 0.0);
}

Contour::Contour(ReadTextFile &src) {
  defaultLoaded = false;

  _Closed = true;
  _type = btRegular;
  const int BfSize = 128;
  char ln[BfSize];
  src.ReadLine(ln, BfSize);
  int vmaj, vmin;
  if (2 != sscanf(ln, "cver %d %d", &vmaj, &vmin)) {
    _Default();
    return;
  }
  int version = 100 * vmaj + vmin;
  bool success = true;
  switch (version) {
  case 101:
    success = _Load0101(src);
    break;
  case 102:
    success = _Load0102(src);
    break;
  case 103:
    success = _Load0103(src);
    break;
  default:
    defaultLoaded = true;
    _Default();
  }
  if (!success) {
    _Default();
    return; // if the file is not read properly we don't do anything
  }
  _Calculate(_cpts);

  _origCpts.resize(_cpts.size());

  Iter it = _origCpts.begin();
  for (Citer cit = _cpts.begin(); cit != _cpts.end(); ++it, ++cit)
    *it = *cit;

  // MC - Sept. 2016 - compute bounding box of user-defined curve
  _maxPt = Vector3d(-1e11, -1e11, 0.0);
  _minPt = Vector3d(1e11, 1e11, 0.0);
  for (size_t i = 0; i < Divisions() && i < _vertex.size(); ++i) {
    if (_vertex[i].X() < _minPt.X())
      _minPt.X(_vertex[i].X());
    if (_vertex[i].X() > _maxPt.X())
      _maxPt.X(_vertex[i].X());

    if (_vertex[i].Y() < _minPt.Y())
      _minPt.Y(_vertex[i].Y());
    if (_vertex[i].Y() > _maxPt.Y())
      _maxPt.Y(_vertex[i].Y());
  }
}

const float *Contour::Vertex(size_t i) const {
  ASSERT(i < Divisions());
  return _vertex[i];
}

const float *Contour::Normal(size_t i) const {
  ASSERT(i < Divisions());
  return _normal[i];
}

bool Contour::Closed() const { return _Closed; }

Vector3d Contour::GetVertex(size_t i) const {
  ASSERT(i < Divisions());
  return _vertex[i];
}

Vector3d Contour::GetNormal(size_t i) const {
  ASSERT(i < Divisions());
  return _normal[i];
}

const char *Contour::Name() const { return _Name; }

size_t Contour::Divisions() const { return _ddata.Divisions(); }

bool Contour::ValidPointId(size_t id) const { return (id < _cpts.size()); }

void Contour::SetPoint(size_t id, float x, float y, float z) {
  ASSERT(ValidPointId(id));
  _cpts[id].Set(x, y, z);
}

size_t Contour::OriginalDivisions() const { return _ddata.OrigDivisions(); }

bool Contour::DivisionsSpecified() const { return _ddata.Specified(); }

std::vector<Vector3d> Contour::GetCtrlPts() { return _cpts; };

void Contour::Reset() {
  _cpts.resize(_origCpts.size());
  for (size_t i = 0; i < _origCpts.size(); ++i)
    _cpts[i] = _origCpts[i];
}

/*
        _Load0101: Load a contour with format 1.1.

        Format:
                name: <name>
                points: <number of points listed> <total number of points
   (including duplicates)> type: <open or closed> x y z m		//m
   represents how many duplicates of this point there are.  eg: 2 would mean
   there are two points at this location.
                ...

        Author: ???
        Comments: Adam Kromm
*/

// can't handle exception with Qt signals
bool Contour::_Load0101(ReadTextFile &src) {
  _type = btRegular;
  const int BfSize = 64;
  char ln[BfSize];

  // Get the name of the contour
  src.ReadLine(ln, BfSize);
  if (1 != sscanf(ln, "name: %30s", _Name))
    return false;
  _Name[LPFGParams::ObjectNameLength] = 0;

  // Get how many points there are
  src.ReadLine(ln, BfSize);
  int n1, n2;
  if (2 != sscanf(ln, "points: %d %d", &n1, &n2))
    return false;
  if (n2 < 4)
    return false;

  // Get whether the curve is open or closed.
  src.ReadLine(ln, BfSize);
  if (!(strncmp(ln + 6, "open", 4)))
    _Closed = false;
  else if (!(strncmp(ln + 6, "closed", 6)))
    _Closed = true;
  else
    return false;

  // Set the size of the control points structure.
  if (_Closed)
    n2 += 3;
  _cpts.resize(n2);
  if (_Closed)
    n2 -= 3;

  Iter iter = _cpts.begin();
  if (_Closed) {
    iter += 3;
  }

  // Read in all the points of the contour.
  for (int i = 0; i < n1; ++i) {
    src.ReadLine(ln, BfSize);
    float x, y, z;
    int m;
    if (4 != sscanf(ln, "%f %f %f %d", &x, &y, &z, &m))
      return false;
    Vector3d v(x, y, z);
    for (int j = 0; j < m; ++j) {
      if (iter == _cpts.end())
        return false;
      *iter = v;
      ++iter;
    }
  }

  // if the curve is closed then copy the last 3 points to the beginning of the
  // list.
  if (_Closed) {
    iter = _cpts.begin();
    Citer citer = _cpts.end();
    citer -= 3;
    for (int i = 0; i < 3; ++i) {
      *iter = *citer;
      ++iter;
      ++citer;
    }
  }
  return true;
}

bool Contour::_Load0102(ReadTextFile &src) {
  const int BfSize = 64;
  char ln[BfSize];
  src.ReadLine(ln, BfSize);
  if (1 != sscanf(ln, "name: %30s", _Name))
    return false;
  _Name[LPFGParams::ObjectNameLength] = 0;
  src.ReadLine(ln, BfSize);
  int n1, n2;
  if (2 != sscanf(ln, "points: %d %d", &n1, &n2))
    return false;
  if (n2 < 4)
    return false;
  src.ReadLine(ln, BfSize);
  char type[11];
  if (1 != sscanf(ln, "type: %10s", type))
    return false;

  if ('o' == type[0])
    _Closed = false;
  else if ('c' == type[0])
    _Closed = true;
  else
    return false;
  if ('r' == type[1])
    _type = btRegular;
  else if ('e' == type[1])
    _type = btEndPoint;
  else
    return false;

  if (_Closed) {
    if (_type == btRegular)
      n2 += 3;
    else
      ++n2;
  }

  _cpts.resize(n2);
  if (_Closed) {
    if (_type == btRegular)
      n2 -= 3;
    else
      --n2;
  }

  Iter iter = _cpts.begin();
  if (_Closed && _type == btRegular) {
    iter += 3;
  }

  for (int i = 0; i < n1; ++i) {
    src.ReadLine(ln, BfSize);
    float x, y, z;
    int m;
    if (4 != sscanf(ln, "%f %f %f %d", &x, &y, &z, &m))
      return false;
    Vector3d v(x, y, z);
    for (int j = 0; j < m; ++j) {
      if (iter == _cpts.end()) {
        printf("Multiplicity information inconsistent for contour %s\n", _Name);
        return false;
      }
      *iter = v;
      ++iter;
    }
  }

  if (_Closed) {
    if (_type == btRegular) {
      iter = _cpts.begin();
      Citer citer = _cpts.end();
      citer -= 3;
      for (int i = 0; i < 3; ++i) {
        *iter = *citer;
        ++iter;
        ++citer;
      }
    } else {
      _cpts.back() = _cpts.front();
    }
  }
  return true;
}

bool Contour::_Load0103(ReadTextFile &src) {
  const int BfSize = 64;
  char ln[BfSize];
  src.ReadLine(ln, BfSize);
  if (1 != sscanf(ln, "name: %30s", _Name))
    return false;
  _Name[LPFGParams::ObjectNameLength] = 0;
  src.ReadLine(ln, BfSize);
  int n1, n2;
  if (2 != sscanf(ln, "points: %d %d", &n1, &n2))
    return false;
  if (n2 < 4)
    return false;
  src.ReadLine(ln, BfSize);
  char type[11];
  if (1 != sscanf(ln, "type: %10s", type))
    return false;

  if ('o' == type[0])
    _Closed = false;
  else if ('c' == type[0])
    _Closed = true;
  else
    return false;
  if ('r' == type[1])
    _type = btRegular;
  else if ('e' == type[1])
    _type = btEndPoint;
  else
    return false;

  src.ReadLine(ln, BfSize);
  {
    int samples = 0;
    if (1 != sscanf(ln, "samples: %d", &samples))
      return false;
    _ddata.SetOrigDivisions(samples);
  }
  // ignore background
  bool readImage = false;
  char c = fgetc(src.Fp());
  if (c == 'b') {
    readImage = true;
  }
  ungetc(c, src.Fp());
  
  if (readImage) {
    src.ReadLine(ln, BfSize); 
    src.ReadLine(ln, BfSize); 
  }
  bool readMarkers = false;
  c = fgetc(src.Fp());
  if (c == 'm') {
    readMarkers = true;
  }
  ungetc(c, src.Fp());
  if (readMarkers) {
    int markers;
    src.ReadLine(ln, BfSize); 
    if (1 != sscanf(ln, "markers: %d\n", &markers)) {
      fprintf(stderr, "Error reading function: Bad markers statement\n");
    }
     for (int i = 0; i < markers; i++) {
       src.ReadLine(ln, BfSize); 
     }
  }
 
  if (_Closed) {
    if (_type == btRegular)
      n2 += 3;
    else
      ++n2;
  }

  _cpts.resize(n2);
  if (_Closed) {
    if (_type == btRegular)
      n2 -= 3;
    else
      --n2;
  }

  Iter iter = _cpts.begin();
  if (_Closed && _type == btRegular) {
    iter += 3;
  }

  for (int i = 0; i < n1; ++i) {
    src.ReadLine(ln, 64);
    float x, y, z;
    int m;
    if (4 != sscanf(ln, "%f %f %f %d", &x, &y, &z, &m))
      return false;
    Vector3d v(x, y, z);
    for (int j = 0; j < m; ++j) {
      if (iter == _cpts.end()) {
        printf("Multiplicity information inconsistent for contour %s\n", _Name);
        return false;
      }
      *iter = v;
      ++iter;
    }
  }

  if (_Closed) {
    if (_type == btRegular) {
      iter = _cpts.begin();
      Citer citer = _cpts.end();
      citer -= 3;
      for (int i = 0; i < 3; ++i) {
        *iter = *citer;
        ++iter;
        ++citer;
      }
    } else {
      _cpts.back() = _cpts.front();
    }
  }
  return true;
}

void Contour::Recalculate() { _Calculate(_cpts); }

void Contour::SetDivisions(size_t d) {
  if (d != Divisions()) {
    _ddata.SetDivisions(d);
    if (_cpts.empty())
      _Default();
    else
      Recalculate();
  }
}

void Contour::_Calculate(const std::vector<Vector3d> &cpts) {
  if (_type == btRegular)
    _CalculateRegular(cpts);
  else
    _CalculateEndPoint(cpts);
}

void Contour::Scale(float x, float y, float z) {
  for (Iter it = _cpts.begin(); it != _cpts.end(); ++it) {
    Vector3d &v = *it;
    v.Scale(x, y, z);
  }
  Recalculate();
}

/*
        _Reparametrize2: Similiar to _Reparametrize, except this one takes in a
   vector of points that represent the curve, and does an arclength
   parametrization of the supplied list (rather than on the _vector[] array) and
   then stores the reparametrization in the _vector[] array for use.

        Author(Code & Comment): Adam Kromm
*/
void Contour::_Reparametrize2(std::vector<Vector3d> &pts,
                              std::vector<Vector3d> &nrm) {
  // calculate the total distance of the curve.
  float totalLength = 0.0f;

  for (int i = 0; i < (int)pts.size() - 1; ++i) {
    totalLength += Distance(pts[i], pts[i + 1]);
  }
  if (totalLength < epsilon)
    totalLength = epsilon;

  // table for arclength values (vertex -> Distance along curve).
  std::vector<float> at(pts.size());

  // Create the table, containing the distances for each point.
  at[0] = 0.0f;
  for (int i = 1; i < (int)pts.size() - 1; ++i)
    at[i] = at[i - 1] + Distance(pts[i - 1], pts[i]) / totalLength;
  at[(int)pts.size() - 1] = 1.0f;
  // create the new list of points for the curve
  if (_vertex.size() < Divisions()) {
    _vertex.resize(Divisions());
    _normal.resize(Divisions());
  }

  _vertex[0] = pts[0];
  _normal[0] = nrm[0];
  int divisions = Divisions() - 1;

  for (int i = 1; i < divisions; ++i) {
    float r;
    int ix = _FindIx(&at[0], static_cast<float>(i) / (divisions), r);
    _vertex[i] = (1.0f - r) * pts[ix] + r * pts[ix + 1];
    _normal[i] = (1.0f - r) * nrm[ix] + r * nrm[ix + 1];
  }
  _vertex[divisions] = pts[(int)pts.size() - 1];
  _normal[divisions] = nrm[(int)nrm.size() - 1];
}

void Contour::_Reparametrize() {}

/*
        _FindIx: Used to find at what index in <at> the value of <at> is closest
   to <t>  and then sets the value <r> to be the amount of interpolation needed
   to get to the actual value of <t> requested.

                It is used after in creating an arc-length parametrization of a
   curve to find the specific points a specific intervals along the curve.

        Author (Code): ???
        Author (Comment): Adam Kromm
*/
int Contour::_FindIx(float at[], float t, float &r) const {
  assert(t > 0.0f);
  assert(t < 1.0f);

  // find the first value of a[i] that is greater than t.
  int i = 1;
  while (at[i] < t) {
    ++i;
  }

  // get the previous value (largest value smaller than t).
  --i;
  t -= at[i];

  // calculate the value for r.
  float dist = at[i + 1] - at[i];
  r = t / dist;
  return i;
}

/*
        _CalculateRegular: Calculates a regular contour using the control points
   cpts.  This is a bit more robust than the original _CalculatePoints in that
   it creates the contour then does an arc-length parametrization of the curve,
   to get points that are regularly dispersed around the curve.  This also
   removes the limitation of the previous impementation of curves needing the
   same number of control points.

                It is also more robust since, it doesn't rely on a fixed length
   array (_vertex[]) until everything is all said and done.

        Author (Code & Comments): Adam Kromm
*/
void Contour::_CalculateRegular(const std::vector<Vector3d> &cpts) {
  // This will store the resultant b-spline curve.
  std::vector<Vector3d> curve, normals;

  // steps is cpts.size() - 3, because the _Load* functions add the last 3
  // points of the control curve to the beginning of the control points list (if
  // it is closed) so we don't need to worry about end points in a closed curve.
  // If the curve is open, then this is how it's done too.
  int steps = (int)cpts.size() - 3;

  // create the curve using the control points <cpts>
  for (int i = 0; i < steps; ++i) {
    // create 20 points for each segment of the line.
    _CalcPoints2(cpts[i], cpts[i + 1], cpts[i + 2], cpts[i + 3], curve, normals,
                 1000);
  }

  if (Closed()) {
    curve.push_back(curve[0]);
    normals.push_back(normals[0]);
  }
  _Reparametrize2(curve, normals);
  // [MIK] fixes null normals
  // FillZeroNormals(); - not clear why this was called twice!!!
  // MC - Mar. 2016 - check if any normals have zero length, and fix
  FillZeroNormals();
}

/*
        FillZeroNormals: If any of the normals are zero in the _normal array,
   this will fill them in with the previous normal.  This function works best
   when most of the normals are correct with only a small few being zero.

                Warning!!
                - If all the normals are zero they will stay zero.
                - If there is only one normal in the list that is non-zero, all
   the normals will become the same.

                Author (code): ???
                Author (comments): Adam Kromm
*/
void Contour::FillZeroNormals() {
  // Make sure the first normal is not zero

  if (_normal[0].IsNull()) {
    for (size_t i = 1; i < Divisions(); ++i) {
      if (!_normal[i].IsNull()) {
        _normal[0] = _normal[i];
        break;
      }
    }
  }

  // Now assign any null normal value of the previous one
  for (size_t i = 1; i < Divisions(); ++i) {
    if (_normal[i].IsNull())
      _normal[i] = _normal[i - 1];
  }
}

void Contour::_CalculateEndPoint(const std::vector<Vector3d> &cpts) {
  if (_vertex.size() < Divisions()) {
    _vertex.resize(Divisions());
    _normal.resize(Divisions());
  }
  _vertex[0] = cpts.front();
  for (size_t i = 1; i < Divisions() - 1; ++i)
    _vertex[i] = _P(static_cast<float>(i) / (Divisions() - 1), cpts);
  _vertex[Divisions() - 1] = cpts.back();
}

Vector3d Contour::_P(float d, const std::vector<Vector3d> &arr) {
  assert(d >= 0.0f);
  assert(d <= 1.0f);
  const size_t n = arr.size() - 1;
  const size_t t = 4;
  float u = d * (n - t + 2);
  Vector3d sum;
  for (size_t k = 0; k <= n; k++)
    sum += arr[k] * _N(k, t, u, n);
  return sum;
}

float Contour::_N(size_t k, size_t t, float u, size_t n) {
  if (1 == t)
    return _Nk1(k, u, n);
  else
    return _Nkt(k, t, u, n);
}

float Contour::_Nk1(size_t k, float u, size_t n) {
  if (_Uk(k, n) <= u) {
    if (u < _Uk(k + 1, n))
      return 1.0;
  }
  return 0.0;
}

float Contour::_Nkt(size_t k, size_t t, float u, size_t n) {
  float sum = 0.0f;
  size_t div = _Uk(k + t - 1, n) - _Uk(k, n);
  if (0 != div)
    sum = (u - _Uk(k, n)) / div * _N(k, t - 1, u, n);

  div = _Uk(k + t, n) - _Uk(k + 1, n);
  if (0 != div)
    sum += (_Uk(k + t, n) - u) / div * _N(k + 1, t - 1, u, n);

  return sum;
}

size_t Contour::_Uk(size_t j, size_t n) {
  const size_t t = 4;
  if (j < t)
    return 0;
  if (j > n)
    return n - t + 2;
  return j - t + 1;
}

float Contour::GetX(float t) const {
  V3f res = GetV3fPoint(t);
  return res.x;
}

float Contour::GetY(float t) const {
  V3f res = GetV3fPoint(t);
  return res.y;
}

float Contour::GetZ(float t) const {
  V3f res = GetV3fPoint(t);
  return res.z;
}

V3f Contour::GetV3fPoint(float t) const {
  assert(t >= 0.0f);
  assert(t <= 1.0f);
  double ifx = t * ((double)Divisions() - 1);
  size_t ix = static_cast<size_t>(ifx);
  float w = ifx - ix;
  Vector3d r;
  if (ix < Divisions() - 1) {
    r = (1.0f - w) * _vertex[ix] + w * _vertex[ix + 1];
  } else {
    r = _vertex[ix];
  }
  V3f res(r.X(), r.Y(), r.Z());
  return res;
}

V3f Contour::GetV3fNormal(float t) const {
  assert(t >= 0.0f);
  assert(t <= 1.0f);
  float ifx = t * (Divisions() - 1);
  size_t ix = static_cast<size_t>(ifx);
  float w = ifx - ix;
  Vector3d r;
  if (ix < Divisions() - 1)
    r = (1.0f - w) * _normal[ix] + w * _normal[ix + 1];
  else
    r = _normal[ix];
  V3f res(r.X(), r.Y(), r.Z());
  return res;
}

V2f Contour::GetV2fPoint(float t) const {
  V3f r = GetV3fPoint(t);
  V2f res(r.x, r.y);
  return res;
}

/*
        _CalcPoints2: Calculates <steps> points between v1, and v2, using a
   cubic bspline.  This is similiar to _CalcPoints, except, doesn't rely on a
   fixed sized _vertex[] array.


        Author (Code & Comments): Adam Kromm
*/
void Contour::_CalcPoints2(const Vector3d &v0, const Vector3d &v1,
                           const Vector3d &v2, const Vector3d &v3,
                           std::vector<Vector3d> &res,
                           std::vector<Vector3d> &nrm, int steps) {
  // construct a matrix with the points to use for the b-spline
  Matrix4x3 m(v0, v1, v2, v3);

  // create a matrix for the different "time" values [t^3, t^2, t, 1] (this is
  // multiplied by the bspline matrix "m" which is multiplied by the point
  // matrix.
  Matrix<1, 4> t;
  t.Set(0, 3, 1.0f);
  for (int i = 0; i < steps; ++i) {
    const float prm = i / (float)steps;
    const float prm2 = prm * prm;

    t.Set(0, 2, prm);        // T
    t.Set(0, 1, prm2);       // T^2
    t.Set(0, 0, prm * prm2); // T^3

    Vector3d point =
        _BSpline(t, m); // Calculate the resultant point on the bspline curve.
    Vector3d normal =
        _BSplineNormal(t, m); // Calculate the resultant normal for the point on
                              // the bspline curve.

    res.push_back(point);  // add it to the line segment.
    nrm.push_back(normal); // add the normal to the normal list.
  }
}

size_t Contour::_CalcPoints(Vector3d v0, Vector3d v1, Vector3d v2, Vector3d v3,
                            size_t ix, size_t steps) {
  ASSERT(steps > 0);
  Matrix4x3 m(v0, v1, v2, v3);
  Matrix<1, 4> t;
  t.Set(0, 3, 1.0f);
  if (1 == steps) {
    const float prm = 0.0f;
    t.Set(0, 2, prm);
    const float prm2 = prm * prm;
    t.Set(0, 1, prm2);
    t.Set(0, 0, prm * prm2);
    ASSERT(ix < Divisions());
    _vertex[ix] = _BSpline(t, m);
    _normal[ix] = _BSplineNormal(t, m);
  } else {
    for (size_t i = 0; i < steps; ++i) {
      ASSERT(steps > 1);
      const float prm = static_cast<float>(i) / static_cast<float>(steps);
      t.Set(0, 2, prm);
      const float prm2 = prm * prm;
      t.Set(0, 1, prm2);
      t.Set(0, 0, prm * prm2);
      ASSERT(ix < Divisions());
      _vertex[ix] = _BSpline(t, m);
      _normal[ix] = _BSplineNormal(t, m);
      ++ix;
    }
  }
  return ix;
}

Vector3d Contour::_BSpline(const Matrix<1, 4> &t, const Matrix4x3 &m) {
  Matrix4x3 stp(_SplineMatrix, m);
  Matrix<1, 3> res;
  res.Product(t, stp);
  return Vector3d(res.Get(0, 0), res.Get(0, 1), res.Get(0, 2));
}

Vector3d Contour::_BSplineNormal(const Matrix<1, 4> &t, const Matrix4x3 &m) {
  Matrix4x3 stp(_SplineNormal, m);
  Matrix<1, 3> res;
  res.Product(t, stp);
  Vector3d tangent(res.Get(0, 0), res.Get(0, 1), res.Get(0, 2));
  if (tangent.Length() < epsilon)
    return Vector3d(0.0f, 0.0f, 0.0f);
  else {
    tangent.Normalize();
    return Vector3d(0.0f, 0.0f, 1.0f) % tangent;
  }
}

void Contour::Dump() const {
  Utils::Message("Contour: %s\n", Name());
  for (size_t i = 0; i < Divisions(); ++i)
    _vertex[i].Dump();
}

void BlendedContour::Blend(const Contour &c1, const Contour &c2, float b) {
  ASSERT(c1.Divisions() == c2.Divisions());
  _ddata.SetDivisions(c1.Divisions());
  if (_vertex.size() < Divisions()) {
    _vertex.resize(Divisions());
    _normal.resize(Divisions());
  }

  const float a = 1.0f - b;
  for (size_t i = 0; i < Divisions(); ++i) {
    _vertex[i] = c1.GetVertex(i) * a + c2.GetVertex(i) * b;
    _normal[i] = c1.GetNormal(i) * a + c2.GetNormal(i) * b;
  }
}

//////////////////////////////////////////////////////////////////////////////////////
//
//	Division Data Functions
//
//////////////////////////////////////////////////////////////////////////////////////
BlendedContour::DivisionData::DivisionData() {
  _divisions = LPFGParams::DefaultContourDivisions;
  _origDiv = 0;
  _specified = false;
}

size_t BlendedContour::DivisionData::Divisions() const { return _divisions; }

bool BlendedContour::DivisionData::Specified() const { return _specified; }

size_t BlendedContour::DivisionData::OrigDivisions() const {
  ASSERT(_specified);
  return _origDiv;
}

void BlendedContour::DivisionData::SetOrigDivisions(size_t d) {
  if ((d< LPFGParams::MinContourDivisions)||(d > LPFGParams::MaxContourDivisions)){
    std::cerr<<"Warning samples should be in the following bounds: "<<LPFGParams::MinContourDivisions<<" - "<< LPFGParams::MaxContourDivisions<<std::endl;
  }
  if (d < LPFGParams::MinContourDivisions){
    d = LPFGParams::MinContourDivisions;
  }
  if (d > LPFGParams::MaxContourDivisions){
    d = LPFGParams::MaxContourDivisions;
  }

  _origDiv = d;
  _divisions = d;
  _specified = true;
}

void BlendedContour::DivisionData::SetDivisions(size_t d) {
  if ((d< LPFGParams::MinContourDivisions)||(d > LPFGParams::MaxContourDivisions)){
    std::cerr<<"Warning samples should be in the following bounds: "<<LPFGParams::MinContourDivisions<<" - "<< LPFGParams::MaxContourDivisions<<std::endl;
  }
  if (d < LPFGParams::MinContourDivisions){
    d = LPFGParams::MinContourDivisions;
  }
  if (d > LPFGParams::MaxContourDivisions){
    d = LPFGParams::MaxContourDivisions;
  }

  _divisions = d;
}
