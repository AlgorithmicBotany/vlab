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



// source file splineXYZ.c++

#include <assert.h>
#include <stdio.h>
#include <math.h>

#include "generate.h"
#include "platform.h"

#include "splineXYZc.h"

#include "dynarray.h"
#include "splineXYZ.h"
#include "utils.h"

CSplineXYZArray SplineXYZArray;

void ReadSplineXYZ(const char *filename, int samples) {
  SplineXYZArray.read(filename, samples);
}

void FreeSplineXYZ() { SplineXYZArray.reset(); }

double CalcSplineX(int id, double t) {

  CPoint hP;

  id--;
  if (id < 0) {
    Warning((char *)"Spline function id < 0", ERROR_LVL);
    return 0.0;
  }
  if (id >= SplineXYZArray.Count()) {
    Utils::Warning("id = %d, count = %d\n", id, SplineXYZArray.Count());
    Warning((char *)"Spline function id >= count", ERROR_LVL);
    return 0.0;
  }

  hP = SplineXYZArray[id].Avalue(t);

  return hP.x;
}

double CalcSplineY(int id, double t) {
  CPoint hP;

  id--;
  if (id < 0) {
    Warning((char *)"Spline function id < 0", ERROR_LVL);
    return 0.0;
  }
  if (id >= SplineXYZArray.Count()) {
    Utils::Warning("id = %d, count = %d\n", id, SplineXYZArray.Count());
    Warning((char *)"Spline function id >= count", ERROR_LVL);
    return 0.0;
  }

  hP = SplineXYZArray[id].Avalue(t);

  return hP.y;
}

double CalcSplineZ(int id, double t) {
  CPoint hP;

  id--;
  if (id < 0) {
    Warning((char *)"Spline function id < 0", ERROR_LVL);
    return 0.0;
  }
  if (id >= SplineXYZArray.Count()) {
    Utils::Warning("id = %d, count = %d\n", id, SplineXYZArray.Count());
    Warning((char *)"Spline function id >= count", ERROR_LVL);
    return 0.0;
  }

  hP = SplineXYZArray[id].Avalue(t);

  return hP.z;
}

CSplineXYZ::CSplineXYZ() {
  _psize = 0;
  _point = NULL;
  _asize = 0;
  _array = NULL;

  init();

  calcArray();
}

CSplineXYZ::CSplineXYZ(const char *filename, int samples) {
  _psize = 0;
  _point = NULL;
  _asize = samples;
  _array = NULL;

  init();

  openCurve(filename);

  calcArray();
}

CSplineXYZ::~CSplineXYZ() { delete[] _point; }

void CSplineXYZ::operator=(const CSplineXYZ &src) {
  int i;

  delete[] _point;

  _psize = src._psize;
  _point = new CPoint[_psize];
  for (i = 0; i < _psize; i++)
    _point[i] = src._point[i];

  delete[] _array;

  _asize = src._asize;
  _array = new double[_asize];
  for (i = 0; i < _asize; i++)
    _array[i] = src._array[i];

  _count = src._count;
  _dim = src._dim;
  _type = src._type;
  _closed = src._closed;
}

void CSplineXYZ::init() {
  int i;

  delete[] _point;

  _psize = 4;
  _point = new CPoint[_psize];
  for (i = 0; i < _psize; i++) {
    _point[i].x = 0.0;
    _point[i].y = double(i);
    _point[i].z = 0.0;
  }

  delete[] _array;

  _array = new double[_asize];

  _count = 0;
  _dim = 2;
  _type = eBSPLINE;
  _closed = 0;
}

void CSplineXYZ::calcArray() {
  int i;
  double t, l, dx, dy, dz;
  CPoint hP[2];

  l = 0.0;
  for (i = 0; i < _asize; i++) {
    t = i / double(_asize - 1);

    hP[1] = Svalue(t);

    if (i > 0) {
      dx = hP[1].x - hP[0].x;
      dy = hP[1].y - hP[0].y;
      dz = hP[1].z - hP[0].z;

      l += sqrt(dx * dx + dy * dy + dz * dz);
    }

    _array[i] = l;

    hP[0] = hP[1];
  }

  // normalize from 0 to 1
  for (i = 0; i < _asize; i++) {
    _array[i] = _array[i] / l;
  }
}

double CSplineXYZ::ti(int i) { return (double)i; }

double CSplineXYZ::Bi1(int i, double t) {
  if ((ti(i) <= t) && (t < ti(i + 1)))
    return 1.0;
  else
    return 0.0;
}

double CSplineXYZ::Bi2(int i, double t) {
  double result;

  result = (t - ti(i)) / (ti(i + 1) - ti(i)) * Bi1(i, t) +
           (ti(i + 2) - t) / (ti(i + 2) - ti(i + 1)) * Bi1(i + 1, t);

  return result;
}

