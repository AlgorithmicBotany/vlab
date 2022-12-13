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



#ifndef __TROPISM_H__
#define __TROPISM_H__

#include "asrt.h"
#include "vector3d.h"

class Tropism {
public:
  enum trType { trTropism, trTorque };
  Tropism(const char *, trType);

  void SetDirection(const float *arr) { _direction.Set(arr); }
  void SetAngle(float g) {
    ASSERT(g >= -M_PI);
    ASSERT(g <= M_PI);
    _gamma = g;
  }
  void SetElasticity(float);
  float GetStep() const { return _step; }
  const Vector3d &Direction() const { return _direction; }
  float SinGamma() const { return _singamma; }
  float EsinGamma() const { return _esingamma; }
  float CosGamma() const { return _cosgamma; }
  float Elasticity() const { return _elasticity; }
  trType Type() const { return _type; }

private:
  void _Default();
  void _Load(const char *);
  void _Update();

  Vector3d _direction;
  float _gamma;
  float _singamma, _cosgamma, _esingamma;
  float _elasticity;
  float _step;

  trType _type;
};

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
