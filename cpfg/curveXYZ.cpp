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



// source file curveXYZ.c++

#include <math.h>

#ifdef LINUX
#include <limits.h>
#endif

#include "maxpth.h"

#include "platform.h"

#include "point.h"
#include "curveXYZ.h"
#include "utils.h"
#include <iostream>

CTurtle::CTurtle() {
  _p = false;
  _ps = 1;
  _pt = 1;

  _cp.x = _cp.y = _cp.z = 0.0;
  _ep.x = _ep.y = _ep.z = 0.0;
  _h.x = _h.z = 0.0;
  _h.y = 1.0;
  _u.x = _u.y = 0.0;
  _u.z = -1.0;
  _sz = 1.0;
}

CTurtle::~CTurtle() {}

CColor::CColor() {
  _tc = _bc = 0;
  _td = _bd = 0.0;
}

CColor::~CColor() {}

CCPoint::CCPoint() {
  _r[0] = _r[1] = _r[2] = 0.5;
  _curve = NULL;

  _pmult = 1;

  _visible = 1;
  _saturation = 1.0;
  _select = 0;
}

CCPoint::~CCPoint() { delete _curve; }

CCurve::CCurve() {
  _csize = 0;
  _point = NULL;
  _ctype = eBSPLINE;
}

CCurve::~CCurve() { delete[] _point; }

CCurve *CCurve::cloneCurve() {
  int i, j;
  CCurve *curve = NULL;

  curve = new CCurve;

  curve->_csize = _csize;
  curve->_point = new CCPoint[curve->_csize];
  for (i = 0; i < curve->_csize; i++) {
    curve->_point[i]._point = _point[i]._point;

    for (j = 0; j < 3; j++)
      curve->_point[i]._r[j] = _point[i]._r[j];

    curve->_point[i]._pmult = _point[i]._pmult;
    curve->_point[i]._visible = _point[i]._visible;
    curve->_point[i]._saturation = _point[i]._saturation;
    curve->_point[i]._select = _point[i]._select;

    if (_point[i]._curve != NULL)
      curve->_point[i]._curve = _point[i]._curve->cloneCurve();
    else
      curve->_point[i]._curve = NULL;
  }
  curve->_ctype = _ctype;

  return curve;
}

CAPoint::CAPoint() { _l = 0.0; }

CAPoint::~CAPoint() {}

CCurveXYZ::CCurveXYZ() {
  _filename[0] = 0;

  _curve = init(eBEZIERSURFACE);

  _psize = 0;
  _ppoint = NULL;
  _ptype = eBSPLINE;

  _asize = 100;
  _apoint = NULL;
  _lineNumber = -1;
  createPPoints(_curve);
  createAPoints();
}

CCurveXYZ::CCurveXYZ(const char *filename, int samples) {
  _filename[0] = 0;

  _curve = init(eBEZIERSURFACE);

  _psize = 0;
  _ppoint = NULL;
  _ptype = eBSPLINE;

  _asize = samples;
  _apoint = NULL;

  openCurve(filename, _lineNumber);

  createPPoints(_curve);
  createAPoints();
}

CCurveXYZ::~CCurveXYZ() {
  delete _curve;
  delete[] _ppoint;
  delete[] _apoint;
}

void CCurveXYZ::operator=(const CCurveXYZ &src) {
  int i;

  strcpy(_filename, src._filename);

  _turtle = src._turtle;

  delete _curve;

  _curve = src._curve->cloneCurve();

  delete[] _ppoint;

  _psize = src._psize;
  _ppoint = new CPoint[_psize];
  for (i = 0; i < _psize; i++)
    _ppoint[i] = src._ppoint[i];
  _ptype = src._ptype;

  delete[] _apoint;

  _asize = src._asize;
  _apoint = new CAPoint[_asize];
  for (i = 0; i < _asize; i++)
    _apoint[i] = src._apoint[i];

  _alength = src._alength;
}

CCurve *CCurveXYZ::init(ECURVE type) {
  int i;
  CCurve *curve;

  curve = new CCurve;

  if (type == eBEZIERSURFACE)
    curve->_csize = 16;
  else
    curve->_csize = 4;

  curve->_point = new CCPoint[curve->_csize];
  for (i = 0; i < curve->_csize; i++) {
    curve->_point[i]._point.x = i % 4 * (curve->_csize / 16) - 1.5;
    curve->_point[i]._point.y = i / (curve->_csize / 4) - 1.5;
    curve->_point[i]._point.z = 0.0;
  }
  curve->_ctype = type;

  return curve;
}

double CCurveXYZ::ti(int i) { return (double)i; }

double CCurveXYZ::Bi1(int i, double t) {
  if ((ti(i) <= t) && (t < ti(i + 1)))
    return 1.0;
  else
    return 0.0;
}

double CCurveXYZ::Bi2(int i, double t) {
  double result;

  result = (t - ti(i)) / (ti(i + 1) - ti(i)) * Bi1(i, t) +
           (ti(i + 2) - t) / (ti(i + 2) - ti(i + 1)) * Bi1(i + 1, t);

  return result;
}

double CCurveXYZ::Bi3(int i, double t) {
  double result;

  result = (t - ti(i)) / (ti(i + 2) - ti(i)) * Bi2(i, t) +
           (ti(i + 3) - t) / (ti(i + 3) - ti(i + 1)) * Bi2(i + 1, t);

  return result;
}

double CCurveXYZ::Bi4(int i, double t) {
  double result;

  result = (t - ti(i)) / (ti(i + 3) - ti(i)) * Bi3(i, t) +
           (ti(i + 4) - t) / (ti(i + 4) - ti(i + 1)) * Bi3(i + 1, t);

  return result;
}

double CCurveXYZ::uk(int k) {
  if (k < 4)
    return 0.0;
  else if (k > _psize - 1)
    return (double)_psize - 3.0;
  else
    return (double)k - 3.0;
}

double CCurveXYZ::Nk1(int k, double u) {
  if ((uk(k) <= u) && (u < uk(k + 1)))
    return 1.0;
  else
    return 0.0;
}

double CCurveXYZ::Nk2(int k, double u) {
  double result = 0.0;

  if (uk(k + 1) - uk(k) != 0.0)
    result += (u - uk(k)) / (uk(k + 1) - uk(k)) * Nk1(k, u);

  if (uk(k + 2) - uk(k + 1) != 0.0)
    result += (uk(k + 2) - u) / (uk(k + 2) - uk(k + 1)) * Nk1(k + 1, u);

  return result;
}

double CCurveXYZ::Nk3(int k, double u) {
  double result = 0.0;

  if (uk(k + 2) - uk(k) != 0.0)
    result += (u - uk(k)) / (uk(k + 2) - uk(k)) * Nk2(k, u);

  if (uk(k + 3) - uk(k + 1) != 0.0)
    result += (uk(k + 3) - u) / (uk(k + 3) - uk(k + 1)) * Nk2(k + 1, u);

  return result;
}

double CCurveXYZ::Nk4(int k, double u) {
  double result = 0.0;

  if (uk(k + 3) - uk(k) != 0.0)
    result += (u - uk(k)) / (uk(k + 3) - uk(k)) * Nk3(k, u);

  if (uk(k + 4) - uk(k + 1) != 0.0)
    result += (uk(k + 4) - u) / (uk(k + 4) - uk(k + 1)) * Nk3(k + 1, u);

  return result;
}

