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



#ifdef WIN32
#include "warningset.h"
#pragma warning(disable : 4189)
#endif

#include <assert.h>
#include <stdio.h>

#ifdef LINUX
#include <limits.h>
#endif

#include "maxpth.h"

#include "control.h"
#include "platform.h"
#include "utility.h"
#include "comlineparam.h"

#include "point.h"
#include "curveXYZ.h"
#include "curveXYZa.h"
#include "curveXYZc.h"

#define ERROR_LVL 1

CCurveXYZArray CurveXYZArray;

int CurveXYZCount() { return CurveXYZArray.Count(); }

const char *CurveXYZName(int i) {
  assert(i >= 0);
  assert(i < CurveXYZArray.Count());
  return CurveXYZArray.GetName(i);
}

void ReadCurveXYZ(const char *filename, int samples) {
  CurveXYZArray.read(filename, samples);
}

int RereadCurveXYZ() { return CurveXYZArray.reread(); }

void FreeCurveXYZ() { CurveXYZArray.reset(); }

double CurveCXT(int id, int i, double t, double s, double r) {
  CPoint hP;

  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return 0.0;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return 0.0;
  }

  hP = CurveXYZArray[id].calcPValue(i, t, s, r);

  return hP.x;
}

double CurveCYT(int id, int i, double t, double s, double r) {
  CPoint hP;

  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return 0.0;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return 0.0;
  }

  hP = CurveXYZArray[id].calcPValue(i, t, s, r);

  return hP.y;
}

double CurveCZT(int id, int i, double t, double s, double r) {
  CPoint hP;

  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return 0.0;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return 0.0;
  }

  hP = CurveXYZArray[id].calcPValue(i, t, s, r);

  return hP.z;
}

double CurveCXS(int id, int i, double t, double s, double r) {
  CPoint hP;

  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return 0.0;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return 0.0;
  }
  if (t < 0) {
    VERBOSE("Parameter %f not defined, 0 used instead\n", t);
    t = 0.0;
  }
  if (t > 1) {
    VERBOSE("Parameter %f not defined, 1 used instead\n", t);
    t = 1.0;
  }

  hP = CurveXYZArray[id].calcAValue(i, t, s, r);

  return hP.x;
}

double CurveCYS(int id, int i, double t, double s, double r) {
  CPoint hP;

  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return 0.0;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return 0.0;
  }
  if (t < 0) {
    VERBOSE("Parameter %f not defined, 0 used instead\n", t);
    t = 0.0;
  }
  if (t > 1) {
    VERBOSE("Parameter %f not defined, 1 used instead\n", t);
    t = 1.0;
  }

  hP = CurveXYZArray[id].calcAValue(i, t, s, r);

  return hP.y;
}

double CurveCZS(int id, int i, double t, double s, double r) {
  CPoint hP;

  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return 0.0;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return 0.0;
  }
  if (t < 0) {
    VERBOSE("Parameter %f not defined, 0 used instead\n", t);
    t = 0.0;
  }
  if (t > 1) {
    VERBOSE("Parameter %f not defined, 1 used instead\n", t);
    t = 1.0;
  }

  hP = CurveXYZArray[id].calcAValue(i, t, s, r);

  return hP.z;
}

double CurveCXX(int id, int i, double t, double s, double r) {
  CPoint hP;

  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return 0.0;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return 0.0;
  }

  hP = CurveXYZArray[id].calcXValue(i, t, s, r);

  return hP.x;
}

double CurveCYX(int id, int i, double t, double s, double r) {
  CPoint hP;

  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return 0.0;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return 0.0;
  }

  hP = CurveXYZArray[id].calcXValue(i, t, s, r);

  return hP.y;
}

double CurveCZX(int id, int i, double t, double s, double r) {
  CPoint hP;

  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return 0.0;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return 0.0;
  }

  hP = CurveXYZArray[id].calcXValue(i, t, s, r);

  return hP.z;
}

