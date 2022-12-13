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



#include <cstdio>
#include <string.h>

#include "function.h"
#include "utils.h"

Function::Function() : _samples(0), _LowLimit(0.0), _UpLimit(1.0) {}

Function::Function(ReadTextFile &src, const float lowLimit,
                   const float upLimit)
    : _samples(0) {
  _LowLimit = lowLimit;
  _UpLimit = upLimit;
  defaultLoaded = false;
  int vmaj, vmin;
  char line[128];
  src.ReadLine(line, 128);
  if (2 != sscanf(line, "fver %d %d", &vmaj, &vmin)) {
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
    _Default();
  }
}

Function::Function(ReadTextFile &, std::string )
    : _samples(0), _LowLimit(0.0), _UpLimit(1.0) {}

Function::Function(const Function &rhs) {
  _name = rhs._name;
  _defaultValue = rhs.GetDefaultValue();

  _LowLimit = rhs.LowLimit();
  _UpLimit = rhs.UpLimit();
  _arr = rhs._arr;
  _samples = rhs._samples;
  for (size_t i = 0; i < _samples; ++i) {
    _vals[i] = rhs._vals[i];
    _tvals[i] = rhs._tvals[i];
  }

}

Function &Function::operator=(const Function &rhs) {
  _LowLimit = rhs.LowLimit();
  _UpLimit = rhs.UpLimit();
  _arr = rhs._arr;
  _samples = rhs._samples;
  for (size_t i = 0; i < _samples; ++i) {
    _vals[i] = rhs._vals[i];
    _tvals[i] = rhs._tvals[i];
  }
  _name = rhs._name;
  _defaultValue = rhs.GetDefaultValue();
  return *this;
}


float Function::GetValue(float v) const {
  ASSERT(v >= 0.0f);
  ASSERT(v <= 1.0f);
  float index = v * (_Samples() - 1);
  size_t i = static_cast<size_t>(floor(index));
  float w1 = 1 - index + i;
  float w2 = index - i;
  ASSERT(i < _Samples());
  if (i == _Samples() - 1)
    return _vals[i];
  else
    return w1 * _vals[i] + w2 * _vals[i + 1];
}

Vector3d Function::GetTangent(float v) const {
  ASSERT(v >= 0.0f);
  ASSERT(v <= 1.0f);
  float index = v * (_Samples() - 1);
  size_t i = static_cast<size_t>(floor(index));
  float w1 = 1 - index + i;
  float w2 = index - i;
  ASSERT(i < _Samples());
  if (i == _Samples() - 1)
    return _tvals[i];
  else
    return w1 * _tvals[i] + w2 * _tvals[i + 1];
}

float Function::_CalcParam(float v) const {

  float low = 0.;
  float high = 1.;
  const float tofind = (v - low) / (high - low);
  const float MaxError = 0.01f / _samples;
  Vector3d tst;
  int counter = 0;
  float check;
  do {
    check = (low + high) / 2.0f;
    tst = P(check);
    if (tst.X() > tofind)
      high = check;
    else
      low = check;
    ++counter;
  } while (fabs(tst.X() - tofind) > MaxError);
  return check;
}

bool Function::_Load0101(ReadTextFile &src) {
  char line[128];
  src.ReadLine(line, 128);
  if (0 == strncmp(line, "name: ", 6)) {
    _name = line + 6;
    if (_name.empty()) {
      return false;
    }
  } else {
    return false;
  }
  src.ReadLine(line, 128);
  {
    int samples;
    if (1 != sscanf(line, "samples: %d", &samples)) {
      return false;
    }
    if (samples < eMinSamples) {
      Utils::Message("samples value too small. Setting to %d\n", eMinSamples);
      _samples = eMinSamples;
    } else if (samples > eMaxSamples) {
      Utils::Message("samples value too big. Setting to %d\n", eMaxSamples);
      _samples = eMaxSamples;
    } else
      _samples = samples;
  }

  // ignore background and flip
  src.ReadLine(line, 128);
  bool readImage = false;
  if (0 == strncmp(line, "background: ", 12)) {
    readImage = true;
  }
  if (readImage) {
    src.ReadLine(line,128);
    src.ReadLine(line,128);

  }
  
  src.ReadLine(line,128);

  int pts;
  if (1 != sscanf(line, "points: %d", &pts)) {
    return false;
  }
  if (pts < 4) {
    Utils::Message("Not enough points specified in function\n");
    return false;
  }

  for (int i = 0; i < pts; ++i) {
    src.ReadLine(line, 128);
    if (!src.good()) {
      std::cerr<<"Files is not good"<<std::endl;
      return false;
    }

    std::string coordinate(line);
    coordinate += " ";
    float x, y;
    int multiplicity = 1;    
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
    if ((sizeVec < 2 )||(sizeVec > 3)) {
      std::cerr<<"Wrong point information: "<<sizeVec<<std::endl;
      return false;
    }
    x = std::stof(posVector[0]);
    y = std::stof(posVector[1]);
    if (sizeVec == 3)
      multiplicity = std::stof(posVector[2]);
    
    for (int j = 0; j < multiplicity; ++j) {
      Vector3d New(x, y, 0.0f);
      _arr.push_back(New);
    }
  }

  _Precompute(_samples);

  return true;
}

void Function::_Precompute(size_t s) {
  ASSERT(s > 2);
  for (size_t i = 0; i < s; ++i) {
    float tval = _CalcParam(float(i) / float(s - 1));
    _vals[i] = P(tval).Y();
    _tvals[i] = T(tval);
  }
}

void Function::_Default() {
  defaultLoaded = true;
  Vector3d def(0.0f, 0.0f, 0.0f);
  _arr.push_back(def);
  def.X(1.0f / 3.0f);
  _arr.push_back(def);
  def.X(2.0f / 3.0f);
  _arr.push_back(def);
  def.X(1.0f);
  _arr.push_back(def);
}