CPoint CCurveXYZ::calcPValue(int i, double t, double s, double r) {
  CPoint hP;

  /* r will be used in the future */
  r = 0;

  if (_ptype == eBEZIERSURFACE) {
    if (i * 16 + 15 < _psize) {
      hP.x = (-1 * s * s * s + 3 * s * s - 3 * s + 1) *
                 (t * t * t *
                      (-1 * _ppoint[i * 16 + 0 * 4 + 0].x +
                       3 * _ppoint[i * 16 + 0 * 4 + 1].x -
                       3 * _ppoint[i * 16 + 0 * 4 + 2].x +
                       _ppoint[i * 16 + 0 * 4 + 3].x) +
                  t * t *
                      (3 * _ppoint[i * 16 + 0 * 4 + 0].x -
                       6 * _ppoint[i * 16 + 0 * 4 + 1].x +
                       3 * _ppoint[i * 16 + 0 * 4 + 2].x) +
                  t * (-3 * _ppoint[i * 16 + 0 * 4 + 0].x +
                       3 * _ppoint[i * 16 + 0 * 4 + 1].x) +
                  1 * (_ppoint[i * 16 + 0 * 4 + 0].x)) +
             (3 * s * s * s - 6 * s * s + 3 * s) *
                 (t * t * t *
                      (-1 * _ppoint[i * 16 + 1 * 4 + 0].x +
                       3 * _ppoint[i * 16 + 1 * 4 + 1].x -
                       3 * _ppoint[i * 16 + 1 * 4 + 2].x +
                       _ppoint[i * 16 + 1 * 4 + 3].x) +
                  t * t *
                      (3 * _ppoint[i * 16 + 1 * 4 + 0].x -
                       6 * _ppoint[i * 16 + 1 * 4 + 1].x +
                       3 * _ppoint[i * 16 + 1 * 4 + 2].x) +
                  t * (-3 * _ppoint[i * 16 + 1 * 4 + 0].x +
                       3 * _ppoint[i * 16 + 1 * 4 + 1].x) +
                  1 * (_ppoint[i * 16 + 1 * 4 + 0].x)) +
             (-3 * s * s * s + 3 * s * s) *
                 (t * t * t *
                      (-1 * _ppoint[i * 16 + 2 * 4 + 0].x +
                       3 * _ppoint[i * 16 + 2 * 4 + 1].x -
                       3 * _ppoint[i * 16 + 2 * 4 + 2].x +
                       _ppoint[i * 16 + 2 * 4 + 3].x) +
                  t * t *
                      (3 * _ppoint[i * 16 + 2 * 4 + 0].x -
                       6 * _ppoint[i * 16 + 2 * 4 + 1].x +
                       3 * _ppoint[i * 16 + 2 * 4 + 2].x) +
                  t * (-3 * _ppoint[i * 16 + 2 * 4 + 0].x +
                       3 * _ppoint[i * 16 + 2 * 4 + 1].x) +
                  1 * (_ppoint[i * 16 + 2 * 4 + 0].x)) +
             (s * s * s) * (t * t * t *
                                (-1 * _ppoint[i * 16 + 3 * 4 + 0].x +
                                 3 * _ppoint[i * 16 + 3 * 4 + 1].x -
                                 3 * _ppoint[i * 16 + 3 * 4 + 2].x +
                                 _ppoint[i * 16 + 3 * 4 + 3].x) +
                            t * t *
                                (3 * _ppoint[i * 16 + 3 * 4 + 0].x -
                                 6 * _ppoint[i * 16 + 3 * 4 + 1].x +
                                 3 * _ppoint[i * 16 + 3 * 4 + 2].x) +
                            t * (-3 * _ppoint[i * 16 + 3 * 4 + 0].x +
                                 3 * _ppoint[i * 16 + 3 * 4 + 1].x) +
                            1 * (_ppoint[i * 16 + 3 * 4 + 0].x));

      hP.y = (-1 * s * s * s + 3 * s * s - 3 * s + 1) *
                 (t * t * t *
                      (-1 * _ppoint[i * 16 + 0 * 4 + 0].y +
                       3 * _ppoint[i * 16 + 0 * 4 + 1].y -
                       3 * _ppoint[i * 16 + 0 * 4 + 2].y +
                       _ppoint[i * 16 + 0 * 4 + 3].y) +
                  t * t *
                      (3 * _ppoint[i * 16 + 0 * 4 + 0].y -
                       6 * _ppoint[i * 16 + 0 * 4 + 1].y +
                       3 * _ppoint[i * 16 + 0 * 4 + 2].y) +
                  t * (-3 * _ppoint[i * 16 + 0 * 4 + 0].y +
                       3 * _ppoint[i * 16 + 0 * 4 + 1].y) +
                  1 * (_ppoint[i * 16 + 0 * 4 + 0].y)) +
             (3 * s * s * s - 6 * s * s + 3 * s) *
                 (t * t * t *
                      (-1 * _ppoint[i * 16 + 1 * 4 + 0].y +
                       3 * _ppoint[i * 16 + 1 * 4 + 1].y -
                       3 * _ppoint[i * 16 + 1 * 4 + 2].y +
                       _ppoint[i * 16 + 1 * 4 + 3].y) +
                  t * t *
                      (3 * _ppoint[i * 16 + 1 * 4 + 0].y -
                       6 * _ppoint[i * 16 + 1 * 4 + 1].y +
                       3 * _ppoint[i * 16 + 1 * 4 + 2].y) +
                  t * (-3 * _ppoint[i * 16 + 1 * 4 + 0].y +
                       3 * _ppoint[i * 16 + 1 * 4 + 1].y) +
                  1 * (_ppoint[i * 16 + 1 * 4 + 0].y)) +
             (-3 * s * s * s + 3 * s * s) *
                 (t * t * t *
                      (-1 * _ppoint[i * 16 + 2 * 4 + 0].y +
                       3 * _ppoint[i * 16 + 2 * 4 + 1].y -
                       3 * _ppoint[i * 16 + 2 * 4 + 2].y +
                       _ppoint[i * 16 + 2 * 4 + 3].y) +
                  t * t *
                      (3 * _ppoint[i * 16 + 2 * 4 + 0].y -
                       6 * _ppoint[i * 16 + 2 * 4 + 1].y +
                       3 * _ppoint[i * 16 + 2 * 4 + 2].y) +
                  t * (-3 * _ppoint[i * 16 + 2 * 4 + 0].y +
                       3 * _ppoint[i * 16 + 2 * 4 + 1].y) +
                  1 * (_ppoint[i * 16 + 2 * 4 + 0].y)) +
             (s * s * s) * (t * t * t *
                                (-1 * _ppoint[i * 16 + 3 * 4 + 0].y +
                                 3 * _ppoint[i * 16 + 3 * 4 + 1].y -
                                 3 * _ppoint[i * 16 + 3 * 4 + 2].y +
                                 _ppoint[i * 16 + 3 * 4 + 3].y) +
                            t * t *
                                (3 * _ppoint[i * 16 + 3 * 4 + 0].y -
                                 6 * _ppoint[i * 16 + 3 * 4 + 1].y +
                                 3 * _ppoint[i * 16 + 3 * 4 + 2].y) +
                            t * (-3 * _ppoint[i * 16 + 3 * 4 + 0].y +
                                 3 * _ppoint[i * 16 + 3 * 4 + 1].y) +
                            1 * (_ppoint[i * 16 + 3 * 4 + 0].y));

      hP.z = (-1 * s * s * s + 3 * s * s - 3 * s + 1) *
                 (t * t * t *
                      (-1 * _ppoint[i * 16 + 0 * 4 + 0].z +
                       3 * _ppoint[i * 16 + 0 * 4 + 1].z -
                       3 * _ppoint[i * 16 + 0 * 4 + 2].z +
                       _ppoint[i * 16 + 0 * 4 + 3].z) +
                  t * t *
                      (3 * _ppoint[i * 16 + 0 * 4 + 0].z -
                       6 * _ppoint[i * 16 + 0 * 4 + 1].z +
                       3 * _ppoint[i * 16 + 0 * 4 + 2].z) +
                  t * (-3 * _ppoint[i * 16 + 0 * 4 + 0].z +
                       3 * _ppoint[i * 16 + 0 * 4 + 1].z) +
                  1 * (_ppoint[i * 16 + 0 * 4 + 0].z)) +
             (3 * s * s * s - 6 * s * s + 3 * s) *
                 (t * t * t *
                      (-1 * _ppoint[i * 16 + 1 * 4 + 0].z +
                       3 * _ppoint[i * 16 + 1 * 4 + 1].z -
                       3 * _ppoint[i * 16 + 1 * 4 + 2].z +
                       _ppoint[i * 16 + 1 * 4 + 3].z) +
                  t * t *
                      (3 * _ppoint[i * 16 + 1 * 4 + 0].z -
                       6 * _ppoint[i * 16 + 1 * 4 + 1].z +
                       3 * _ppoint[i * 16 + 1 * 4 + 2].z) +
                  t * (-3 * _ppoint[i * 16 + 1 * 4 + 0].z +
                       3 * _ppoint[i * 16 + 1 * 4 + 1].z) +
                  1 * (_ppoint[i * 16 + 1 * 4 + 0].z)) +
             (-3 * s * s * s + 3 * s * s) *
                 (t * t * t *
                      (-1 * _ppoint[i * 16 + 2 * 4 + 0].z +
                       3 * _ppoint[i * 16 + 2 * 4 + 1].z -
                       3 * _ppoint[i * 16 + 2 * 4 + 2].z +
                       _ppoint[i * 16 + 2 * 4 + 3].z) +
                  t * t *
                      (3 * _ppoint[i * 16 + 2 * 4 + 0].z -
                       6 * _ppoint[i * 16 + 2 * 4 + 1].z +
                       3 * _ppoint[i * 16 + 2 * 4 + 2].z) +
                  t * (-3 * _ppoint[i * 16 + 2 * 4 + 0].z +
                       3 * _ppoint[i * 16 + 2 * 4 + 1].z) +
                  1 * (_ppoint[i * 16 + 2 * 4 + 0].z)) +
             (s * s * s) * (t * t * t *
                                (-1 * _ppoint[i * 16 + 3 * 4 + 0].z +
                                 3 * _ppoint[i * 16 + 3 * 4 + 1].z -
                                 3 * _ppoint[i * 16 + 3 * 4 + 2].z +
                                 _ppoint[i * 16 + 3 * 4 + 3].z) +
                            t * t *
                                (3 * _ppoint[i * 16 + 3 * 4 + 0].z -
                                 6 * _ppoint[i * 16 + 3 * 4 + 1].z +
                                 3 * _ppoint[i * 16 + 3 * 4 + 2].z) +
                            t * (-3 * _ppoint[i * 16 + 3 * 4 + 0].z +
                                 3 * _ppoint[i * 16 + 3 * 4 + 1].z) +
                            1 * (_ppoint[i * 16 + 3 * 4 + 0].z));
    }
  } else if (_ptype == eBEZIER) {
    if (3 < _psize) {
      int n, c, p;

      // number of bezier segments
      n = 1 + int((_psize - 4) / 3);
      // current bezier segment
      c = int(t / (1.000001 / double(n)));

      i = c * 3;
      t = (t - (double)c * (1.0 / double(n))) * (double)n / 1.000001;

      p = _psize;
      _psize = 4;

      hP.x = _ppoint[i].x * Nk4(0, t) + _ppoint[i + 1].x * Nk4(1, t) +
             _ppoint[i + 2].x * Nk4(2, t) + _ppoint[i + 3].x * Nk4(3, t);
      hP.y = _ppoint[i].y * Nk4(0, t) + _ppoint[i + 1].y * Nk4(1, t) +
             _ppoint[i + 2].y * Nk4(2, t) + _ppoint[i + 3].y * Nk4(3, t);
      hP.z = _ppoint[i].z * Nk4(0, t) + _ppoint[i + 1].z * Nk4(1, t) +
             _ppoint[i + 2].z * Nk4(2, t) + _ppoint[i + 3].z * Nk4(3, t);

      _psize = p;
    }
  } else if (_ptype == eBSPLINEENDPOINT) {
    if (3 < _psize) {
      t = t * ((double)_psize - 3.000001);

      hP.x = hP.y = hP.z = 0.0;
      for (i = 0; i < _psize; i++) {
        hP.x += _ppoint[i].x * Nk4(i, t);
        hP.y += _ppoint[i].y * Nk4(i, t);
        hP.z += _ppoint[i].z * Nk4(i, t);
      }
    }
  } else {
    if (3 < _psize) {
      i = 3 + int(t * (_psize - 3.000001));
      t = 3.0 + t * ((double)_psize - 3.000001);

      hP.x = _ppoint[i - 3].x * Bi4(i - 3, t) +
             _ppoint[i - 2].x * Bi4(i - 2, t) +
             _ppoint[i - 1].x * Bi4(i - 1, t) + _ppoint[i].x * Bi4(i, t);
      hP.y = _ppoint[i - 3].y * Bi4(i - 3, t) +
             _ppoint[i - 2].y * Bi4(i - 2, t) +
             _ppoint[i - 1].y * Bi4(i - 1, t) + _ppoint[i].y * Bi4(i, t);
      hP.z = _ppoint[i - 3].z * Bi4(i - 3, t) +
             _ppoint[i - 2].z * Bi4(i - 2, t) +
             _ppoint[i - 1].z * Bi4(i - 1, t) + _ppoint[i].z * Bi4(i, t);
    }
  }

  return hP;
}

