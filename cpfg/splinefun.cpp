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
#endif

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef LINUX
#include <limits.h>
#endif

#include "maxpth.h"

#include "splinefunC.h"

#include "splinefun.h"
#include "dynarray.h"
#include "splinearr.h"

#include "generate.h"
#include "file.h"
#include "geometry.h"
#include "platform.h"
#include "utility.h"
#include "comlineparam.h"
#include "utils.h"

#ifdef LINUX
#include <qvalidator.h>
#include <iostream>
#endif // LINUX

SplineFunctionArray SplineFunctions;

static bool _FunctionsFromGallery = false;

/******************************************************

  C interface functions

******************************************************/

int ReadSplineFunction(const char *fname, int samples) {
  return SplineFunctions.Read(fname, samples);
}

int RereadFunctions() { return SplineFunctions.Reread(); }

double SplineFuncRange(int id, double limit) {
  id--;
  if (id < 0) {
    Warning((char *)"Spline function identifier<1", ERROR_LVL);
    return 0;
  }
  if (id >= SplineFunctions.Count()) {
    Warning((char *)"Spline function identifier out of range", ERROR_LVL);
    return 0;
  }
  if ((fabs(limit) > 0.01) && (fabs(limit - 1.0) > 0.01)) {
    Warning((char *)"Limit query ambigious", ERROR_LVL);
    return 0;
  }

  if (fabs(limit) < 0.01) {
    return SplineFunctions[id].LowerLimit();
  } else {
    return SplineFunctions[id].UpperLimit();
  }
}

void FreeSplineFunctions() { SplineFunctions.Reset(); }

double SplineFuncValue(int id, double val) {
  id--;
  if (id < 0) {
    Warning((char *)"Spline function id < 0", ERROR_LVL);
    return 0.0;
  }
  if (id >= SplineFunctions.Count()) {
    Utils::Warning("id = %d, count = %d\n", id, SplineFunctions.Count());
    Warning((char *)"Spline function id >= count", ERROR_LVL);
    return 0.0;
  }
  return SplineFunctions[id].GetValue(val);
}

int ReadFunctionSet(const char *fname) {
  SplineFunctions.Reset();
  return SplineFunctions.ReadGallery(fname);
}

int SplineFuncCount() { return SplineFunctions.Count(); }

const char *SplineFuncName(int i) {
  assert(i >= 0);
  assert(i < SplineFunctions.Count());
  return SplineFunctions.GetName(i);
}

int FunctionsFromGallery(void) { return _FunctionsFromGallery; }

void SetFunctionsFromGallery(int i) {
  if (i)
    _FunctionsFromGallery = true;
  else
    _FunctionsFromGallery = false;
}

const char *GetGalleryName(void) { return SplineFunctions.GetGalleryName(); }

/******************************************************

  Member functions implementation

******************************************************/

SplineFunction::SplineFunction() {
  _LowLimit = 0.0;
  _UpLimit = 1.0;
  _arr = NULL;
  _vals = NULL;
  _size = 0;
  _samples = 0;
  _Fname[0] = 0;
}

SplineFunction::SplineFunction(const char *fname, int samples) {
  _arr = NULL;
  _vals = NULL;
  strcpy(_Fname, fname);
  _samples = 0;

  ReadTextFile src(_Fname);
  if (!src) {
    Utils::Warning("Cannot open %s for reading\n", _Fname);
    _Default();
    return;
  }
  char line[256];
  fgets(line, 256, src);
  float l, u;
  if (2 == sscanf(line, "range: %f %f", &l, &u))
    _Load0000(src, samples);
  else {
    int vmaj, vmin;
    if (2 != sscanf(line, "fver %d %d", &vmaj, &vmin)) {
      Message("Error reading %s\n", _Fname);
      _Default();
      return;
    }
    int ver = 100 * vmaj + vmin;
    switch (ver) {
    case 101:
      if (!_Load0101(src))
        _Default();
      break;
    default:
      Message("Unsupported format %d.%d in the function file %s\n", vmaj, vmin,
              fname);
      _Default();
    }
  }
}

