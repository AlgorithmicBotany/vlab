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



#ifndef __RECT_H__
#define __RECT_H__

#ifdef _WINDOWS
#include <windows.h>
#else
#include <qrect.h>
#endif

struct Rect {
#ifdef _WINDOWS
  Rect(const RECT &);
  Rect(int l, int t, int r, int b) : left(l), top(t), right(r), bottom(b) {}
#else
  Rect(const QRect &);
#endif
  Rect() : left(0), top(0), right(0), bottom(0) {}
  int left;
  int top;
  int right;
  int bottom;
};

struct Size {
  int cx;
  int cy;
};

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
