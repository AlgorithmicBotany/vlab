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



#ifndef __TROPISMDATA_H__
#define __TROPISMDATA_H__

#include <vector>

#include "vector3d.h"

class TropismData {
public:
  TropismData();
  void SetElasticity(size_t id, float v) { _elasticity[id] = v; }
  float GetElasticity(size_t id) const { return _elasticity[id]; }
  void UpdateTropisms() const;
  void resetToInitialTropism() const;

private:
  std::vector<float> _elasticity;
};

class SimpleTropism {
public:
  SimpleTropism() { Default(); }
  void Default() {
    _direction.Set(0.0f, 1.0f, 0.0f);
    _elasticity = 0.0f;
  }
  bool Read(const char *);
  Vector3d Direction() const { return _direction; }
  void Direction(Vector3d dir) { _direction = dir; }
  float Elasticity() const { return _elasticity; }
  void Elasticity(float e) { _elasticity = e; }

private:
  Vector3d _direction;
  float _elasticity;
};

extern SimpleTropism STropism;

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