SplineFunction::SplineFunction(ReadTextFile &src, int samples) {
  _arr = NULL;
  _vals = NULL;
  _Fname[0] = 0;
  _samples = 0;
  char line[256];
  fgets(line, 256, src);
  float l, u;
  if (2 == sscanf(line, "range: %f %f", &l, &u))
    _Load0000(src, samples);
  else {
    int vmaj, vmin;
    if (2 != sscanf(line, "fver %d %d", &vmaj, &vmin)) {
      Message("Error reading function in Constructor\n");
      _Default();
      return;
    }
    int ver = 100 * vmaj + vmin;
    switch (ver) {
    case 101:
      if (!_Load0101(src))
        _Default();
      break;
    default:
      Message("Unsupported function format %d.%02d\n", vmaj, vmin);
      _Default();
    }
  }
}

bool SplineFunction::_Load0000(ReadTextFile &src, int samples) {
  bool success = true;
  _LowLimit = 0.0;
  _UpLimit = 1.0;
  size_t pts;
  fscanf(src, "points: %zd\n", &pts);
  _size = pts;
  if (_size < 4) {
    Message("Error reading file %s : not enough points specified\n", _Fname);
    success = false;
  }

  _arr = new WorldPoint[_size];
  for (pts = 0; pts < _size; pts++) {
    float x, y;
    if (2 != fscanf(src, "%f %f\n", &x, &y)) {
      Message("Error reading from file %s\n", _Fname);
      x = y = 0.0f;
      success = false;
    }
    _arr[pts].Set(x, y, 0.0);
  }
  if (samples > 2) {
    VERBOSE("Precomputing values\n");
    _vals = new double[samples];
    _Precompute(samples);
    _samples = samples;
  } else
    _vals = NULL;
  return success;
}

bool SplineFunction::_Load0101(ReadTextFile &src) {
  _LowLimit = 0.0;
  _UpLimit = 1.0;
  char line[128];
  fgets(line, 128, src);
  if (1 != sscanf(line, "name: %s", _Fname)) {
    Message("Error reading function Load 0101\n");
    return false;
  }
  strcat(_Fname, ".func");
  fgets(line, 128, src);
  int samples;
  if (1 != sscanf(line, "samples: %d", &samples)) {
    Message("Error reading function Load 0101\n");
    return false;
  }
  // ignore background
  bool readImage = false;
  char c = fgetc(src);
  if (c == 'b') {
    readImage = true;
  }
  ungetc(c,src);
  if (readImage) {
    fgets(line, 128, src);
    fgets(line, 128, src);
  }
  // check for any markers 
  bool readMarkers = false;
  c = fgetc(src);
  if (c == 'm') {
    readMarkers = true;
  }
  ungetc(c,src);
  if (readMarkers) {
    int markers;
    fgets(line, 128, src);
    if (1 != sscanf(line, "markers: %d\n", &markers)) {
      fprintf(stderr, "Error reading function: Bad markers statement\n");
    }
    for (int i = 0; i < markers; i++) {
      float x, y;
      int m;
      fgets(line, 128, src);
      if (3 != sscanf(line, "m%d %f %f\n", &m, &x, &y)) {
	fprintf(stderr, "Error reading function: Bad marker\n");
      }
    }
  }

  // flip: ignored
  fgets(line, 128, src);

  fgets(line, 128, src);

  size_t pts;
  if (1 != sscanf(line, "points: %zd", &pts)) {
    Message("Error reading function Load 0101\n");
    return false;
  }
  if (pts < 4) {
    Message("Error reading file %s : not enough points specified\n", _Fname);
    return false;
  }

  std::vector<WorldPoint> arr;
  for (size_t i = 0; i < pts; i++) {
    float x, y;
    int multiplicity = 1;
    std::string coordinate;
    char c = fgetc(src);
    while (c != '\n') {
      coordinate += c;
      c = fgetc(src);
    }
    coordinate += " ";
    std::string delimiter = " ";
    size_t pos = 0;
    std::string token;
    std::vector<std::string> posVector;
    while ((pos = coordinate.find(delimiter)) != std::string::npos) {
      token = coordinate.substr(0, pos);
      if (token != "")
	posVector.push_back(token);
      coordinate.erase(0, pos + delimiter.length());
    }
    int sizeVec = posVector.size();
    if ((2 > sizeVec )||(3 < sizeVec)) {
      Message("Error reading function Load 0101\n");
      return false;
    }
    x = std::stof(posVector[0]);
    y = std::stof(posVector[1]);
    if (sizeVec == 3)
      multiplicity = std::stof(posVector[2]);
    /*
    float x, y;
    if (2 != fscanf(src, "%f %f\n", &x, &y)) {
      Message("Error reading function Load 0101\n");
      return false;
    }
    */
    for (int j = 0; j < multiplicity; ++j) {
      WorldPoint New(x, y, 0.0f);
      arr.push_back(New);
    }
  }
  _size = arr.size();
  _arr = new WorldPoint[_size];
  for (size_t i =0; i < _size; ++i){
    _arr[i].Set(arr[i]);
  }
  if (samples > 2) {
    VERBOSE("Precomputing values\n");
    _vals = new double[samples];
    _Precompute(samples);
    _samples = samples;
  } else
    _vals = NULL;
  return true;
}