double CSplineXYZ::Bi3(int i, double t) {
  double result;

  result = (t - ti(i)) / (ti(i + 2) - ti(i)) * Bi2(i, t) +
           (ti(i + 3) - t) / (ti(i + 3) - ti(i + 1)) * Bi2(i + 1, t);

  return result;
}

double CSplineXYZ::Bi4(int i, double t) {
  double result;

  result = (t - ti(i)) / (ti(i + 3) - ti(i)) * Bi3(i, t) +
           (ti(i + 4) - t) / (ti(i + 4) - ti(i + 1)) * Bi3(i + 1, t);

  return result;
}

double CSplineXYZ::uk(int k) {
  if (k < 4)
    return 0.0;
  else if (k > _psize - 1)
    return (double)_psize - 3.0;
  else
    return (double)k - 3.0;
}

double CSplineXYZ::Nk1(int k, double u) {
  if ((uk(k) <= u) && (u < uk(k + 1)))
    return 1.0;
  else
    return 0.0;
}

double CSplineXYZ::Nk2(int k, double u) {
  double result = 0.0;

  if (uk(k + 1) - uk(k) != 0.0)
    result += (u - uk(k)) / (uk(k + 1) - uk(k)) * Nk1(k, u);

  if (uk(k + 2) - uk(k + 1) != 0.0)
    result += (uk(k + 2) - u) / (uk(k + 2) - uk(k + 1)) * Nk1(k + 1, u);

  return result;
}

double CSplineXYZ::Nk3(int k, double u) {
  double result = 0.0;

  if (uk(k + 2) - uk(k) != 0.0)
    result += (u - uk(k)) / (uk(k + 2) - uk(k)) * Nk2(k, u);

  if (uk(k + 3) - uk(k + 1) != 0.0)
    result += (uk(k + 3) - u) / (uk(k + 3) - uk(k + 1)) * Nk2(k + 1, u);

  return result;
}

double CSplineXYZ::Nk4(int k, double u) {
  double result = 0.0;

  if (uk(k + 3) - uk(k) != 0.0)
    result += (u - uk(k)) / (uk(k + 3) - uk(k)) * Nk3(k, u);

  if (uk(k + 4) - uk(k + 1) != 0.0)
    result += (uk(k + 4) - u) / (uk(k + 4) - uk(k + 1)) * Nk3(k + 1, u);

  return result;
}

CPoint CSplineXYZ::Svalue(double t) {
  int i;
  CPoint hP;

  if (_type == eBSPLINEENDPOINTS) {
    if (3 < _psize) {
      t = t * ((double)_psize - 3.000001);

      hP.x = hP.y = hP.z = 0.0;
      for (i = 0; i < _psize; i++) {
        hP.x += _point[i].x * Nk4(i, t);
        hP.y += _point[i].y * Nk4(i, t);
        hP.z += _point[i].z * Nk4(i, t);
      }
    }

  } else {
    if (3 < _psize) {
      i = 3 + int(t * (_psize - 3.000001));
      t = 3.0 + t * ((double)_psize - 3.000001);

      hP.x = _point[i - 3].x * Bi4(i - 3, t) + _point[i - 2].x * Bi4(i - 2, t) +
             _point[i - 1].x * Bi4(i - 1, t) + _point[i].x * Bi4(i, t);
      hP.y = _point[i - 3].y * Bi4(i - 3, t) + _point[i - 2].y * Bi4(i - 2, t) +
             _point[i - 1].y * Bi4(i - 1, t) + _point[i].y * Bi4(i, t);
      hP.z = _point[i - 3].z * Bi4(i - 3, t) + _point[i - 2].z * Bi4(i - 2, t) +
             _point[i - 1].z * Bi4(i - 1, t) + _point[i].z * Bi4(i, t);
    }
  }

  return hP;
}

CPoint CSplineXYZ::Avalue(double s) {
  int min, med, max;
  double t;

  // find
  min = 0;
  max = _asize;
  while (max - min > 1) {
    med = int((min + max) / 2);
    if (_array[med] > s)
      max = med;
    else
      min = med;
  }

  // interpolate
  t = (min + ((s - _array[min]) / (_array[max] - _array[min]))) / (_asize - 1);

  return Svalue(t);
}

