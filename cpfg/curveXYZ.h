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



// header file curveXYZ.h

#ifndef __CURVEXYZ_H__
#define __CURVEXYZ_H__

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

enum ECURVE {
  eBSPLINE = 0,
  eBSPLINECLOSED,
  eBSPLINEENDPOINT,
  eBSPLINEPHANTOM,
  eBEZIER,
  eBEZIERSURFACE
};

class CCPoint;
class CCurve;

class CTurtle {
public:
  bool _p;
  int _ps, _pt;

  CPoint _cp, _ep, _h, _u;
  double _sz;

public:
  CTurtle();
  ~CTurtle();
};

class CColor {
public:
  int _tc, _bc;
  double _td, _bd;

public:
  CColor();
  ~CColor();
};

class CCPoint {
public:
  CPoint _point;

  double _r[3];
  CCurve *_curve;

  int _pmult, _visible, _select;
  double _saturation;

public:
  CCPoint();
  ~CCPoint();
};

class CCurve {
public:
  int _csize;
  CCPoint *_point;
  ECURVE _ctype;

public:
  CCurve();
  ~CCurve();

  CCurve *cloneCurve();
};

class CAPoint {
public:
  double _l;

  CPoint _point;

public:
  CAPoint();
  ~CAPoint();
};

class CCurveXYZ {
public:
  char _filename[MaxPath + 1];
  int _lineNumber;

  CTurtle _turtle;
  CColor _color;

  CCurve *_curve;

  int _psize;
  CPoint *_ppoint;
  ECURVE _ptype;

  int _asize;
  CAPoint *_apoint;
  double _alength;

  int _cmax, _clevel;
  CCurve **_ccurve;
  int *_cpoint, *_cdimen;

public:
  CCurveXYZ();
  CCurveXYZ(const char *filename, int samples);
  ~CCurveXYZ();

  void operator=(const CCurveXYZ &);

  const char *GetName() const { return _filename; }

  CCurve *init(ECURVE type);

  double ti(int i);
  double Bi1(int i, double t);
  double Bi2(int i, double t);
  double Bi3(int i, double t);
  double Bi4(int i, double t);

  double uk(int k);
  double Nk1(int k, double u);
  double Nk2(int k, double u);
  double Nk3(int k, double u);
  double Nk4(int k, double u);

  CPoint calcPValue(int i, double t, double s, double r);
  CPoint calcAValue(int i, double t, double s, double r);
  CPoint calcXValue(int i, double t, double s, double r);

  void calcVariety();

  void createPPoints(CCurve *curve);
  void createAPoints();

  void addPoint(CCurve *curve, int num, double x, double y, double z);
  void deletePoint(CCurve *curve, int num);

  void divideCurve(CCurve *curve);

  void translatePoints(double x, double y, double z);
  void scalePoints(double x, double y, double z);
  void rotatePoints(double alpha, double x, double y, double z);
  void rotateFrames(double t1x, double t1y, double t1z, double n1x, double n1y,
                    double n1z, double t2x, double t2y, double t2z, double n2x,
                    double n2y, double n2z);

  int readCurve(char *command);
  int openCurve(const char *filename, int &lineNumber);
  int rereadCurve();
};

#endif
