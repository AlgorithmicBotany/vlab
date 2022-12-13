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



#ifndef __SURFARR_H__
#define __SURFARR_H__

#include "surface.h"

class SurfaceArray : private std::vector<Surface> {
public:
  SurfaceArray();
  bool AddSurface(const char *);
  void Clear();
  void Draw(size_t id, float sx, float sy, float sz, const UVPrecision &);
  void GetVolume(size_t id, const float rot[16], Volume &v) const {
    ASSERT(ValidId(id));
    operator[](id).GetVolume(rot, v);
  }
  bool ValidId(size_t id) const { return (id < size()); }
  void Record();
  bool IsTextured(size_t id) const { return operator[](id).IsTextured(); }
  int TextureId(size_t id) const { return operator[](id).TextureId(); }
  void SetTextureId(size_t id, int textureId) {
    operator[](id).SetTextureId(textureId);
  }
  void DisableTexture(size_t);
  void OutputToPOVRay(std::ostream &, const char *) const;
  void OutputToPOVRay(size_t id, std::ostream &, const char *, OpenGLMatrix &,
                      OpenGLMatrix &, OpenGLMatrix &);
  const Surface &Get(size_t id) const { return operator[](id); }
  bool Reread();
  size_t Count() const { return size(); }

private:
  unsigned int _listBase;
  std::vector<std::string> _surfaceFile;
};

extern SurfaceArray surfaces;

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