void CSplineXYZ::readCurve(char *command) {
  static char *labels[] = {
      (char *)"//", /* 0 */
      (char *)"range:", (char *)"points:",
      (char *)"dim:",   (char *)"dimension:",
      (char *)"type:",  (char *)"closed:",
      (char *)":",      NULL /* the last one must be NULL */
  };

  char **labelPtr;
  int labelIndex;
  char format[200];

  /*  determine parameter type - index, when found, is used in switch */
  labelPtr = labels;
  for (labelIndex = 0; *labelPtr != NULL; labelIndex++) {
    if (strncmp(command, *(labelPtr), strlen(*(labelPtr))) == 0)
      break;

    labelPtr++;
  }

  char c40[3][40];
  float flo[3];
  int num[2];

  /* process accordingly */
  switch (labelIndex) {
  case 0: // //
    break;

  case 1: // range
    strcpy(format, labels[labelIndex]);
    strcat(format, " %f %f");

    if (2 == sscanf(command, format, &flo[0], &flo[1])) {
    } else
      Utils::Warning("Error: %s\n", command);

    break;

  case 2: // points
    strcpy(format, labels[labelIndex]);
    strcat(format, " %i");

    if (1 == sscanf(command, format, &num[0])) {
      _psize = num[0];
      _point = new CPoint[_psize];
    } else
      Utils::Warning("Error: %s\n", command);

    break;

  case 3: // dim
    strcpy(format, labels[labelIndex]);
    strcat(format, " %i");

    if (1 == sscanf(command, format, &num[0])) {
      _dim = num[0];
    } else
      Utils::Warning("Error: %s\n", command);

    break;

  case 4: // dimension
    strcpy(format, labels[labelIndex]);
    strcat(format, " %i");

    if (1 == sscanf(command, format, &num[0])) {
      _dim = num[0];
    } else
      Utils::Warning("Error: %s\n", command);

    break;

  case 5: // type
    strcpy(format, labels[labelIndex]);
    strcat(format, " %s");

    if (1 == sscanf(command, format, &c40[0])) {
      if (strcmp(c40[0], "bezier") == 0)
        _type = eBEZIER;
      else if (strcmp(c40[0], "bspline-endpoints") == 0)
        _type = eBSPLINEENDPOINTS;
      else if (strcmp(c40[0], "bspline-phantom") == 0)
        _type = eBSPLINEPHANTOM;
      else
        _type = eBSPLINE;

    } else
      Utils::Warning("Error: %s\n", command);

    break;

  case 6: // closed
    strcpy(format, labels[labelIndex]);
    strcat(format, " %s");

    if (1 == sscanf(command, format, &c40[0])) {
      if (strcmp(c40[0], "yes") == 0) {
        _closed = 1;

        delete[] _point;
        _point = new CPoint[_psize + 3];
      }
    } else
      Utils::Warning("Error: %s\n", command);

    break;

  case 7: // :

    break;

  default: // NULL
    if (strlen(command) > 1) {
      strcpy(format, "%s %s %s");
      if (_dim == sscanf(command, format, &c40[0], &c40[1], &c40[2])) {
        if (_count < _psize) {
          _point[_count].x = atof(c40[0]);
          _point[_count].y = atof(c40[1]);
          if (_dim == 2)
            _point[_count].z = 0.0;
          else
            _point[_count].z = atof(c40[2]);

          _count += 1;
        } else
          Utils::Warning("Error: points is invalid -> %s\n", command);
      } else
        Utils::Warning("Error: dimension is invalid -> %s\n", command);
    }
  }
}

void CSplineXYZ::openCurve(const char *filename) {
  const int bufsize = 1024;

  FILE *fp;
  char buffer[bufsize], c;
  int index = 0;

  if ((fp = fopen(filename, "r")) != NULL) {
    delete[] _point;

    _psize = 0;
    _count = 0;
    _dim = 2;
    _type = eBSPLINE;
    _closed = 0;

    c = fgetc(fp);
    while (!feof(fp)) {
      index = 0;
      while (c != '\n') {
        buffer[index] = c;
        index++;
        c = fgetc(fp);
      }

      buffer[index] = 0;

      readCurve(buffer);

      c = fgetc(fp);
    }

    fclose(fp);

    if ((_count < _psize) || (_psize == 0)) {
      Utils::Warning("Error: points is invalid\n");
      init();
    } else {
      if (_type == eBSPLINEPHANTOM) {
        CPoint *_tmp = _point;

        _point = new CPoint[_psize + 2];
        for (int i = 0; i < _psize; i++)
          _point[i + 1] = _tmp[i];

        delete[] _tmp;

        _point[0].x = 2 * _point[0].x - _point[1].x;
        _point[0].y = 2 * _point[0].y - _point[1].y;
        _point[0].z = 2 * _point[0].z - _point[1].z;
        _point[_psize + 1].x = 2 * _point[_psize].x - _point[_psize - 1].x;
        _point[_psize + 1].y = 2 * _point[_psize].y - _point[_psize - 1].y;
        _point[_psize + 1].z = 2 * _point[_psize].z - _point[_psize - 1].z;
        _psize += 2;
      }

      if (_closed == 1) {
        CPoint *_tmp = _point;

        _point = new CPoint[_psize + 3];
        for (int i = 0; i < _psize; i++)
          _point[i] = _tmp[i];

        delete[] _tmp;

        _point[_psize] = _point[0];
        _point[_psize + 1] = _point[1];
        _point[_psize + 2] = _point[2];
        _psize += 3;
      }
    }
  } else
    Utils::Warning("Error: Can't open the file %s\n", filename);
}

void CSplineXYZArray::read(const char *filename, int samples) {
  CSplineXYZ SplineXYZ(filename, samples);

  Add(SplineXYZ);
}

void CSplineXYZArray::reset() { _items = 0; }
