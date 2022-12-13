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



#include <fstream>

#ifdef _WINDOWS
#include <windows.h>
#include <GL/gl.h>
#else
#include <qgl.h>
#endif
#include <iostream>
#include "colormap.h"
#include "exception.h"
#include "utils.h"
#include "comlineparam.h"

void Colormap::Load(const char *fnm) {
  std::ifstream src(fnm, std::ios::in | std::ios::binary);
  for (int i = 0; i < LPFGParams::NumOfColors; ++i) {
    int count = 0;
    while (!src.good() && (count < 10000)) {
      ++count;
    }
    if (!src.good()) {
      if (comlineparam.VerboseMode())	
	Utils::Message("Warning! Can't read colormap file\n");
      return;
    }
    src.read(reinterpret_cast<char *>(_Colormap + 3 * i), 3);
  }
}
