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



#include "materialset.h"
#include "utils.h"

MaterialSet::MaterialSet() : _set(false) {}

void MaterialSet::Load(const char *fnm) {
  std::ifstream src(fnm, std::ios::in | std::ios::binary);
  if (!src.good()) {
    Utils::Message("Cannot read materials from file %s\n", fnm);
    return;
  }

  while (!src.eof()) {
    unsigned char c;
    src.read(reinterpret_cast<char *>(&c), 1);
    if (src.good())
      _material[c].Read(src);
    else
      break;
  }
  _set = true;
}