bool SplineFunction::Reread() {
  if (!waitOpenFile(_Fname))
    return false;
  ReadTextFile src(_Fname);
  if (!src) {
    Message("Cannot open %s for reading\n", _Fname);
    return false;
  }
  char line[256];
  fgets(line, 256, src);
  float l, u;
  if (2 == sscanf(line, "range: %f %f", &l, &u))
    return _Reread0000(src);
  else {
    int vmaj, vmin;
    if (2 != sscanf(line, "fver %d %d", &vmaj, &vmin)) {
      Message("Error reading %s\n", _Fname);
      //_Default();
      return false;
    }
    int ver = 100 * vmaj + vmin;
    switch (ver) {
    case 101:
      return _Reread0101(src);
      break;
    default:
      Message("Unsupported format %d.%d in the function file %s\n", vmaj, vmin,
              _Fname);
      return false;
    }
  }
  return true;
}

bool SplineFunction::Reread(ReadTextFile &src) {
  //  if (!waitOpenFile(_Fname))
  //  return false;

  char line[256];
  fgets(line, 256, src);
  float l, u;
  if (2 == sscanf(line, "range: %f %f", &l, &u))
    return _Reread0000(src);
  else {
    int vmaj, vmin;
    if (2 != sscanf(line, "fver %d %d", &vmaj, &vmin)) {
      Utils::Warning("Error reading function Reread\n");
      return false;
    }
    int ver = 100 * vmaj + vmin;
    switch (ver) {
    case 101:
      return _Reread0101(src);
      break;
    default:
      Message("Unsupported function format %d.%d\n", vmaj, vmin);
      return false;
    }
  }
  return true;
}

bool SplineFunction::_Reread0000(ReadTextFile &src) {
  bool recompute = false;
  if (!src.Valid()) {
    Utils::Warning("Cannot open %s for reading\n", _Fname);
    return false;
  }
  float l, u;
  fscanf(src, "range: %f %f\n", &l, &u);
  _LowLimit = 0.0;
  _UpLimit = 1.0;
  size_t pts;
  fscanf(src, "points: %zd\n", &pts);
  if (pts < 4) {
    Utils::Warning("Error reading file %s : not enough points specified\n", _Fname);
    return false;
  }

  if (pts != _size) {
    _size = pts;
    delete[] _arr;
    _arr = new WorldPoint[_size];
    recompute = true;
  }
  for (pts = 0; pts < _size; pts++) {
    float x, y;
    if (2 != fscanf(src, "%f %f\n", &x, &y)) {
      Utils::Warning("Error reading from file %s\n", _Fname);
      return false;
      x = y = 0.0f;
    }
    double dx = x, dy = y;
    if (dx != _arr[pts].GetX())
      recompute = true;
    else if (dy != _arr[pts].GetY())
      recompute = true;
    _arr[pts].Set(x, y, 0.0);
  }
  if ((_samples > 2) && recompute) {
    VERBOSE("Recomputing values for %s\n", _Fname);
    int s = _samples;
    _samples = 0;
    _Precompute(s);
    _samples = s;
  }
  return true;
}