CPoint CCurveXYZ::calcAValue(int i, double t, double s, double r) {
  if (_ptype == eBEZIERSURFACE) {
    int j, k;
    CPoint point[4];
    CCurveXYZ *curveXYZ = NULL;

    curveXYZ = new CCurveXYZ;

    curveXYZ->_asize = _asize;
    curveXYZ->_curve->_ctype = eBEZIER;
    for (j = 0; j < 4; j++) {
      for (k = 0; k < 4; k++)
        curveXYZ->_curve->_point[k]._point = _ppoint[i * 16 + j * 4 + k];

      curveXYZ->createPPoints(curveXYZ->_curve);
      curveXYZ->createAPoints();

      point[j] = curveXYZ->calcAValue(0, t, 0, 0);
    }

    for (j = 0; j < 4; j++)
      curveXYZ->_curve->_point[j]._point = point[j];

    curveXYZ->createPPoints(curveXYZ->_curve);
    curveXYZ->createAPoints();

    point[0] = curveXYZ->calcAValue(0, s, 0, 0);

    delete curveXYZ;

    return point[0];
  } else {
    int min, med, max;

    min = 0;
    max = _asize - 1;
    while (max - min > 1) {
      med = int((min + max) / 2);
      if (_apoint[med]._l > t)
        max = med;
      else
        min = med;
    }

    // interpolate
    t = (min + ((t - _apoint[min]._l) / (_apoint[max]._l - _apoint[min]._l))) /
        (_asize - 1);

    return calcPValue(i, t, s, r);
  }
}

