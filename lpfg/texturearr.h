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



#ifndef __TEXTUREARR_H__
#define __TEXTUREARR_H__

#include <vector>
#include <string>

#include "texture.h"

class TextureArray : private std::vector<Texture> {
public:
  void Add(const char *);
  bool Reread();
  bool Bind();
  void MakeActive(int) const;
  const char *getFilename(int) const;
  bool IsValid(size_t id) const { return (id < size()); }
  void OutputToPOVRay(std::ostream &stream, const char *append) const;
  void Clear();
  int NumTextures() const;
  bool Initialized(size_t id) const { return operator[](id).Initialized(); }
  std::vector<std::string> _textureFile;
};

extern TextureArray textures;

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