// [Pascal] keep opening the file until the size is stable
int SplineFunction::waitOpenFile(const char *fname) {
  int counter = 0;
  int current_size = -1;
  int size = 0;
  // get size of file
  FILE *fp = fopen(fname, "r");
  // MIK - THis is a big hack. Need to add a counter to avoid infinite loop if
  // view file is missing
  while ((fp == NULL) && (counter < 10000)) {
    fp = fopen(fname, "r");
    counter++;
  }
  if (counter == 10000) {
    fprintf(stderr, "WARNING: Can't open file %s.\n", fname);
    return 0;
  } else {
    fseek(fp, 0, SEEK_END); // seek to end of file
    size = ftell(fp);       // get current file pointer
    counter = 0;
    while (((size == 0) || (current_size != size)) && (counter < 10000)) {
      current_size = size;
      fclose(fp);

      fp = NULL;
      while (fp == NULL) {
        fp = fopen(fname, "r");
        counter++;
      }
      fseek(fp, 0, SEEK_END); // seek to end of file
      size = ftell(fp);       // get current file pointer
    }
  }
  fseek(fp, 0L, SEEK_SET);
  fclose(fp);
  if (counter >= 1000) {
    Utils::Warning("we can't reload the file : %d\n",counter);
    return 0;
  }

  return 1;
}


bool SplineFunction::_Reread0101(ReadTextFile &src) {

  bool recompute = false;
  char line[128];
  // ignore the name
  fgets(line, 128, src);

  fgets(line, 128, src);
  int samples;
  if (1 != sscanf(line, "samples: %d", &samples)) {
    return false;
  }
  if (samples != _samples)
    recompute = true;

  // flip: ignored
  fgets(line, 128, src);

  fgets(line, 128, src);
  size_t pts;
  if (1 != sscanf(line, "points: %zd", &pts)) {
    Utils::Warning("Error reading points\n");
    return false;
  }
  if (pts < 4) {
    Utils::Warning("Error reading file %s : not enough points specified\n", _Fname);
    return false;
  }

  std::vector<WorldPoint> arr;
  for (size_t i = 0; i < pts; i++) {
    float x, y;
    int multiplicity = 1;
    std::string coordinate;
    char c = fgetc(src);
    while (c != '\n') {
      coordinate += c;
      c = fgetc(src);
    }
    coordinate += " ";
    std::string delimiter = " ";
    size_t pos = 0;
    std::string token;
    std::vector<std::string> posVector;
    while ((pos = coordinate.find(delimiter)) != std::string::npos) {
      token = coordinate.substr(0, pos);
      if (token != "")
	posVector.push_back(token);
      coordinate.erase(0, pos + delimiter.length());
    }
    int sizeVec = posVector.size();
    if ((2 > sizeVec )||(3 < sizeVec)) {
      Utils::Warning("Error reading function Load 0101\n");
      return false;
    }
    x = std::stof(posVector[0]);
    y = std::stof(posVector[1]);
    if (sizeVec == 3)
      multiplicity = std::stof(posVector[2]);
    /*
    float x, y;
    if (2 != fscanf(src, "%f %f\n", &x, &y)) {
      Message("Error reading function Load 0101\n");
      return false;
    }
    */
    for (int j = 0; j < multiplicity; ++j) {
      WorldPoint New(x, y, 0.0f);
      arr.push_back(New);
    }
  }

  recompute = true;
  delete[] _arr;
  _size = arr.size();
  _arr = new WorldPoint[_size];
  for (size_t i =0; i < _size; ++i){
    _arr[i].Set(arr[i]);
    double nx = arr[i].GetX(), ny = arr[i].GetY();
    if (nx != _arr[i].GetX())
      recompute = true;
    else if (ny != _arr[i].GetY())
      recompute = true;
    _arr[i].Set(arr[i]);
  }
  if ((_samples > 2) && recompute) {
    VERBOSE("Recomputing values for %s\n", _Fname);
    int s = _samples;
    _samples = 0;
    _Precompute(s);
    _samples = s;
  }
  return true;
}