CPoint CCurveXYZ::calcXValue(int i, double t, double s, double r) {
  if (_ptype == eBEZIERSURFACE) {
    return calcPValue(i, t, s, r);
  } else {
    int min, med, max;

    min = 0;
    max = _asize - 1;
    if (t < _apoint[min]._point.x) {
      t = 0;
    } else if (t > _apoint[max]._point.x) {
      t = 1;
    } else {
      while (max - min > 1) {
        med = int((min + max) / 2);
        if (_apoint[med]._point.x > t)
          max = med;
        else
          min = med;
      }

      // interpolate
      t = (min + ((t - _apoint[min]._point.x) /
                  (_apoint[max]._point.x - _apoint[min]._point.x))) /
          (_asize - 1);
    }

    return calcPValue(i, t, s, r);
  }
}

void CCurveXYZ::createPPoints(CCurve *curve) {
  int i, j, p, offset;

  offset = 0;

  _psize = 0;
  for (i = 0; i < curve->_csize; i++)
    _psize += curve->_point[i]._pmult;

  _ptype = curve->_ctype;

  if (_ptype == eBSPLINECLOSED) {
    _psize += 3;
  }

  if (_ptype == eBSPLINEPHANTOM) {
    _psize += 2;
    offset += 1;
  }

  delete[] _ppoint;

  _ppoint = new CPoint[_psize];
  p = 0;
  for (i = 0; i < curve->_csize; i++) {
    for (j = 0; j < curve->_point[i]._pmult; j++) {
      _ppoint[p + offset] = curve->_point[i]._point;
      p += 1;
    }
  }

  if (_ptype == eBSPLINECLOSED) {
    for (i = 0; i < 3; i++)
      _ppoint[_psize - 3 + i - offset] = _ppoint[i];
  }

  if (_ptype == eBSPLINEPHANTOM) {
    _ppoint[0].x = 2 * _ppoint[1].x - _ppoint[2].x;
    _ppoint[0].y = 2 * _ppoint[1].y - _ppoint[2].y;
    _ppoint[0].z = 2 * _ppoint[1].z - _ppoint[2].z;
    _ppoint[_psize - 1].x = 2 * _ppoint[_psize - 2].x - _ppoint[_psize - 3].x;
    _ppoint[_psize - 1].y = 2 * _ppoint[_psize - 2].y - _ppoint[_psize - 3].y;
    _ppoint[_psize - 1].z = 2 * _ppoint[_psize - 2].z - _ppoint[_psize - 3].z;
  }
}

void CCurveXYZ::createAPoints() {
  int i;
  double t, dx, dy, dz;

  delete[] _apoint;

  _apoint = new CAPoint[_asize];

  _alength = 0.0;
  for (i = 0; i < _asize; i++) {
    t = i / double(_asize - 1);

    _apoint[i]._point = calcPValue(0, t, 0.0, 0.0);

    if (i > 0) {
      dx = _apoint[i]._point.x - _apoint[i - 1]._point.x;
      dy = _apoint[i]._point.y - _apoint[i - 1]._point.y;
      dz = _apoint[i]._point.z - _apoint[i - 1]._point.z;

      _alength += sqrt(dx * dx + dy * dy + dz * dz);
    }

    _apoint[i]._l = _alength;
  }

  // normalize (0,1)
  for (i = 0; i < _asize; i++) {
    _apoint[i]._l = _apoint[i]._l / _alength;
  }
}

void CCurveXYZ::calcVariety() {
  int i;
  CPoint hP;

  for (i = 0; i < _curve->_csize; i++) {
    if ((_curve->_point[i]._curve != NULL)) {
      createPPoints(_curve->_point[i]._curve);

      createAPoints();

      hP = calcAValue(0, _curve->_point[i]._r[0], _curve->_point[i]._r[1],
                      _curve->_point[i]._r[2]);

      _curve->_point[i]._point = hP;
    }
  }
}

void CCurveXYZ::addPoint(CCurve *curve, int num, double x, double y, double z) {
  int i, offset;
  CCPoint *tmp;

  curve->_csize += 1;
  tmp = new CCPoint[curve->_csize];

  offset = 0;
  for (i = 0; i < curve->_csize; i++) {
    if (i == num) {
      offset -= 1;

      tmp[i]._point.x = x;
      tmp[i]._point.y = y;
      tmp[i]._point.z = z;
    } else {
      tmp[i] = curve->_point[i + offset];

      curve->_point[i + offset]._curve = NULL;
    }
  }

  delete[] curve->_point;

  curve->_point = tmp;
}

void CCurveXYZ::deletePoint(CCurve *curve, int num) {
  int i, offset;
  CCPoint *tmp;

  curve->_csize -= 1;
  tmp = new CCPoint[curve->_csize];

  offset = 0;
  for (i = 0; i < curve->_csize; i++) {
    if (i == num)
      offset += 1;

    tmp[i] = curve->_point[i + offset];

    curve->_point[i + offset]._curve = NULL;
  }

  delete[] curve->_point;

  curve->_point = tmp;
}

