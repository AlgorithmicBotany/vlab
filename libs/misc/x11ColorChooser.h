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

#include <X11/Intrinsic.h>
#include <Xm/XmAll.h>

class ColorChooser;

typedef void (*CC_callback)(ColorChooser *, XtPointer);

class ColorChooser {
private:
  int red, green, blue;
  Widget shell;
  Widget parent;
  Widget red_slider;
  Widget green_slider;
  Widget blue_slider;
  Widget red_label;
  Widget green_label;
  Widget blue_label;
  Widget red_value_label;
  Widget green_value_label;
  Widget blue_value_label;
  Widget form;
  Widget separator1;
  Widget separator2;
  Widget example_label;
  Widget frame;
  Widget close_button;

  // function to be called then the 'close button' is pushed
  CC_callback close_callback;
  XtPointer close_callback_ud;
  // function to be called when the rgb value changes
  CC_callback rgb_changed_callback;
  XtPointer rgb_changed_callback_ud;

  Boolean last_color_valid;
  Pixel last_pixel;

  // change the color of the test label
  void update_test_color(void);

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

  friend void red_slider_cb(Widget, XtPointer, XtPointer);
  friend void green_slider_cb(Widget, XtPointer, XtPointer);
  friend void blue_slider_cb(Widget, XtPointer, XtPointer);
  friend void close_button_cb(Widget, XtPointer, XtPointer);
};

#endif
