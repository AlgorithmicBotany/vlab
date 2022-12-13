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

#include "tropismdata.h"

#include "tropismarr.h"

TropismData::TropismData() : _elasticity(Tropisms.size()) {
  for (TropismArray::size_type i = 0; i < Tropisms.size(); ++i)
    _elasticity[i] = Tropisms.GetElasticity(i);
}

void TropismData::UpdateTropisms() const { Tropisms.Update(_elasticity); }

void TropismData::resetToInitialTropism() const {

  Tropisms.resetToInitialTropism();
}

bool SimpleTropism::Read(const char *line) {
  float x, y, z, e;
  int res = sscanf(line, "%f %f %f, %f", &x, &y, &z, &e);
  if (4 != res)
    return false;
  _direction.Set(x, y, z);
  _elasticity = e;
  return true;
}
