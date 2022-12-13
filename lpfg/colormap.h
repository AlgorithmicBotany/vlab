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



#ifndef __COLORMAP_H__
#define __COLORMAP_H__

#include "lpfgparams.h"

class Colormap {
public:
  Colormap() {
    SetColor(0, 0, 0, 0);
    for (int i = 1; i < LPFGParams::NumOfColors; ++i)
      SetColor(i, 255, 255, 255);
  }
  const unsigned char *GetColor(int i) const { return _Colormap + 3 * i; }
  void GetColor(int i, float arr[3]) const {
    arr[0] = _Colormap[i * 3] / 255.0f;
    arr[1] = _Colormap[i * 3 + 1] / 255.0f;
    arr[2] = _Colormap[i * 3 + 2] / 255.0f;
  }
  void Load(const char *);

private:
  void SetColor(int i, unsigned char r, unsigned char g, unsigned char b) {
    _Colormap[i * 3] = r;
    _Colormap[i * 3 + 1] = g;
    _Colormap[i * 3 + 2] = b;
  }
  unsigned char _Colormap[LPFGParams::NumOfColors * 3];
};

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
