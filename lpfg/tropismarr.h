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



#ifndef __TROPISMARR_H__
#define __TROPISMARR_H__

#include <cstddef>
#include <vector>

#include "tropism.h"

class TropismArray : public std::vector<Tropism> {
public:
  TropismArray() : _TwistLength(0.0f) {}
  void Add(const Tropism &src);
  void resetToInitialTropism() {
    Clear();
    for (size_t i = 0; i < _initialTropism.size(); ++i) {
      Tropism src = _initialTropism[i];
      push_back(src);
      if (src.Type() == Tropism::trTropism)
        _GlobalDirection += src.Direction() * src.CosGamma() * src.Elasticity();
      else {
        _TwistDirection += src.Direction() * src.Elasticity();
        _TwistLength = _TwistDirection.Length();
      }
    }
  }

  void Reset() {
    if (!empty())
      clear();
    if (!_initialTropism.empty())
      _initialTropism.clear();

    _GlobalDirection.Set(0.0f, 0.0f, 0.0f);
    _TwistDirection.Set(0.0f, 0.0f, 0.0f);
    _TwistLength = 0.0f;
  }

  void Clear() {
    if (!empty())
      clear();
    _GlobalDirection.Set(0.0f, 0.0f, 0.0f);
    _TwistDirection.Set(0.0f, 0.0f, 0.0f);
    _TwistLength = 0.0f;
  }

  Vector3d GlobalDirection() const { return _GlobalDirection; }
  bool ValidId(size_type id) const { return id < size(); }
  void SetElasticity(size_t, float);
  void IncElasticity(size_t);
  void DecElasticity(size_t);
  float GetStep(size_t id) const {
    ASSERT(ValidId(id));
    return operator[](id).GetStep();
  }
  float GetElasticity(size_t id) const {
    ASSERT(ValidId(id));
    return operator[](id).Elasticity();
  }
  void Update(const std::vector<float> &);
  float TwistLength() const { return _TwistLength; }
  Vector3d Twist() const { return _TwistDirection; }

private:
  Vector3d _GlobalDirection;
  Vector3d _TwistDirection;
  float _TwistLength;
  std::vector<Tropism> _initialTropism;
};

extern TropismArray Tropisms;

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