void CCurveXYZ::divideCurve(CCurve *curve) {
  int i, j;
  CCPoint *tmp;

  if ((curve->_ctype == eBSPLINE) || (curve->_ctype == eBSPLINEENDPOINT) ||
      (curve->_ctype == eBSPLINEPHANTOM)) {
    tmp = new CCPoint[2 * curve->_csize - 3];

    j = 0;
    for (i = 0; i < curve->_csize - 1; i++) {
      tmp[j]._select = curve->_point[i]._select;

      if (i == 0) {
        tmp[j]._point.x = 0.5 * curve->_point[i]._point.x +
                          0.5 * curve->_point[i + 1]._point.x;
        tmp[j]._point.y = 0.5 * curve->_point[i]._point.y +
                          0.5 * curve->_point[i + 1]._point.y;
        tmp[j]._point.z = 0.5 * curve->_point[i]._point.z +
                          0.5 * curve->_point[i + 1]._point.z;
        j += 1;
      } else {
        tmp[j]._point.x = 0.125 * curve->_point[i - 1]._point.x +
                          0.75 * curve->_point[i]._point.x +
                          0.125 * curve->_point[i + 1]._point.x;
        tmp[j]._point.y = 0.125 * curve->_point[i - 1]._point.y +
                          0.75 * curve->_point[i]._point.y +
                          0.125 * curve->_point[i + 1]._point.y;
        tmp[j]._point.z = 0.125 * curve->_point[i - 1]._point.z +
                          0.75 * curve->_point[i]._point.z +
                          0.125 * curve->_point[i + 1]._point.z;
        j += 1;
        tmp[j]._point.x = 0.5 * curve->_point[i]._point.x +
                          0.5 * curve->_point[i + 1]._point.x;
        tmp[j]._point.y = 0.5 * curve->_point[i]._point.y +
                          0.5 * curve->_point[i + 1]._point.y;
        tmp[j]._point.z = 0.5 * curve->_point[i]._point.z +
                          0.5 * curve->_point[i + 1]._point.z;
        j += 1;
      }
    }

    if (curve->_ctype == eBSPLINEENDPOINT) {
      tmp[0]._point = curve->_point[0]._point;
      tmp[1]._point.x =
          0.5 * curve->_point[0]._point.x + 0.5 * curve->_point[1]._point.x;
      tmp[1]._point.y =
          0.5 * curve->_point[0]._point.y + 0.5 * curve->_point[1]._point.y;
      tmp[1]._point.z =
          0.5 * curve->_point[0]._point.z + 0.5 * curve->_point[1]._point.z;
      tmp[2 * curve->_csize - 5]._point.x =
          0.5 * curve->_point[curve->_csize - 2]._point.x +
          0.5 * curve->_point[curve->_csize - 1]._point.x;
      tmp[2 * curve->_csize - 5]._point.y =
          0.5 * curve->_point[curve->_csize - 2]._point.y +
          0.5 * curve->_point[curve->_csize - 1]._point.y;
      tmp[2 * curve->_csize - 5]._point.z =
          0.5 * curve->_point[curve->_csize - 2]._point.z +
          0.5 * curve->_point[curve->_csize - 1]._point.z;
      tmp[2 * curve->_csize - 4]._point =
          curve->_point[curve->_csize - 1]._point;
    }

    delete[] curve->_point;

    curve->_csize = 2 * curve->_csize - 3;
    curve->_point = tmp;
  }

  if (curve->_ctype == eBEZIER) {
    tmp = new CCPoint[(1 + (curve->_csize - 4) / 3) * 6 + 1];

    j = 0;
    for (i = 0; i < 1 + (curve->_csize - 4) / 3; i++) {
      if (i == 0) {
        tmp[j]._select = curve->_point[i]._select;
        tmp[j]._point = curve->_point[i]._point;
        j += 1;
      }

      tmp[j]._point.x = 0.5 * curve->_point[i * 3 + 0]._point.x +
                        0.5 * curve->_point[i * 3 + 1]._point.x;
      tmp[j]._point.y = 0.5 * curve->_point[i * 3 + 0]._point.y +
                        0.5 * curve->_point[i * 3 + 1]._point.y;
      tmp[j]._point.z = 0.5 * curve->_point[i * 3 + 0]._point.z +
                        0.5 * curve->_point[i * 3 + 1]._point.z;
      j += 1;

      tmp[j]._select = curve->_point[i * 3 + 1]._select;
      tmp[j]._point.x = 0.25 * curve->_point[i * 3 + 0]._point.x +
                        0.5 * curve->_point[i * 3 + 1]._point.x +
                        0.25 * curve->_point[i * 3 + 2]._point.x;
      tmp[j]._point.y = 0.25 * curve->_point[i * 3 + 0]._point.y +
                        0.5 * curve->_point[i * 3 + 1]._point.y +
                        0.25 * curve->_point[i * 3 + 2]._point.y;
      tmp[j]._point.z = 0.25 * curve->_point[i * 3 + 0]._point.z +
                        0.5 * curve->_point[i * 3 + 1]._point.z +
                        0.25 * curve->_point[i * 3 + 2]._point.z;
      j += 1;

      tmp[j]._point.x = 0.125 * curve->_point[i * 3 + 0]._point.x +
                        0.375 * curve->_point[i * 3 + 1]._point.x +
                        0.375 * curve->_point[i * 3 + 2]._point.x +
                        0.125 * curve->_point[i * 3 + 3]._point.x;
      tmp[j]._point.y = 0.125 * curve->_point[i * 3 + 0]._point.y +
                        0.375 * curve->_point[i * 3 + 1]._point.y +
                        0.375 * curve->_point[i * 3 + 2]._point.y +
                        0.125 * curve->_point[i * 3 + 3]._point.y;
      tmp[j]._point.z = 0.125 * curve->_point[i * 3 + 0]._point.z +
                        0.375 * curve->_point[i * 3 + 1]._point.z +
                        0.375 * curve->_point[i * 3 + 2]._point.z +
                        0.125 * curve->_point[i * 3 + 3]._point.z;
      j += 1;

      tmp[j]._select = curve->_point[i * 3 + 2]._select;
      tmp[j]._point.x = 0.25 * curve->_point[i * 3 + 1]._point.x +
                        0.5 * curve->_point[i * 3 + 2]._point.x +
                        0.25 * curve->_point[i * 3 + 3]._point.x;
      tmp[j]._point.y = 0.25 * curve->_point[i * 3 + 1]._point.y +
                        0.5 * curve->_point[i * 3 + 2]._point.y +
                        0.25 * curve->_point[i * 3 + 3]._point.y;
      tmp[j]._point.z = 0.25 * curve->_point[i * 3 + 1]._point.z +
                        0.5 * curve->_point[i * 3 + 2]._point.z +
                        0.25 * curve->_point[i * 3 + 3]._point.z;
      j += 1;

      tmp[j]._point.x = 0.5 * curve->_point[i * 3 + 2]._point.x +
                        0.5 * curve->_point[i * 3 + 3]._point.x;
      tmp[j]._point.y = 0.5 * curve->_point[i * 3 + 2]._point.y +
                        0.5 * curve->_point[i * 3 + 3]._point.y;
      tmp[j]._point.z = 0.5 * curve->_point[i * 3 + 2]._point.z +
                        0.5 * curve->_point[i * 3 + 3]._point.z;
      j += 1;

      tmp[j]._select = curve->_point[i * 3 + 3]._select;
      tmp[j]._point = curve->_point[i * 3 + 3]._point;
      j += 1;
    }

    delete[] curve->_point;

    curve->_csize = (1 + (curve->_csize - 4) / 3) * 6 + 1;
    curve->_point = tmp;
  }

  if (curve->_ctype == eBEZIERSURFACE) {
    int r, c;
    double U[4][4] = {{1, 0, 0, 0},
                      {0.5, 0.5, 0, 0},
                      {0.25, 0.5, 0.25, 0},
                      {0.125, 0.375, 0.375, 0.125}};
    double V[4][4] = {{1, 0.5, 0.25, 0.125},
                      {0, 0.5, 0.5, 0.375},
                      {0, 0, 0.25, 0.375},
                      {0, 0, 0, 0.125}};
    double W[4][4] = {{0.125, 0.375, 0.375, 0.125},
                      {0, 0.25, 0.5, 0.25},
                      {0, 0, 0.5, 0.5},
                      {0, 0, 0, 1}};
    double X[4][4] = {{0.125, 0, 0, 0},
                      {0.375, 0.25, 0, 0},
                      {0.375, 0.5, 0.5, 0},
                      {0.125, 0.25, 0.5, 1}};
    double Yx[4][4], Yy[4][4], Yz[4][4];
    double Zx[4][4], Zy[4][4], Zz[4][4];

    tmp = new CCPoint[curve->_csize / 16 * 64];

    j = 0;
    for (i = 0; i < curve->_csize / 16; i++) {
      // calculate Y
      for (r = 0; r < 4; r++) {
        for (c = 0; c < 4; c++) {
          Yx[r][c] = 0;
          Yy[r][c] = 0;
          Yz[r][c] = 0;
          for (j = 0; j < 4; j++) {
            Yx[r][c] =
                Yx[r][c] + U[r][j] * curve->_point[i * 16 + j * 4 + c]._point.x;
            Yy[r][c] =
                Yy[r][c] + U[r][j] * curve->_point[i * 16 + j * 4 + c]._point.y;
            Yz[r][c] =
                Yz[r][c] + U[r][j] * curve->_point[i * 16 + j * 4 + c]._point.z;
          }
        }
      }

      // calculate Z
      for (r = 0; r < 4; r++) {
        for (c = 0; c < 4; c++) {
          Zx[r][c] = 0;
          Zy[r][c] = 0;
          Zz[r][c] = 0;
          for (j = 0; j < 4; j++) {
            Zx[r][c] =
                Zx[r][c] + W[r][j] * curve->_point[i * 16 + j * 4 + c]._point.x;
            Zy[r][c] =
                Zy[r][c] + W[r][j] * curve->_point[i * 16 + j * 4 + c]._point.y;
            Zz[r][c] =
                Zz[r][c] + W[r][j] * curve->_point[i * 16 + j * 4 + c]._point.z;
          }
        }
      }

      // calculate s [0,0.5], t [0,0.5]
      for (r = 0; r < 4; r++) {
        for (c = 0; c < 4; c++) {
          tmp[(i * 4 + 0) * 16 + r * 4 + c]._point.x = 0;
          tmp[(i * 4 + 0) * 16 + r * 4 + c]._point.y = 0;
          tmp[(i * 4 + 0) * 16 + r * 4 + c]._point.z = 0;
          for (j = 0; j < 4; j++) {
            tmp[(i * 4 + 0) * 16 + r * 4 + c]._point.x =
                tmp[(i * 4 + 0) * 16 + r * 4 + c]._point.x + Yx[r][j] * V[j][c];
            tmp[(i * 4 + 0) * 16 + r * 4 + c]._point.y =
                tmp[(i * 4 + 0) * 16 + r * 4 + c]._point.y + Yy[r][j] * V[j][c];
            tmp[(i * 4 + 0) * 16 + r * 4 + c]._point.z =
                tmp[(i * 4 + 0) * 16 + r * 4 + c]._point.z + Yz[r][j] * V[j][c];
          }
        }
      }

      // calculate s [0,0.5], t [0.5,1]
      for (r = 0; r < 4; r++) {
        for (c = 0; c < 4; c++) {
          tmp[(i * 4 + 1) * 16 + r * 4 + c]._point.x = 0;
          tmp[(i * 4 + 1) * 16 + r * 4 + c]._point.y = 0;
          tmp[(i * 4 + 1) * 16 + r * 4 + c]._point.z = 0;
          for (j = 0; j < 4; j++) {
            tmp[(i * 4 + 1) * 16 + r * 4 + c]._point.x =
                tmp[(i * 4 + 1) * 16 + r * 4 + c]._point.x + Yx[r][j] * X[j][c];
            tmp[(i * 4 + 1) * 16 + r * 4 + c]._point.y =
                tmp[(i * 4 + 1) * 16 + r * 4 + c]._point.y + Yy[r][j] * X[j][c];
            tmp[(i * 4 + 1) * 16 + r * 4 + c]._point.z =
                tmp[(i * 4 + 1) * 16 + r * 4 + c]._point.z + Yz[r][j] * X[j][c];
          }
        }
      }

      // calculate s [0.5,1], t [0,0.5]
      for (r = 0; r < 4; r++) {
        for (c = 0; c < 4; c++) {
          tmp[(i * 4 + 2) * 16 + r * 4 + c]._point.x = 0;
          tmp[(i * 4 + 2) * 16 + r * 4 + c]._point.y = 0;
          tmp[(i * 4 + 2) * 16 + r * 4 + c]._point.z = 0;
          for (j = 0; j < 4; j++) {
            tmp[(i * 4 + 2) * 16 + r * 4 + c]._point.x =
                tmp[(i * 4 + 2) * 16 + r * 4 + c]._point.x + Zx[r][j] * V[j][c];
            tmp[(i * 4 + 2) * 16 + r * 4 + c]._point.y =
                tmp[(i * 4 + 2) * 16 + r * 4 + c]._point.y + Zy[r][j] * V[j][c];
            tmp[(i * 4 + 2) * 16 + r * 4 + c]._point.z =
                tmp[(i * 4 + 2) * 16 + r * 4 + c]._point.z + Zz[r][j] * V[j][c];
          }
        }
      }

      // calculate s [0.5,1], t [0.5,1]
      for (r = 0; r < 4; r++) {
        for (c = 0; c < 4; c++) {
          tmp[(i * 4 + 3) * 16 + r * 4 + c]._point.x = 0;
          tmp[(i * 4 + 3) * 16 + r * 4 + c]._point.y = 0;
          tmp[(i * 4 + 3) * 16 + r * 4 + c]._point.z = 0;
          for (j = 0; j < 4; j++) {
            tmp[(i * 4 + 3) * 16 + r * 4 + c]._point.x =
                tmp[(i * 4 + 3) * 16 + r * 4 + c]._point.x + Zx[r][j] * X[j][c];
            tmp[(i * 4 + 3) * 16 + r * 4 + c]._point.y =
                tmp[(i * 4 + 3) * 16 + r * 4 + c]._point.y + Zy[r][j] * X[j][c];
            tmp[(i * 4 + 3) * 16 + r * 4 + c]._point.z =
                tmp[(i * 4 + 3) * 16 + r * 4 + c]._point.z + Zz[r][j] * X[j][c];
          }
        }
      }
    }

    delete[] curve->_point;

    curve->_csize = curve->_csize / 16 * 64;
    curve->_point = tmp;
  }
}

