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



#include "contourarr.h"
#include "file.h"
#include "exception.h"

ContourArray contours;

ContourArray::ContourArray() { Clear(); }

void ContourArray::Clear() {
  clear();
  Contour def;
  push_back(def);
}

bool ContourArray::Load(const char *csetfile) {
  Clear();
  bool success = true;
  try {
    ReadTextFile src(csetfile,true);
    if (!src.good())
      return false;
    const int BfSize = 64;
    char ln[BfSize];
    src.ReadLine(ln, BfSize);
    int vmaj, vmin;
    if (2 != sscanf(ln, "contourgalleryver %d %d", &vmaj, &vmin))
      return false;
    //			throw Exception("Error reading contour gallery from
    //%s\n", csetfile);
    int n;
    src.ReadLine(ln, BfSize);
    if (1 != sscanf(ln, "items: %d", &n))
      return false;
    //			throw Exception("Error reading contour gallery from
    //%s\n", csetfile);
    if (n < 1)
      return false;
    //			throw Exception("Error reading contour gallery from
    //%s\n", csetfile);
    for (int i = 0; i < n; ++i) {
      Contour cnt(src);
      if (cnt.isDefaultLoaded()) {
        success = false;
        return false;
      }
      push_back(cnt);
    }
  } catch (Exception) {
    if (0 == size()) {
      Contour def;
      push_back(def);
    }
    throw;
  }
  return success;
}

bool ContourArray::LoadIndividualContours(
    const std::vector<std::string> contoursFileVector) {
  bool success = true;
  for (size_t i = 0; i < contoursFileVector.size(); ++i) {
    std::string fileName = contoursFileVector[i];
    ReadTextFile src(fileName.c_str(),true);
    if (!src.good())
      return false;
 
    Contour New(src);
    if (New.isDefaultLoaded())
      success = false;

    push_back(New);
  }
  return success;
}

const char *ContourArray::GetName(size_type n) const {
  ASSERT(ValidId(n));
  return Get(n).Name();
}
