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



#ifdef _WINDOWS
#include <GL/glew.h>
#include <windows.h>
#include <GL/gl.h>
#else
#include <qgl.h>
#endif
 
#include "mesh.h"
#include "asrt.h"
#include "drawparam.h"
#include "glenv.h"
#include "glutils.h"

void Mesh::AddPoint(Vector3d v, int c) {
  ASSERT(!_mesh.empty());
  (--(_mesh.end()))->push_back(std::make_pair(v, c));
}

void Mesh::_IteratePoints(bool filled) const {
  if (_mesh.size() <= 1)
    return;

  if (filled)
    glPolygonMode(GL_FRONT, GL_FILL);
  else
    glPolygonMode(GL_FRONT, GL_LINE);

  const _Mesh::const_iterator startRow = _mesh.begin();
  const _Mesh::const_iterator endRow = _mesh.end();

  for (_Mesh::const_iterator current = startRow; current != endRow; ++current) {
    _Mesh::const_iterator next = (++current)--; // <--- WOW !!!

    if (current->empty() || next->empty())
      continue;

    _MeshRow::const_iterator c_point = current->begin();
    _MeshRow::const_iterator n_point = next->begin();

    _MeshRow::const_iterator c_last = --current->end();
    _MeshRow::const_iterator n_last = --next->end();

    const float ratio =
        static_cast<float>(current->size()) / static_cast<float>(next->size());
    const int reps =
        static_cast<int>((ratio >= 1.0f) ? ceil(ratio) : ceil(1.0 / ratio));
    int i = reps;
    _MeshRow::const_iterator c_prev = c_point, n_prev = n_point;

    GLprimitive tstrip(GL_TRIANGLE_STRIP);

    gl.SetColor(c_point->second);
    c_prev = c_point++;
    _PlaceNormal(c_point->first, c_prev->first, n_point->first);
    glVertex3fv(c_prev->first);

    gl.SetColor(n_point->second);
    n_prev = n_point++;
    glVertex3fv(n_prev->first);

    do {
      _PlaceNormal(n_point->first, c_point->first, c_prev->first);
      gl.SetColor(c_point->second);
      glVertex3fv(c_point->first);
      gl.SetColor(n_point->second);
      glVertex3fv(n_point->first);

      if (ratio >= 1.0f) {
        // More in the current row than the next
        if (c_point != c_last)
          c_prev = c_point++;
        if (--i == 0) {
          if (n_point != n_last)
            n_prev = n_point++;
          i = reps;
        }
      } else {
        // More in the next row than the current
        if (n_point != n_last)
          n_prev = n_point++;
        if (--i == 0) {
          if (c_point != c_last)
            c_prev = c_point++;
          i = reps;
        }
      }
    } while (c_point != c_last && n_point != n_last);

    if (c_point != c_last)
      c_prev = c_point;
    if (n_point != n_last)
      n_prev = n_point;

    _PlaceNormal(n_last->first, c_last->first, c_prev->first);
    gl.SetColor(c_last->second);
    glVertex3fv(c_last->first);
    gl.SetColor(c_last->second);
    glVertex3fv(n_last->first);
  }

  switch (drawparams.RenderMode()) {
  case DParams::rmWireframe:
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    break;
  case DParams::rmFilled:
  case DParams::rmShaded:
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    break;
  default:
    Utils::Message("enumeration value 'rmShadows' not handled \n");
    break;
  }
}

void Mesh::_PlaceNormal(const Vector3d &left, const Vector3d &mid,
                        const Vector3d &right) const {
  Vector3d a = left - mid, b = right - mid;
  Vector3d n = a % b;
  n.Normalize();
  glNormal3fv(n);
}