void CCurveXYZ::translatePoints(double x, double y, double z) {
  int i;

  for (i = 0; i < _psize; i++) {
    _ppoint[i].x += x;
    _ppoint[i].y += y;
    _ppoint[i].z += z;
  }
}

void CCurveXYZ::scalePoints(double x, double y, double z) {
  int i;

  for (i = 0; i < _psize; i++) {
    _ppoint[i].x *= x;
    _ppoint[i].y *= y;
    _ppoint[i].z *= z;
  }
}

void CCurveXYZ::rotatePoints(double alpha, double x, double y, double z) {
  int i;
  double s, c, t, l, theta, m[3][3];

  l = sqrt(x * x + y * y + z * z);
  x = x / l;
  y = y / l;
  z = z / l;

  theta = alpha / 180.0 * 3.14;
  s = sin(theta);
  c = cos(theta);
  t = 1 - c;

  m[0][0] = t * x * x + c;
  m[0][1] = t * x * y - s * z;
  m[0][2] = t * x * z + s * y;

  m[1][0] = t * x * y + s * z;
  m[1][1] = t * y * y + c;
  m[1][2] = t * y * z - s * x;

  m[2][0] = t * x * z - s * y;
  m[2][1] = t * y * z + s * x;
  m[2][2] = t * z * z + c;

  for (i = 0; i < _psize; i++) {
    x = _ppoint[i].x;
    y = _ppoint[i].y;
    z = _ppoint[i].z;

    _ppoint[i].x = m[0][0] * x + m[0][1] * y + m[0][2] * z;
    _ppoint[i].y = m[1][0] * x + m[1][1] * y + m[1][2] * z;
    _ppoint[i].z = m[2][0] * x + m[2][1] * y + m[2][2] * z;
  }
}

