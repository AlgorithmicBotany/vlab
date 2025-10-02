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



#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include <iostream>

#include "maxpath.h"

#ifdef WIN32
#include <GL/glew.h>
#include <windows.h>
#include <GL/gl.h>
#else
#include <qgl.h>
#endif

class Texture {
public:
  Texture(const char *);
  bool Bind();
  bool Reread();
  void MakeActive() const;
  bool Initialized() const { return _id != tUninitialized; }
  const char *getFilename() const { return _fname; }
  void OutputToPOVRay(std::ostream &stream) const;
  void OutputToPOVRayAlphaMapped(std::ostream &stream) const;

private:
  unsigned char *_LoadRGB(const char *);
  unsigned char *_LoadPNG(const char *);
  char _fname[MaxPath];

  int _Xsize, _Ysize, _Zsize;

  enum { tUninitialized = 65535 };
  GLuint _id;
};

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
