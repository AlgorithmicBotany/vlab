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



#ifndef __POVRAYINTRFC_H__
#define __POVRAYINTRFC_H__

#include <iostream>

#include "vector3d.h"
#include "asrt.h"

namespace POVRay {
class Triangle {
public:
  Triangle() : _empty(true), _nsset(false), _uvset(false) {}
  void SetVertices(Vector3d, Vector3d, Vector3d);
  void SetNormals(Vector3d, Vector3d, Vector3d);
  void SetUV(Vector3d, Vector3d, Vector3d);
  bool Empty() const { return _empty; }
  bool Flat() const { return !_nsset; }
  bool Textured() const { return _uvset; }
  void Clear() {
    _empty = true;
    _nsset = false;
    _uvset = false;
  }
  Vector3d V(int i) const {
    ASSERT(i >= 0);
    ASSERT(i < 3);
    return _vs[i];
  }
  Vector3d N(int i) const {
    ASSERT(i >= 0);
    ASSERT(i < 3);
    return _ns[i];
  }
  Vector3d U(int i) const {
    ASSERT(i >= 0);
    ASSERT(i < 3);
    return _uv[i];
  }
  char currentTexture[100];

private:
  bool _empty;
  bool _nsset;
  bool _uvset;
  Vector3d _vs[3];
  Vector3d _ns[3];
  Vector3d _uv[3];
};

class Mesh {
public:
  Mesh(std::ostream &trg);
  ~Mesh();
  void AddTriangle(const Triangle &);
  bool Empty() const { return _empty; }

private:
  std::ostream &_trg;
  bool _empty;
};

std::ostream &operator<<(std::ostream &, const Triangle &);
void Dump(std::ostream &trg, Vector3d v);
void DumpPair(std::ostream &trg, Vector3d v);

} // namespace POVRay

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
