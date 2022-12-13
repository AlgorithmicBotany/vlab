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



#include "Color.h"
#include "xmemory.h"
#include <stdio.h>

//######################################################################
//
// constructor
//

Color::Color() {
  txt = NULL;
  conversion_done = false;
}

//######################################################################
//
// destructor
//

Color::~Color() {
  // free the textual representation
  xfree(txt);

  // free the converted pixel
  if (conversion_done)
    XFreeColors(XtDisplay(widget), cmap, &xcol.pixel, 1, 0);
}

//######################################################################
//
// sets a new color
//
//  if a conversion to 'pixel' has been done on the last color, the
//  pixel is de-allocated from the colormap
//

void Color::set(const char *str) {
  xfree(txt);
  txt = strdup(str);

  // if conversion was already done, free the last pixel
  if (conversion_done) {
    XFreeColors(XtDisplay(widget), cmap, &xcol.pixel, 1, 0);

    // no conversion done on this color
    conversion_done = false;
  }
}

//######################################################################
//
// get the textual value of the color
//

char *Color::get(void) { return txt; }

//######################################################################
//
// allocate a pixel associated with this color, and return it
//
//   - it will be dealocated either when a new color is set,
//     or in a destructor
//

Pixel Color::get_pixel(Widget w) {
  // first find out whether we have already done this, and if so,
  // just return the old pixel
  if (conversion_done)
    return xcol.pixel;

  // do the conversion:
  // ------------------
  widget = w;

  // get the default colormap
  cmap = DefaultColormapOfScreen(XtScreen(widget));

  // figure out RGB value for this color
  if (!XParseColor(XtDisplay(widget), cmap, txt, &xcol)) {
    fprintf(stderr, "Color::get_pixel(): color '%s' bad format.\n", txt);
    return 0;
  }

  // allocate a pixel
  if (!XAllocColor(XtDisplay(widget), cmap, &xcol)) {
    // could not allocate an entry for this color :-(
    fprintf(stderr, "Color::get_pixel(): could not allocate color '%s'.\n",
            txt);
    return 0;
  }

  conversion_done = true;
  return xcol.pixel;
}
