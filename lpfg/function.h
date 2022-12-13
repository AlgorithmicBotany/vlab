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



#ifndef __FUNCTION_H__
#define __FUNCTION_H__

#include <fstream>
#include <string>
#include <vector>
#include <iostream>

#include "asrt.h"
#include "vector3d.h"
#include "file.h"

class Function {
public:
  Function();
  Function(const Function &);
  Function(ReadTextFile &, const float lowLimit = 0.0,
           const float upLimit = 1.0);
  Function(ReadTextFile &, std::string);
  Function(const std::string name, const float defaultValue): _samples(0), _LowLimit(0.0), _UpLimit(1.0){
    _name = name;
    _defaultValue = defaultValue;
    defaultLoaded = false;
  }

  float GetDefaultValue() const{
   return _defaultValue;
  }
    
					 
  void SetLowLimit(const float lowLimit) { _LowLimit = lowLimit; }
  void SetUpLimit(const float upLimit) { _UpLimit = upLimit; }
  void SetLimit(const float lowLimit, const float upLimit) {
    _LowLimit = lowLimit;
    _UpLimit = upLimit;
  }

  float LowLimit() const { return _LowLimit; }

  float UpLimit() const { return _UpLimit; }

  float GetValue(float) const;
  Vector3d GetTangent(float v) const;
  const char *GetName() const { return _name.c_str(); }

  Function &operator=(const Function &);
  bool isDefaultLoaded() { return defaultLoaded; }

private:
  bool defaultLoaded;
  std::vector<Vector3d> _arr; // control points
  enum { ePreCompCount = 100, eMinSamples = 10, eMaxSamples = 1000 };
  float _vals[eMaxSamples];
  Vector3d _tvals[eMaxSamples];
  size_t _samples;

  std::string _name;
  float _defaultValue;

  enum { eInitSize = 8 };

  float _CalcParam(float) const;
  size_t _Samples() const { return _samples; }
  size_t _PointsCount() const { return _arr.size(); }
  void _Precompute(size_t);

  bool _Load0101(ReadTextFile &);

  void _Default();

  Vector3d P(float d) const // evaluate B-spline point
  {
    ASSERT(d >= 0.0f);
    ASSERT(d <= 1.0f);
    const size_t n = _PointsCount() - 1;
    const size_t t = 4;
    float u = d * (n - t + 2);
    Vector3d sum;
    for (size_t k = 0; k <= n; ++k)
      sum += _arr[k] * N(k, t, u);
    return sum;
  }

  Vector3d T(float d) const // evaluate B-spline tangent vector
  {
    ASSERT(d >= 0.0f);
    ASSERT(d <= 1.0f);
    const size_t n = _PointsCount() - 1;
    const size_t t = 4;
    float u = d * (n - t + 2);
    Vector3d sum;
    for (size_t k = 0; k <= n; ++k) {
      // The derivative of B-spline basis N(k,t,u)
      // is a weighted sum of N(k,t-1,u) and N(k+1,t-1,u)
      float Nprime = 0;
      int knotDiff = Uk(k + t - 1) - Uk(k);
      if (knotDiff > 0)
        Nprime += ((t - 1.) / knotDiff) * N(k, t - 1, u);
      knotDiff = Uk(k + t) - Uk(k + 1);
      if (knotDiff > 0)
        Nprime -= ((t - 1.) / knotDiff) * N(k + 1, t - 1, u);

      sum += _arr[k] * Nprime;
    }
    return sum;
  }

  // de Boor's algorithm for B-spline evaluation
  float N(size_t k, size_t t, float u) const {
    if (1 == t)
      return Nk1(k, u);
    else
      return Nkt(k, t, u);
  }
  float Nk1(size_t k, float u) const {
    if (Uk(k) <= u) {
      if (u < Uk(k + 1))
        return 1.0f;
    }
    return 0.0f;
  }
  float Nkt(size_t k, size_t t, float u) const {
    float sum = 0.0f;
    const size_t uk = Uk(k);
    size_t div = Uk(k + t - 1) - uk;
    if (0 != div)
      sum = (u - uk) / div * N(k, t - 1, u);

    const size_t ukt = Uk(k + t);
    div = ukt - Uk(k + 1);
    if (0 != div)
      sum += (ukt - u) / div * N(k + 1, t - 1, u);

    return sum;
  }
  // knot values: 0,0,0,0, 1, 2, ... (n-1), n,n,n,n
  size_t Uk(size_t j) const {
    const size_t t = 4;
    if (j < t)
      return 0;
    const size_t n = _PointsCount() - 1;
    if (j > n)
      return n - t + 2;
    return j - t + 1;
  }

  float _LowLimit;
  float _UpLimit;
};

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
