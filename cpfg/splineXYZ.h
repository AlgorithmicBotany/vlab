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



// header file splineXYZ.h

#ifndef __SPLINEXYZ_H__
#define __SPLINEXYZ_H__

#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "point.h"

enum ESpline { eBSPLINE = 0, eBSPLINEENDPOINTS, eBSPLINEPHANTOM, eBEZIER };

class CSplineXYZ {
public:
  int _psize;
  CPoint *_point;
  int _asize;
  double *_array;

  int _count, _dim;
  ESpline _type;
  int _closed;

public:
  CSplineXYZ();
  CSplineXYZ(const char *filename, int samples);
  ~CSplineXYZ();

  void operator=(const CSplineXYZ &);

  void init();

  void calcArray();

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

  CPoint Svalue(double t);
  CPoint Avalue(double s);

  void readCurve(char *command);
  void openCurve(const char *filename);
};

class CSplineXYZArray : public DynArray<CSplineXYZ> {
public:
  CSplineXYZArray() : DynArray<CSplineXYZ>(16) {}

  void read(const char *filename, int samples);
  void reset();
};

#endif
