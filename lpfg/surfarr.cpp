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



#include "surfarr.h"
#include "glutils.h"
#include "utils.h"

SurfaceArray::SurfaceArray() { _listBase = 0; }

bool SurfaceArray::AddSurface(const char *cmnd) {
  bool found = false;
  std::string tfile = std::string(cmnd);
  for (size_t i = 0; (i < _surfaceFile.size()) && (!found); ++i) {
    if (tfile.compare(_surfaceFile[i]) == 0) {
      return operator[](i).Reread();
    }
  }
  Surface newsurf(cmnd);
  if (newsurf.loaded()) {
    push_back(newsurf);
    _surfaceFile.push_back(cmnd);
  }
  return true;
}

void SurfaceArray::Clear() {
  if (0 != _listBase)
    glDeleteLists(_listBase, static_cast<GLsizei>(size()));
  if (!empty()) {
    clear();
    _surfaceFile.clear();
  }
}

bool SurfaceArray::Reread() {
  for (iterator it = begin(); it != end(); ++it) {
    bool success = it->Reread();
    if (!success)
      return false;
  }
  return true;
}

void SurfaceArray::Record() {}

void SurfaceArray::Draw(size_t id, float sx, float sy, float sz,
                        const UVPrecision &precision) {
  ASSERT(ValidId(id));
  glScalef(sx, sy, sz);
  Surface &s = operator[](id);
  glScalef(s.Scale(), s.Scale(), s.Scale());
  s.Draw(precision);
}

void SurfaceArray::DisableTexture(size_t id) {
  ASSERT(ValidId(id));
  Surface &surface = operator[](id);
  surface.DisableTexture();
}

void SurfaceArray::OutputToPOVRay(std::ostream &stream,
                                  const char *append) const {
  int i = 0;
  const_iterator itr = begin();
  while (itr != end()) {
    stream << "#declare Surface_" << i << append << " = ";
    i++;
    itr->OutputToPOVRay(stream);
    stream << std::endl;
    itr++;
  }
}

void SurfaceArray::OutputToPOVRay(size_t id, std::ostream &stream,
                                  const char *currentTexture,
                                  OpenGLMatrix &trans, OpenGLMatrix &rot,
                                  OpenGLMatrix &scale) {
  ASSERT(ValidId(id));
  Surface &surface = operator[](id);
  surface.OutputToPOVRay(stream, currentTexture, trans, rot, scale);
}
