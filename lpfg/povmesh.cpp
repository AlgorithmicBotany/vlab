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



#include "povray.h"

POVRay::Mesh::Mesh(std::ostream &trg) : _trg(trg), _empty(true) {}

POVRay::Mesh::~Mesh() {
  if (!Empty())
    _trg << std::endl << '}' << std::endl;
}

void POVRay::Mesh::AddTriangle(const POVRay::Triangle &trngl) {
  assert(!trngl.Empty());
  if (Empty()) {
    _trg << "mesh" << std::endl << '{' << std::endl;
    _empty = false;
  }
  _trg << trngl << std::endl;
}