int CurveGCN(int id) {
  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return 0;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return 0;
  }

  return CurveXYZArray[id]._curve->_csize;
}

double CurveGCX(int id, int num) {
  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return 0.0;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return 0.0;
  }
  if (num < 0) {
    Warning((char *)"Curve point < 0", ERROR_LVL);
    return 0.0;
  }
  if (num >= CurveXYZArray[id]._curve->_csize) {
    Message("id = %d, n = %d\n", id, CurveXYZArray[id]._curve->_csize - 1);
    Warning((char *)"Curve point > n", ERROR_LVL);
    return 0.0;
  }

  return CurveXYZArray[id]._curve->_point[num]._point.x;
}

double CurveGCY(int id, int num) {
  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return 0.0;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return 0.0;
  }
  if (num < 0) {
    Warning((char *)"Curve point < 0", ERROR_LVL);
    return 0.0;
  }
  if (num >= CurveXYZArray[id]._curve->_csize) {
    Message("id = %d, n = %d\n", id, CurveXYZArray[id]._curve->_csize - 1);
    Warning((char *)"Curve point > n", ERROR_LVL);
    return 0.0;
  }

  return CurveXYZArray[id]._curve->_point[num]._point.y;
}

double CurveGCZ(int id, int num) {
  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return 0.0;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return 0.0;
  }
  if (num < 0) {
    Warning((char *)"Curve point < 0", ERROR_LVL);
    return 0.0;
  }
  if (num >= CurveXYZArray[id]._curve->_csize) {
    Message("id = %d, n = %d\n", id, CurveXYZArray[id]._curve->_csize - 1);
    Warning((char *)"Curve point > n", ERROR_LVL);
    return 0.0;
  }

  return CurveXYZArray[id]._curve->_point[num]._point.z;
}

void CurveSCX(int id, int num, double x) {
  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return;
  }
  if (num < 0) {
    Warning((char *)"Curve point < 0", ERROR_LVL);
    return;
  }
  if (num >= CurveXYZArray[id]._curve->_csize) {
    Message("id = %d, n = %d\n", id, CurveXYZArray[id]._curve->_csize - 1);
    Warning((char *)"Curve point > n", ERROR_LVL);
    return;
  }

  CurveXYZArray[id]._curve->_point[num]._point.x = x;
}

void CurveSCY(int id, int num, double y) {
  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return;
  }
  if (num < 0) {
    Warning((char *)"Curve point < 0", ERROR_LVL);
    return;
  }
  if (num >= CurveXYZArray[id]._curve->_csize) {
    Message("id = %d, n = %d\n", id, CurveXYZArray[id]._curve->_csize - 1);
    Warning((char *)"Curve point > n", ERROR_LVL);
    return;
  }

  CurveXYZArray[id]._curve->_point[num]._point.y = y;
}

void CurveSCZ(int id, int num, double z) {
  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return;
  }
  if (num < 0) {
    Warning((char *)"Curve point < 0", ERROR_LVL);
    return;
  }
  if (num >= CurveXYZArray[id]._curve->_csize) {
    Message("id = %d, n = %d\n", id, CurveXYZArray[id]._curve->_csize - 1);
    Warning((char *)"Curve point > n", ERROR_LVL);
    return;
  }

  CurveXYZArray[id]._curve->_point[num]._point.z = z;
}

double CurveGCM(int id, int num) {
  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return 0.0;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return 0.0;
  }
  if (num < 0) {
    Warning((char *)"Curve point < 0", ERROR_LVL);
    return 0.0;
  }
  if (num >= CurveXYZArray[id]._curve->_csize) {
    Message("id = %d, n = %d\n", id, CurveXYZArray[id]._curve->_csize - 1);
    Warning((char *)"Curve point > n", ERROR_LVL);
    return 0.0;
  }

  return CurveXYZArray[id]._curve->_point[num]._pmult;
}

