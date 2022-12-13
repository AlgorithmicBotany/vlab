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



#ifndef __COLOR_CHOOSER__H__
#define __COLOR_CHOOSER__H__

#include <Sgm/ColorC.h>
#include <X11/Intrinsic.h>
#include <Xm/XmAll.h>

class ColorChooser;

typedef void (*CC_callback)(ColorChooser *, XtPointer);

class ColorChooser {
private:
  int red, green, blue;
  Widget color_chooser;
  Widget parent;

  // function to be called then the 'close button' is pushed
  CC_callback close_callback;
  XtPointer close_callback_ud;
  // function to be called when the rgb value changes
  CC_callback rgb_changed_callback;
  XtPointer rgb_changed_callback_ud;

public:
  // constructor
  ColorChooser(Widget parent, int r = 0, int g = 0, int b = 0);
  // destructor
  ~ColorChooser();
  // sets the callback for close button
  void close_button_set_callback(CC_callback, XtPointer p = NULL);
  // sets the callback for the rgb_changed
  void rgb_changed_set_callback(CC_callback, XtPointer p = NULL);
  // get the current values of r, g, b
  void getRGB(int &r, int &g, int &b);
  // set the values of rgb
  void setRGB(int r, int g, int b, Boolean notify = False);
  // manage the color chooser
  void manage(void);
  // unmanage the color chooser (hide)
  void unmanage(void);
  // set the title of the color chooser
  void set_title(String str);

  friend void close_cb(Widget, XtPointer, XtPointer);
  friend void color_set_cb(Widget, XtPointer, XtPointer);
};

#endif
