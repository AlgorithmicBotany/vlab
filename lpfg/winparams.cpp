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



#include <sstream>
#include "winparams.h"

WindowParams::WindowParams() {
  _left = 0.0f;
  _top = 0.0f;
  _width = 1.0f;
  _height = 1.0f;
  _modifiers.set = false;
}

bool WindowParams::Read(const char *ln) {
  std::stringstream str(ln);
  str >> _name >> _left >> _top >> _width >> _height;
  if (str.bad())
    return false;
  return true;
}

WindowBorderParams::WindowBorderParams() {
  _width = 1;
  _red = 0;
  _green = 0;
  _blue = 0;
}

bool WindowBorderParams::Read(const char *ln) {
  std::stringstream str(ln);
  str >> _width >> _red >> _green >> _blue;
  if (str.bad())
    return false;
  return true;
}