void CurveSCM(int id, int num, int m) {
  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return;
  }
  if (num < 0) {
    Warning((char *)"Curve point < 0", ERROR_LVL);
    return;
  }
  if (num >= CurveXYZArray[id]._curve->_csize) {
    Message("id = %d, n = %d\n", id, CurveXYZArray[id]._curve->_csize - 1);
    Warning((char *)"Curve point > n", ERROR_LVL);
    return;
  }

  CurveXYZArray[id]._curve->_point[num]._pmult = m;
}

double CurveGCT(int id) {
  int type;

  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return 0.0;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return 0.0;
  }

  switch (CurveXYZArray[id]._curve->_ctype) {
  case eBSPLINEENDPOINT:
    type = 1;
    break;
  case eBSPLINEPHANTOM:
    type = 2;
    break;
  case eBEZIER:
    type = 3;
    break;
  case eBEZIERSURFACE:
    type = 4;
    break;
  default:
    type = 0;
  }

  return type;
}

void CurveSCT(int id, int type) {
  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return;
  }

  if (type < 0) {
    Warning((char *)"Curve type < 0", ERROR_LVL);
    return;
  }
  if (type > 4) {
    Message("0=bspline, 1=bspline-endpoint, 2=bspline-phantom, 3=bezier\n");
    Message("4=bezier-surface\n");
    Warning((char *)"Curve type > 4", ERROR_LVL);
    return;
  }

  switch (type) {
  case 1:
    CurveXYZArray[id]._curve->_ctype = eBSPLINEENDPOINT;
    break;
  case 2:
    CurveXYZArray[id]._curve->_ctype = eBSPLINEPHANTOM;
    break;
  case 3:
    CurveXYZArray[id]._curve->_ctype = eBEZIER;
    break;
  case 4:
    CurveXYZArray[id]._curve->_ctype = eBEZIERSURFACE;
    break;
  default:
    CurveXYZArray[id]._curve->_ctype = eBSPLINE;
  }
}

void CurveACL(int id, int num, double x, double y, double z) {
  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return;
  }
  if (num < 0) {
    Warning((char *)"Curve point < 0", ERROR_LVL);
    return;
  }
  if (num >= CurveXYZArray[id]._curve->_csize) {
    Message("id = %d, n = %d\n", id, CurveXYZArray[id]._curve->_csize - 1);
    Warning((char *)"Curve point > n", ERROR_LVL);
    return;
  }

  CurveXYZArray[id].addPoint(CurveXYZArray[id]._curve, num, x, y, z);
}

void CurveACR(int id, int num, double x, double y, double z) {
  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return;
  }
  if (num < 0) {
    Warning((char *)"Curve point < 0", ERROR_LVL);
    return;
  }
  if (num >= CurveXYZArray[id]._curve->_csize) {
    Message("id = %d, n = %d\n", id, CurveXYZArray[id]._curve->_csize - 1);
    Warning((char *)"Curve point > n", ERROR_LVL);
    return;
  }

  CurveXYZArray[id].addPoint(CurveXYZArray[id]._curve, num + 1, x, y, z);
}

void CurveDCP(int id, int num) {
  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return;
  }
  if (num < 0) {
    Warning((char *)"Curve point < 0", ERROR_LVL);
    return;
  }
  if (num >= CurveXYZArray[id]._curve->_csize) {
    Message("id = %d, n = %d\n", id, CurveXYZArray[id]._curve->_csize - 1);
    Warning((char *)"Curve point > n", ERROR_LVL);
    return;
  }

  CurveXYZArray[id].deletePoint(CurveXYZArray[id]._curve, num);
}

