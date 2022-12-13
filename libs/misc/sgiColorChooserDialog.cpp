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



#include "ColorChooserDialog.h"
#include "FString.h"
#include "xstring.h"
#include <Sgm/ColorC.h>
#include <Xm/XmAll.h>
#include <stdio.h>

// prototype of the undocumented function from Sgm library
#ifdef __cplusplus
extern "C" {
#endif
extern Widget SgColorChooserGetChild(Widget, unsigned char);
#ifdef __cplusplus
}
#endif

//######################################################################
// constructor

ColorChooserDialog::ColorChooserDialog(Widget parent, CC_callback user_callback,
                                       void *user_data) {
  this->user_callback = user_callback;
  this->user_data = user_data;

  // create the dialog
  color_chooser =
      SgCreateColorChooserDialog(parent, "sgiColorChooser", NULL, 0);

  XtUnmanageChild(SgColorChooserGetChild(color_chooser, XmDIALOG_APPLY_BUTTON));
  XtUnmanageChild(
      SgColorChooserGetChild(color_chooser, XmDIALOG_CANCEL_BUTTON));
  XtUnmanageChild(SgColorChooserGetChild(color_chooser, XmDIALOG_HELP_BUTTON));

  // update the 'close' button
  FString s("Hide");
  XtVaSetValues(color_chooser, XmNokLabelString, s.get_xmstring(), NULL);

  XtAddCallback(color_chooser, XmNdragCallback, color_set_cb, (XtPointer)this);
  XtAddCallback(color_chooser, XmNvalueChangedCallback, color_set_cb,
                (XtPointer)this);
  XtAddCallback(color_chooser, XmNokCallback, close_cb, (XtPointer)this);
}

//######################################################################
// - destructor - destroy the dialog widget

ColorChooserDialog::~ColorChooserDialog() {
  XtDestroyWidget(XtParent(color_chooser));
}

//######################################################################
//   manage()

void ColorChooserDialog::manage(void) {
  XtManageChild(color_chooser);
  XtMapWidget(XtParent(color_chooser));
}

//######################################################################
//   unmanage()

void ColorChooserDialog::unmanage(void) { XtUnmanageChild(color_chooser); }

//######################################################################
// change the color

void ColorChooserDialog::set_color(const char *color) {
  // find out the RGB values for this color
  Colormap cmap = DefaultColormapOfScreen(XtScreen(color_chooser));
  XColor col;
  if (!XParseColor(XtDisplay(color_chooser), cmap, color, &col)) {
    fprintf(stderr, "ColorChooserDialog::set_color(): color '%s' bad format.\n",
            color);
    return;
  }

  short red = col.red >> 8;
  short green = col.green >> 8;
  short blue = col.blue >> 8;

  SgColorChooserSetColor(color_chooser, red, green, blue);
}

//######################################################################
//
// get the current color (in RGB format)
//

void ColorChooserDialog::get_rgb_color(long &red, long &green, long &blue) {
  short rr, gg, bb;
  SgColorChooserGetColor(color_chooser, &rr, &gg, &bb);
  red = rr;
  green = gg;
  blue = bb;
}

//######################################################################
// get the current color
//

char *ColorChooserDialog::get_color(void) {
  short rr, gg, bb;
  SgColorChooserGetColor(color_chooser, &rr, &gg, &bb);

  char buff[256];

  sprintf(buff, "#%x%x%x%x%x%x", (rr / 16) % 16, (rr) % 16, (gg / 16) % 16,
          (gg) % 16, (bb / 16) % 16, (bb) % 16);
  return xstrdup(buff);
}

//######################################################################
// callback for close_button
//

void ColorChooserDialog::close_cb(Widget, XtPointer ud, XtPointer) {
  ColorChooserDialog *cc = (ColorChooserDialog *)ud;
  XtUnmanageChild(cc->color_chooser);
}

//######################################################################
// color set callback

void ColorChooserDialog::color_set_cb(Widget, XtPointer ud, XtPointer) {
  ColorChooserDialog *cc = (ColorChooserDialog *)ud;

  cc->user_callback(cc, cc->user_data);
}

//######################################################################
//
// set_title() -- will set the title of the dialog
//

void ColorChooserDialog::set_title(const char *title) {
  FString s(title);
  XtVaSetValues(color_chooser, XmNdialogTitle, s.get_xmstring(), NULL);
}
