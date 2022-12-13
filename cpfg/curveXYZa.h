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



// header file curveXYZa.h

#ifndef __CURVEXYZA_H__
#define __CURVEXYZA_H__

#include "dynarray.h"
#include "curveXYZ.h"

class CCurveXYZArray : public DynArray<CCurveXYZ> {
public:
  CCurveXYZArray() : DynArray<CCurveXYZ>(16) {}

  void read(const char *filename, int samples);
  int reread();
  void reset();
  const char *GetName(int i) const;
};

#endif