double CurveGCU(int id, int num) {
  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return 0.0;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return 0.0;
  }
  if (num < 0) {
    Warning((char *)"Curve point < 0", ERROR_LVL);
    return 0.0;
  }
  if (num >= CurveXYZArray[id]._curve->_csize) {
    Message("id = %d, n = %d\n", id, CurveXYZArray[id]._curve->_csize - 1);
    Warning((char *)"Curve point > n", ERROR_LVL);
    return 0.0;
  }

  return CurveXYZArray[id]._curve->_point[num]._r[0];
}

double CurveGCV(int id, int num) {
  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return 0.0;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return 0.0;
  }
  if (num < 0) {
    Warning((char *)"Curve point < 0", ERROR_LVL);
    return 0.0;
  }
  if (num >= CurveXYZArray[id]._curve->_csize) {
    Message("id = %d, n = %d\n", id, CurveXYZArray[id]._curve->_csize - 1);
    Warning((char *)"Curve point > n", ERROR_LVL);
    return 0.0;
  }

  return CurveXYZArray[id]._curve->_point[num]._r[1];
}

double CurveGCW(int id, int num) {
  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return 0.0;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return 0.0;
  }
  if (num < 0) {
    Warning((char *)"Curve point < 0", ERROR_LVL);
    return 0.0;
  }
  if (num >= CurveXYZArray[id]._curve->_csize) {
    Message("id = %d, n = %d\n", id, CurveXYZArray[id]._curve->_csize - 1);
    Warning((char *)"Curve point > n", ERROR_LVL);
    return 0.0;
  }

  return CurveXYZArray[id]._curve->_point[num]._r[2];
}

void CurveSCU(int id, int num, double u) {
  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return;
  }
  if (num < 0) {
    Warning((char *)"Curve point < 0", ERROR_LVL);
    return;
  }
  if (num >= CurveXYZArray[id]._curve->_csize) {
    Message("id = %d, n = %d\n", id, CurveXYZArray[id]._curve->_csize - 1);
    Warning((char *)"Curve point > n", ERROR_LVL);
    return;
  }

  CurveXYZArray[id]._curve->_point[num]._r[0] = u;
}

void CurveSCV(int id, int num, double v) {
  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return;
  }
  if (num < 0) {
    Warning((char *)"Curve point < 0", ERROR_LVL);
    return;
  }
  if (num >= CurveXYZArray[id]._curve->_csize) {
    Message("id = %d, n = %d\n", id, CurveXYZArray[id]._curve->_csize - 1);
    Warning((char *)"Curve point > n", ERROR_LVL);
    return;
  }

  CurveXYZArray[id]._curve->_point[num]._r[1] = v;
}

void CurveSCW(int id, int num, double w) {
  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return;
  }
  if (num < 0) {
    Warning((char *)"Curve point < 0", ERROR_LVL);
    return;
  }
  if (num >= CurveXYZArray[id]._curve->_csize) {
    Message("id = %d, n = %d\n", id, CurveXYZArray[id]._curve->_csize - 1);
    Warning((char *)"Curve point > n", ERROR_LVL);
    return;
  }

  CurveXYZArray[id]._curve->_point[num]._r[2] = w;
}

void CurveCCV(int id) {
  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return;
  }

  CurveXYZArray[id].calcVariety();
}

void CurveCPP(int id) {
  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return;
  }

  CurveXYZArray[id].createPPoints(CurveXYZArray[id]._curve);
}

int CurveGPN(int id) {
  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return 0;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return 0;
  }

  return CurveXYZArray[id]._psize;
}

double CurveGPX(int id, int num) {
  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return 0.0;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return 0.0;
  }
  if (num < 0) {
    Warning((char *)"PPoint < 0", ERROR_LVL);
    return 0.0;
  }
  if (num >= CurveXYZArray[id]._psize) {
    Message("id = %d, n = %d\n", id, CurveXYZArray[id]._psize - 1);
    Warning((char *)"PPoint > n", ERROR_LVL);
    return 0.0;
  }

  return CurveXYZArray[id]._ppoint[num].x;
}

