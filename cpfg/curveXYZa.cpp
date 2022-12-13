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



// source file curveXYZa.c++

#ifdef WIN32
#include "warningset.h"
#pragma warning(disable : 4189)
#endif

#include <assert.h>

#ifdef LINUX
#include <limits.h>
#endif

#include "maxpth.h"

#include "point.h"
#include "curveXYZa.h"

void CCurveXYZArray::read(const char *filename, int samples) {
  CCurveXYZ CurveXYZ(filename, samples);

  Add(CurveXYZ);
}

int CCurveXYZArray::reread() {
  for (int i = 0; i < _items; i++) {
    if (!_arr[i].rereadCurve())
      return 0;
  }
  return 1;
}

void CCurveXYZArray::reset() { _items = 0; }

const char *CCurveXYZArray::GetName(int i) const {
  assert(i >= 0);
  assert(i < Count());
  return _arr[i].GetName();
}
