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



#include "tropismarr.h"

void TropismArray::Add(const Tropism &src) {
  push_back(src);
  _initialTropism.push_back(src);

  if (src.Type() == Tropism::trTropism)
    _GlobalDirection += src.Direction() * src.CosGamma() * src.Elasticity();
  else {
    _TwistDirection += src.Direction() * src.Elasticity();
    _TwistLength = _TwistDirection.Length();
  }
}

void TropismArray::SetElasticity(size_t id, float e) {
  ASSERT(ValidId(id));
  Tropism &tropism = operator[](id);
  const float de = e - tropism.Elasticity();
  tropism.SetElasticity(e);
  Vector3d v = tropism.Direction() * de * tropism.CosGamma();
  _GlobalDirection += v;
}

void TropismArray::IncElasticity(size_t id) {
  ASSERT(ValidId(id));
  Tropism &tropism = operator[](id);
  SetElasticity(id, tropism.Elasticity() + tropism.GetStep());
}

void TropismArray::DecElasticity(size_t id) {
  ASSERT(ValidId(id));
  Tropism &tropism = operator[](id);
  SetElasticity(id, tropism.Elasticity() - tropism.GetStep());
}

void TropismArray::Update(const std::vector<float> &arr) {
  _GlobalDirection.Set(0.0f, 0.0f, 0.0f);
  typedef std::vector<Tropism>::iterator TropismIter;
  int i = 0;
  for (TropismIter it = begin(); it != end(); ++it) {
    it->SetElasticity(arr[i]);
    _GlobalDirection += it->Direction() * it->CosGamma() * it->Elasticity();
    ++i;
  }
}
