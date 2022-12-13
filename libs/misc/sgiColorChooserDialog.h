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



#ifndef __COLOR_CHOOSER_DIALOG__H__
#define __COLOR_CHOOSER_DIALOG__H__

#include <X11/Intrinsic.h>

class ColorChooserDialog;

typedef void (*CC_callback)(ColorChooserDialog *, void *);

class ColorChooserDialog {

public:
  /* constructor */ ColorChooserDialog(Widget, CC_callback, void *);
  /* destructor */ ~ColorChooserDialog();
  void set_color(const char *);
  void manage(void);
  void unmanage(void);
  char *get_color(void);
  void get_rgb_color(long &red, long &green, long &blue);
  void set_title(const char *);

private:
  Widget color_chooser;

  static void color_set_cb(Widget, XtPointer, XtPointer);
  static void close_cb(Widget, XtPointer, XtPointer);

  // user callback information
  CC_callback user_callback;
  void *user_data;
};

#endif