// source, destination
void CCurveXYZ::rotateFrames(double t1x, double t1y, double t1z, double n1x,
                             double n1y, double n1z, double t2x, double t2y,
                             double t2z, double n2x, double n2y, double n2z) {
  int i;
  double rx, ry, rz;
  double s, c, t, l, theta, m[3][3];

  // rotation vector t1Xt2
  rx = t1y * t2z - t1z * t2y;
  ry = t1z * t2x - t1x * t2z;
  rz = t1x * t2y - t1y * t2x;

  l = sqrt(rx * rx + ry * ry + rz * rz);
  if (l > 0) {
    rx = rx / l;
    ry = ry / l;
    rz = rz / l;

    l = sqrt(t1x * t1x + t1y * t1y + t1z * t1z);
    if (l > 0) {
      t1x = t1x / l;
      t1y = t1y / l;
      t1z = t1z / l;

      if (l > 0) {
        l = sqrt(t2x * t2x + t2y * t2y + t2z * t2z);
        t2x = t2x / l;
        t2y = t2y / l;
        t2z = t2z / l;

        // rotate around r(x, y, z); align tangent vectors
        theta = acos(t1x * t2x + t1y * t2y + t1z * t2z);
        if (theta != 0) {
          s = sin(theta);
          c = cos(theta);
          t = 1 - c;

          m[0][0] = t * rx * rx + c;
          m[0][1] = t * rx * ry - s * rz;
          m[0][2] = t * rx * rz + s * ry;

          m[1][0] = t * rx * ry + s * rz;
          m[1][1] = t * ry * ry + c;
          m[1][2] = t * ry * rz - s * rx;

          m[2][0] = t * rx * rz - s * ry;
          m[2][1] = t * ry * rz + s * rx;
          m[2][2] = t * rz * rz + c;

          for (i = 0; i < _psize; i++) {
            rx = _ppoint[i].x;
            ry = _ppoint[i].y;
            rz = _ppoint[i].z;

            _ppoint[i].x = m[0][0] * rx + m[0][1] * ry + m[0][2] * rz;
            _ppoint[i].y = m[1][0] * rx + m[1][1] * ry + m[1][2] * rz;
            _ppoint[i].z = m[2][0] * rx + m[2][1] * ry + m[2][2] * rz;
          }

          // new normal vector
          rx = n1x;
          ry = n1y;
          rz = n1z;

          n1x = m[0][0] * rx + m[0][1] * ry + m[0][2] * rz;
          n1y = m[1][0] * rx + m[1][1] * ry + m[1][2] * rz;
          n1z = m[2][0] * rx + m[2][1] * ry + m[2][2] * rz;
        }
      }
    }
  }

  // rotation vector n1Xn2
  rx = n1y * n2z - n1z * n2y;
  ry = n1z * n2x - n1x * n2z;
  rz = n1x * n2y - n1y * n2x;

  l = sqrt(rx * rx + ry * ry + rz * rz);
  if (l > 0) {
    rx = rx / l;
    ry = ry / l;
    rz = rz / l;

    l = sqrt(n1x * n1x + n1y * n1y + n1z * n1z);
    if (l > 0) {
      n1x = n1x / l;
      n1y = n1y / l;
      n1z = n1z / l;

      l = sqrt(n2x * n2x + n2y * n2y + n2z * n2z);
      if (l > 0) {
        n2x = n2x / l;
        n2y = n2y / l;
        n2z = n2z / l;

        // rotate around r(x, y, z); align normal vectors
        theta = acos(n1x * n2x + n1y * n2y + n1z * n2z);
        if (theta != 0) {
          s = sin(theta);
          c = cos(theta);
          t = 1 - c;

          m[0][0] = t * rx * rx + c;
          m[0][1] = t * rx * ry - s * rz;
          m[0][2] = t * rx * rz + s * ry;

          m[1][0] = t * rx * ry + s * rz;
          m[1][1] = t * ry * ry + c;
          m[1][2] = t * ry * rz - s * rx;

          m[2][0] = t * rx * rz - s * ry;
          m[2][1] = t * ry * rz + s * rx;
          m[2][2] = t * rz * rz + c;

          for (i = 0; i < _psize; i++) {
            rx = _ppoint[i].x;
            ry = _ppoint[i].y;
            rz = _ppoint[i].z;

            _ppoint[i].x = m[0][0] * rx + m[0][1] * ry + m[0][2] * rz;
            _ppoint[i].y = m[1][0] * rx + m[1][1] * ry + m[1][2] * rz;
            _ppoint[i].z = m[2][0] * rx + m[2][1] * ry + m[2][2] * rz;
          }
        }
      }
    }
  }
}