double CurveGPY(int id, int num) {
  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return 0.0;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return 0.0;
  }
  if (num < 0) {
    Warning((char *)"PPoint < 0", ERROR_LVL);
    return 0.0;
  }
  if (num >= CurveXYZArray[id]._psize) {
    Message("id = %d, n = %d\n", id, CurveXYZArray[id]._psize - 1);
    Warning((char *)"PPoint > n", ERROR_LVL);
    return 0.0;
  }

  return CurveXYZArray[id]._ppoint[num].y;
}

double CurveGPZ(int id, int num) {
  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return 0.0;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return 0.0;
  }
  if (num < 0) {
    Warning((char *)"PPoint < 0", ERROR_LVL);
    return 0.0;
  }
  if (num >= CurveXYZArray[id]._psize) {
    Message("id = %d, n = %d\n", id, CurveXYZArray[id]._psize - 1);
    Warning((char *)"PPoint > n", ERROR_LVL);
    return 0.0;
  }

  return CurveXYZArray[id]._ppoint[num].z;
}

void CurveSPX(int id, int num, double x) {
  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return;
  }
  if (num < 0) {
    Warning((char *)"Point < 0", ERROR_LVL);
    return;
  }
  if (num >= CurveXYZArray[id]._psize) {
    Message("id = %d, n = %d\n", id, CurveXYZArray[id]._psize - 1);
    Warning((char *)"PPoint > n", ERROR_LVL);
    return;
  }

  CurveXYZArray[id]._ppoint[num].x = x;
}

void CurveSPY(int id, int num, double y) {
  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return;
  }
  if (num < 0) {
    Warning((char *)"Point < 0", ERROR_LVL);
    return;
  }
  if (num >= CurveXYZArray[id]._psize) {
    Message("id = %d, n = %d\n", id, CurveXYZArray[id]._psize - 1);
    Warning((char *)"PPoint > n", ERROR_LVL);
    return;
  }

  CurveXYZArray[id]._ppoint[num].y = y;
}

void CurveSPZ(int id, int num, double z) {
  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return;
  }
  if (num < 0) {
    Warning((char *)"Point < 0", ERROR_LVL);
    return;
  }
  if (num >= CurveXYZArray[id]._psize) {
    Message("id = %d, n = %d\n", id, CurveXYZArray[id]._psize - 1);
    Warning((char *)"PPoint > n", ERROR_LVL);
    return;
  }

  CurveXYZArray[id]._ppoint[num].z = z;
}

void CurveTPP(int id, double x, double y, double z) {
  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return;
  }

  CurveXYZArray[id].translatePoints(x, y, z);
}

void CurveSPP(int id, double x, double y, double z) {
  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return;
  }

  CurveXYZArray[id].scalePoints(x, y, z);
}

void CurveRPP(int id, double alpha, double x, double y, double z) {
  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return;
  }

  CurveXYZArray[id].rotatePoints(alpha, x, y, z);
}

void CurveRPF(int id, double t1x, double t1y, double t1z, double n1x,
              double n1y, double n1z, double t2x, double t2y, double t2z,
              double n2x, double n2y, double n2z) {
  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return;
  }

  CurveXYZArray[id].rotateFrames(t1x, t1y, t1z, n1x, n1y, n1z, t2x, t2y, t2z,
                                 n2x, n2y, n2z);
}

void CurveCAP(int id, int samples) {
  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return;
  }

  if (samples > 0)
    CurveXYZArray[id]._asize = samples;

  CurveXYZArray[id].createAPoints();
}

double CurveGAL(int id) {
  id--;
  if (id < 0) {
    Warning((char *)"Curve id < 0", ERROR_LVL);
    return 0.0;
  }
  if (id >= CurveXYZArray.Count()) {
    Message("id = %d, count = %d\n", id, CurveXYZArray.Count());
    Warning((char *)"Curve id >= count", ERROR_LVL);
    return 0.0;
  }

  return CurveXYZArray[id]._alength;
}
