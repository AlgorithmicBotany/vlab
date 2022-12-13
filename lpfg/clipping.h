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



#ifndef __CLIPPING_H__
#define __CLIPPING_H__

#include "asrt.h"

class Clipping {
public:
  Clipping() { Clear(); }
  void Clear() {
    _frontspecified = false;
    _backspecified = false;
  }
  void SetFront(float f) {
    _frontspecified = true;
    _front = f;
  }
  void SetBack(float f) {
    _backspecified = true;
    _back = f;
  }
  bool Specified() const { return _frontspecified && _backspecified; }
  float Front() const {
    ASSERT(Specified());
    return _front;
  }
  float Back() const {
    ASSERT(Specified());
    return _back;
  }

private:
  bool _frontspecified;
  float _front;
  bool _backspecified;
  float _back;
};

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