int CCurveXYZ::readCurve(char *command) {
  static const char *labels[] = {
      "//", /*  0                                  */
      "version:", "contact:",   "end:",
      "heading:", "up:", /*  5                                  */
      "size:",    "points:",    "range:",
      "dim:",     "dimension:", /*  10                                 */
      "type:",    NULL          /* the last one must be NULL           */
  };

  const char **labelPtr;
  int labelIndex;
  char format[200];

  /*  determine parameter type - index, when found, is used in switch */
  labelPtr = labels;
  for (labelIndex = 0; *labelPtr != NULL; labelIndex++) {
    if (strncmp(command, *(labelPtr), strlen(*(labelPtr))) == 0)
      break;

    labelPtr++;
  }

  char txt[4][256];
  float flo[3];
  int num[2];

  /* process accordingly */
  switch (labelIndex) {
  case 0: // //

    break;

  case 1: // version
    strcpy(format, labels[labelIndex]);
    strcat(format, " %s");

    if (1 == sscanf(command, format, &txt[0])) {
    } else {
      Message("Error: %s\n", command);
      return 0;
    }

    break;

  case 2: // contact
    strcpy(format, labels[labelIndex]);
    strcat(format, " %f %f %f");

    if (3 == sscanf(command, format, &flo[0], &flo[1], &flo[2])) {
      _turtle._cp.x = flo[0];
      _turtle._cp.y = flo[1];
      _turtle._cp.z = flo[2];
    } else {
      Message("Error: %s\n", command);
      return 0;
    }

    break;

  case 3: // end
    strcpy(format, labels[labelIndex]);
    strcat(format, " %f %f %f");

    if (3 == sscanf(command, format, &flo[0], &flo[1], &flo[2])) {
      _turtle._ep.x = flo[0];
      _turtle._ep.y = flo[1];
      _turtle._ep.z = flo[2];
    } else {
      Utils::Warning("Error: %s\n", command);
      return 0;
    }

    break;

  case 4: // heading
    strcpy(format, labels[labelIndex]);
    strcat(format, " %f %f %f");

    if (3 == sscanf(command, format, &flo[0], &flo[1], &flo[2])) {
      _turtle._h.x = flo[0];
      _turtle._h.y = flo[1];
      _turtle._h.z = flo[2];
    } else {
      Utils::Warning("Error: %s\n", command);
      return 0;
    }

    break;

  case 5: // up
    strcpy(format, labels[labelIndex]);
    strcat(format, " %f %f %f");

    if (3 == sscanf(command, format, &flo[0], &flo[1], &flo[2])) {
      _turtle._u.x = flo[0];
      _turtle._u.y = flo[1];
      _turtle._u.z = flo[2];
    } else {
      Utils::Warning("Error: %s\n", command);
      return 0;
    }

    break;

  case 6: // size
    strcpy(format, labels[labelIndex]);
    strcat(format, " %f");

    if (1 == sscanf(command, format, &flo[0])) {
      _turtle._sz = flo[0];
    } else {
      Utils::Warning("Error: %s\n", command);
      return 0;
    }

    break;

  case 7: // points
    strcpy(format, labels[labelIndex]);
    strcat(format, " %i");

    if (1 == sscanf(command, format, &num[0])) {
      _clevel += 1;
      if (_clevel == 0)
        _ccurve[_clevel] = _curve;
      else {
        _ccurve[_clevel - 1]->_point[_cpoint[_clevel - 1]]._curve = new CCurve;
        _ccurve[_clevel] =
            _ccurve[_clevel - 1]->_point[_cpoint[_clevel - 1]]._curve;
      }
      _cpoint[_clevel] = -1;
      _cdimen[_clevel] = 2;

      _ccurve[_clevel]->_csize = num[0];
      _ccurve[_clevel]->_point = new CCPoint[_ccurve[_clevel]->_csize];
    } else {
      Utils::Warning("Error: %s\n", command);
      return 0;
    }

    break;

  case 8: // range
    strcpy(format, labels[labelIndex]);
    strcat(format, " %f %f");

    if (2 == sscanf(command, format, &flo[0], &flo[1])) {
    } else {
      Utils::Warning("Error: %s\n", command);
      return 0;
    }

    break;

  case 9:  // dim
  case 10: // dimension
    strcpy(format, labels[labelIndex]);
    strcat(format, " %i");

    if (1 == sscanf(command, format, &num[0])) {
      _cdimen[_clevel] = num[0];
    } else {
      Utils::Warning("Error: %s\n", command);
      return 0;
    }

    break;

  case 11: // type
    strcpy(format, labels[labelIndex]);
    strcat(format, " %s");

    if (1 == sscanf(command, format, &txt[0])) {
      if (strcmp(txt[0], "bezier-surface") == 0)
        _ccurve[_clevel]->_ctype = eBEZIERSURFACE;
      else if (strcmp(txt[0], "bezier") == 0)
        _ccurve[_clevel]->_ctype = eBEZIER;
      else if (strcmp(txt[0], "bspline-phantom") == 0)
        _ccurve[_clevel]->_ctype = eBSPLINEPHANTOM;
      else if (strcmp(txt[0], "bspline-endpoint") == 0)
        _ccurve[_clevel]->_ctype = eBSPLINEENDPOINT;
      else if (strcmp(txt[0], "bspline-closed") == 0)
        _ccurve[_clevel]->_ctype = eBSPLINECLOSED;
      else
        _ccurve[_clevel]->_ctype = eBSPLINE;
    } else {
      Utils::Warning("Error: %s\n", command);
      return 0;
    }

    break;

  default: // NULL
    if ((strlen(command) > 2) && (strchr(command, ':') == NULL)) {
      strcpy(format, "%s %s %s %s");
      if (_cdimen[_clevel] ==
          sscanf(command, format, &txt[0], &txt[1], &txt[2], &txt[3])) {
        _cpoint[_clevel] += 1;
        while (_cpoint[_clevel] >= _ccurve[_clevel]->_csize) {
          _clevel -= 1;
          _cpoint[_clevel] += 1;
        }

        _ccurve[_clevel]->_point[_cpoint[_clevel]]._point.x = atof(txt[0]);
        _ccurve[_clevel]->_point[_cpoint[_clevel]]._point.y = atof(txt[1]);
        if (_cdimen[_clevel] > 2) {
          _ccurve[_clevel]->_point[_cpoint[_clevel]]._point.z = atof(txt[2]);

          if (_cdimen[_clevel] > 3)
            _ccurve[_clevel]->_point[_cpoint[_clevel]]._pmult =
                int(atof(txt[3]));
        }
      } else {
        Utils::Warning("Error: dimension is invalid -> %s\n", command);
        return 0;
      }
    }
  }
  return 1;
}

// [Pascal] keep opening the file until the size is stable
static FILE *testOpenFile(const char *fname) {

#ifndef WIN32
  int counter = 0;
  int current_size = -1;
  int size = 0;
  // get size of file
  FILE *control_fp = fopen(fname, "r");
  // MIK - THis is a big hack. Need to add a counter to avoid infinite loop if
  // view file is missing
  while ((control_fp == NULL) && (counter < 10)) {
    control_fp = fopen(fname, "rb");
    counter++;
  }
  if (counter == 10){
    fprintf(stderr, "WARNING (curveXYZ.c): Can't open the view file %s - using defaults.\n",
            fname);
    return NULL;
  }
    fseek(control_fp, 0, SEEK_END); // seek to end of file
    size = ftell(control_fp);       // get current file pointer

    while ((size == 0) || (current_size != size)) {
      current_size = size;
      fclose(control_fp);

      control_fp = fopen(fname, "r");
      while (control_fp == NULL) {
        control_fp = fopen(fname, "r");
        counter++;
      }
      fseek(control_fp, 0, SEEK_END); // seek to end of file
      size = ftell(control_fp);       // get current file pointer
    }
  
  fseek(control_fp, 0L, SEEK_SET);
#else
  FILE *control_fp = fopen(fname, "rb");
#endif

  return control_fp;
}

int CCurveXYZ::openCurve(const char *filename, int &lineNumber) {
  FILE *fp;

  if ((fp = testOpenFile(filename)) == NULL) {
    Utils::Warning("Error: Can't open the file %s\n", filename);
    return 0;
  }

  char buf[256], c;
  int i;

  lineNumber = 0;
  if (fp != NULL) {
    strcpy(_filename, filename);

    delete _curve;

    _curve = new CCurve;

    _cmax = 20;
    _ccurve = new CCurve *[_cmax];
    for (i = 0; i < _cmax; i++)
      _ccurve[i] = NULL;
    _cpoint = new int[_cmax];
    _cdimen = new int[_cmax];
    _clevel = -1;

    c = char(fgetc(fp));
    while (!feof(fp)) {
      lineNumber++;
      i = 0;
      while (c != '\n') {
        buf[i] = c;
        i++;
        c = char(fgetc(fp));
      }
      buf[i] = 0;

      if (!readCurve(buf))
        return 0;
      c = char(fgetc(fp));
    }

    for (i = 0; i < _cmax; i++)
      _ccurve[i] = NULL;
    delete[] _ccurve;
    delete[] _cpoint;
    delete[] _cdimen;

    fclose(fp);
    return 1;
  } else {
    Utils::Warning("Error: Can't open the file %s\n", filename);
    return 0;
  }
}

int CCurveXYZ::rereadCurve() {
  int lineNumber;

  if (!openCurve(_filename, lineNumber))
    return 0;

  if (lineNumber < 2) { // [Pascal] hack to make sure the curve is fully written
    return 0;
  }

  createPPoints(_curve);
  createAPoints();
  return 1;
}
