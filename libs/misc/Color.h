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



#ifndef __COLOR_H__
#define __COLOR_H__

#include <X11/Intrinsic.h>

class Color {

public:
  /* constructor */ Color();
  /* destructor */ ~Color();
  void set(const char *str);
  char *get(void);
  Pixel get_pixel(Widget w);

protected:
  bool conversion_done; // indicates whether conversion to pixel
                        // was done or not
  Widget widget;        // last widget
  Colormap cmap;        // last colormap
  XColor xcol;          // last xcolor

  char *txt;
};

#endif
