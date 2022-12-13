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



#include "texturearr.h"
#include "utils.h"
#include "exception.h"
#include "comlineparam.h"

void TextureArray::Add(const char *fname) {
  std::string tfile = std::string(fname);
  _textureFile.push_back(tfile);
  Texture newtex(fname);
  push_back(newtex);
  comlineparam.SetTexturefile(fname);
}


bool TextureArray::Reread() {
  for (iterator it = begin(); it != end(); ++it) {
    bool success = it->Reread();
    if (!success)
      return false;
  }
  return true;
}


bool TextureArray::Bind() {
  iterator itr = begin();
  while (itr != end()) {
    try {
      if (!itr->Bind())
        return false;
    } catch (Exception e) {
      Utils::Error(e.Msg());
    }

    ++itr;
  }
  return true;
}

void TextureArray::MakeActive(int i) const {
  if (operator[](i).Initialized())
    operator[](i).MakeActive();
  else
    Utils::Message("Uninitialized texture id = %d\n", i);
}

const char *TextureArray::getFilename(int i) const {
  return operator[](i).getFilename();
}

void TextureArray::OutputToPOVRay(std::ostream &stream,
                                  const char *append) const {
  int i = 0;
  const_iterator itr = begin();
  while (itr != end()) {
    stream << "#if (USE_ALPHA_MAPPED_TEXTURES=1)\n";
    stream << "#declare Texture_" << i << append << " = ";
    itr->OutputToPOVRayAlphaMapped(stream);
    stream << std::endl;

    stream << "#else\n";
    stream << "#declare Texture_" << i << append << " = ";
    itr->OutputToPOVRay(stream);
    stream << std::endl;

    stream << "#end\n";

    i++;
    itr++;
  }
}

void TextureArray::Clear(void) {
  if (!empty()) {
    clear();
    _textureFile.clear();
    comlineparam.clearTextureFiles();
  }
}

int TextureArray::NumTextures() const { return size(); }
