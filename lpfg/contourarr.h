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



#ifndef __CONTOURARRAY_H__
#define __CONTOURARRAY_H__

#include "contour.h"

class ContourArray : private std::vector<Contour> {
public:
  ContourArray();
  void Clear();
  bool Load(const char *);

  bool LoadIndividualContours(const std::vector<std::string>);
  bool ValidId(size_type id) const { return id < size(); }

  Contour &GetNC(size_type n) {
    ASSERT(ValidId(n));
    return operator[](n);
  }
  const Contour &Get(size_type n) const {
    ASSERT(ValidId(n));
    return operator[](n);
  }
  const Contour &Get1(size_type n1, size_type n2, float b) {
    if (0.0f == b)
      return Get(n1);
    else if (1.0f == b)
      return Get(n2);
    else {
      _bc1.Blend(Get(n1), Get(n2), b);
      return _bc1;
    }
  }
  const Contour &Get2(size_type n1, size_type n2, float b) {
    if (0.0f == b)
      return Get(n1);
    else if (1.0f == b)
      return Get(n2);
    else {
      _bc2.Blend(Get(n1), Get(n2), b);
      return _bc2;
    }
  }
  const char *GetName(size_type) const;
  Contour &GetAccess(size_type n) {
    ASSERT(ValidId(n));
    return operator[](n);
  }
  void CopyCurve(size_type) {}

private:
  BlendedContour _bc1;
  BlendedContour _bc2;
};

extern ContourArray contours;

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
