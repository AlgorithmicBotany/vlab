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



#ifndef __UVPRECISION_H__
#define __UVPRECISION_H__

class UVPrecision {
public:
  enum eParams {
    eUnspecified = -1,
    eUDivDefault = 10,
    eVDivDefault = 10,
    eUDivMin = 2,
    eUDivMax = 64,
    eVDivMin = 2,
    eVDivMax = 64
  };

  UVPrecision() { Default(); }
  void Default() { _u = _v = eUnspecified; }
  void SetU(int u) { _u = u; }
  void SetV(int v) { _v = v; }
  bool IsUSpecified() const { return _u != eUnspecified; }
  int U() const { return _u; }
  bool IsVSpecified() const { return _v != eUnspecified; }
  int V() const { return _v; }
  bool IsValidU(int u) const {
    if (u < eUDivMin)
      return false;
    if (u > eUDivMax)
      return false;
    return true;
  }

  bool IsValidV(int v) const {
    if (v < eVDivMin)
      return false;
    if (v > eVDivMax)
      return false;
    return true;
  }

private:
  int _u, _v;
};

#endif
