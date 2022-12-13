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



#ifndef __SCREENPOINT_H__
#define __SCREENPOINT_H__

class ScreenPoint {
public:
  ScreenPoint(int x = 0, int y = 0) : _x(x), _y(y) {}
  int x() const { return _x; }
  int y() const { return _y; }
  void Set(int x, int y) {
    _x = x;
    _y = y;
  }
  ScreenPoint operator-(ScreenPoint r) {
    ScreenPoint res(_x - r.x(), _y - r.y());
    return res;
  }

private:
  int _x;
  int _y;
};

#else
#error File already included
#endif