void SplineFunction::_Precompute(int s) {
  assert(s > 2);
  for (int i = 0; i < s; i++)
    _vals[i] = GetValue(double(i) / double(s - 1));
}

SplineFunction::~SplineFunction() {
  delete[] _vals;
  delete[] _arr;
}

void SplineFunction::operator=(const SplineFunction &src) {
  _LowLimit = src._LowLimit;
  _UpLimit = src._UpLimit;
  if (src._size != _size) {
    delete[] _arr;
    _arr = new WorldPoint[src._size];
    _size = src._size;
  }
  for (size_t i = 0; i < _size; i++)
    _arr[i] = src._arr[i];
  if (src._samples != _samples) {
    _samples = src._samples;
    delete[] _vals;
  }
  if (_samples > 2) {
    _vals = new double[_samples];
    for (int i = 0; i < _samples; i++)
      _vals[i] = src._vals[i];
  } else
    _vals = NULL;
  strcpy(_Fname, src._Fname);
}

double SplineFunction::GetValue(double val) const {
  if (val < _LowLimit) {
    VERBOSE("Value for %f not defined, %f used instead\n", val, _LowLimit);
    return _Value(_LowLimit);
  }

  if (val > _UpLimit) {
    VERBOSE("Value for %f not defined, %f used instead\n", val, _UpLimit);
    return _Value(_UpLimit);
  }

  return _Value(val);
}

double SplineFunction::_Value(double v) const {
  if (_samples > 2) {
    double index = v * (_samples - 1);
    int i = int(floor(index));
    double w1 = 1 - index + i;
    double w2 = index - i;
    assert(i >= 0);
    assert(i < _samples);
    if (i == _samples - 1)
      return _vals[i];
    else
      return w1 * _vals[i] + w2 * _vals[i + 1];
  } else {
    const double tofind = (v - _LowLimit) / (_UpLimit - _LowLimit);
    const double MaxError = 0.001;
    double low = _LowLimit;
    double high = _UpLimit;
    double check = 0.5;
    WorldPoint tst;
    int counter = 0;
    do {
      check = (low + high) / 2.0;
      tst = P(check);
      if (tst.GetX() > tofind)
        high = check;
      else
        low = check;
      counter++;
    } while (fabs(tst.GetX() - tofind) > MaxError);
    return tst.GetY();
  }
}

WorldPoint SplineFunction::P(double d) const {
  assert(d >= 0.0);
  assert(d <= 1.0);
  const int n = _size - 1;
  const int t = 4;
  double u = d * (n - t + 2);
  WorldPoint sum;
  for (int k = 0; k <= n; k++)
    sum += _arr[k] * N(k, t, u);
  return sum;
}

double SplineFunction::N(int k, int t, double u) const {
  if (1 == t)
    return Nk1(k, u);
  else
    return Nkt(k, t, u);
}

double SplineFunction::Nk1(int k, double u) const {
  if (Uk(k) <= u) {
    if (u < Uk(k + 1))
      return 1.0;
  }
  return 0.0;
}

double SplineFunction::Nkt(int k, int t, double u) const {
  double sum = 0.0;
  int div = Uk(k + t - 1) - Uk(k);
  if (0 != div)
    sum = (u - Uk(k)) / div * N(k, t - 1, u);

  div = Uk(k + t) - Uk(k + 1);
  if (0 != div)
    sum += (Uk(k + t) - u) / div * N(k + 1, t - 1, u);

  return sum;
}

int SplineFunction::Uk(int j) const {
  const int n = _size - 1;
  const int t = 4;
  if (j < t)
    return 0;
  if (j > n)
    return n - t + 2;
  return j - t + 1;
}

void SplineFunction::_Default() {
  _LowLimit = 0.0;
  _UpLimit = 1.0;
  delete[] _vals;
  _vals = NULL;
  delete[] _arr;
  _arr = NULL;
  _size = 4;
  _arr = new WorldPoint[_size];
  _arr[3].Set(1.0, 0.0, 0.0);
  _samples = 0;
}
