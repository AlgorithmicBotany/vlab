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



#include <sstream>
#include <string>

#include "povray.h"

void POVRay::Triangle::SetVertices(Vector3d v0, Vector3d v1, Vector3d v2) {
  _vs[0] = v0;
  _vs[1] = v1;
  _vs[2] = v2;

  _empty = false;

  std::stringstream sv0;
  Dump(sv0, v0);
  std::stringstream sv1;
  Dump(sv1, v1);
  std::stringstream sv2;
  Dump(sv2, v2);
  std::string ssA(sv0.str());
  std::string ssB(sv1.str());
  if (0 == ssA.compare(ssB))
    _empty = true;
  ssB = sv2.str();
  if (0 == ssA.compare(ssB))
    _empty = true;
  ssA = sv1.str();
  if (0 == ssA.compare(ssB))
    _empty = true;
}

void POVRay::Triangle::SetNormals(Vector3d n0, Vector3d n1, Vector3d n2) {
  _ns[0] = n0;
  _ns[1] = n1;
  _ns[2] = n2;
  _nsset = true;
}

void POVRay::Triangle::SetUV(Vector3d uv0, Vector3d uv1, Vector3d uv2) {
  _uv[0] = uv0;
  _uv[1] = uv1;
  _uv[2] = uv2;
  _uvset = true;
}

std::ostream &POVRay::operator<<(std::ostream &os, const POVRay::Triangle &t) {
  if (t.Flat()) {
    os << "triangle" << std::endl << '{' << std::endl;
    Dump(os, t.V(0));
    os << ',';
    Dump(os, t.V(1));
    os << ',';
    Dump(os, t.V(2));
    os << std::endl;
  } else {
    os << "smooth_triangle" << std::endl << '{' << std::endl;
    Dump(os, t.V(0));
    os << ',';
    Dump(os, t.N(0));
    os << ',' << std::endl;
    Dump(os, t.V(1));
    os << ',';
    Dump(os, t.N(1));
    os << ',' << std::endl;
    Dump(os, t.V(2));
    os << ',';
    Dump(os, t.N(2));
    os << std::endl;
  }
  if (t.Textured()) {
    os << "uv_vectors ";
    DumpPair(os, t.U(0));
    os << ',';
    DumpPair(os, t.U(1));
    os << ',';
    DumpPair(os, t.U(2));
    os << std::endl;

    os << "texture { " << t.currentTexture << "}" << std::endl;
  }
  os << '}' << std::endl;
  return os;
}
