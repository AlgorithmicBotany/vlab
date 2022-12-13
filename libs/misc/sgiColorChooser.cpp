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



#include "sgiColorChooser.h"

// prototypes
void close_cb(Widget, XtPointer, XtPointer);
void color_set_cb(Widget, XtPointer, XtPointer);
void default_rgb_changed_cb(ColorChooser *, XtPointer);
void default_close_cb(ColorChooser *, XtPointer);

// prototype of the undocumented function from Sgm library
#ifdef __cplusplus
extern "C" {
#endif
extern Widget SgColorChooserGetChild(Widget, unsigned char);
#ifdef __cplusplus
}
#endif

//----------------------------------------------------------------------------
//
// constructor
//

ColorChooser::ColorChooser(Widget p, int r, int g, int b) {
  parent = p;

  color_chooser = SgCreateColorChooserDialog(parent, "colorChooser", NULL, 0);

  XtUnmanageChild(SgColorChooserGetChild(color_chooser, XmDIALOG_APPLY_BUTTON));
  XtUnmanageChild(
      SgColorChooserGetChild(color_chooser, XmDIALOG_CANCEL_BUTTON));
  XtUnmanageChild(SgColorChooserGetChild(color_chooser, XmDIALOG_HELP_BUTTON));

  XmString str = XmStringCreateLocalized("Close");
  XtVaSetValues(color_chooser, XmNokLabelString, str, NULL);
  XmStringFree(str);

  XtAddCallback(color_chooser, XmNdragCallback, color_set_cb, (XtPointer)this);
  XtAddCallback(color_chooser, XmNvalueChangedCallback, color_set_cb,
                (XtPointer)this);
  XtAddCallback(color_chooser, XmNokCallback, close_cb, (XtPointer)this);

  // reset the widgets
  setRGB(r, g, b, False);
  // set the default callbacks
  close_callback = default_close_cb;
  rgb_changed_callback = default_rgb_changed_cb;
}

//----------------------------------------------------------------------------
//
// manages the color chooser
//

void ColorChooser::manage(void) { XtManageChild(color_chooser); }

//----------------------------------------------------------------------------
//
// un-manages the color chooser
//

void ColorChooser::unmanage(void) { XtUnmanageChild(color_chooser); }

//----------------------------------------------------------------------------
//
// change the color
//

void ColorChooser::setRGB(int r, int g, int b, Boolean notify) {
  red = r;
  green = g;
  blue = b;

  SgColorChooserSetColor(color_chooser, red, green, blue);

  if (notify)
    (*rgb_changed_callback)(this, rgb_changed_callback_ud);
}

//----------------------------------------------------------------------------
//
// get the rgb values
//

void ColorChooser::getRGB(int &r, int &g, int &b) {
  short rr, gg, bb;

  SgColorChooserGetColor(color_chooser, &rr, &gg, &bb);

  r = rr;
  g = gg;
  b = bb;
}

//----------------------------------------------------------------------------
//
// set the callback for the close button
//

void ColorChooser::close_button_set_callback(CC_callback cb, XtPointer ud) {
  close_callback = cb;
  close_callback_ud = ud;
}

//----------------------------------------------------------------------------
//
// set the callback rgb changed
//

void ColorChooser::rgb_changed_set_callback(CC_callback cb, XtPointer ud) {
  rgb_changed_callback = cb;
  rgb_changed_callback_ud = ud;
}

//----------------------------------------------------------------------------
//
// set the title of the color chooser dialog box
//

void ColorChooser::set_title(String str) {
  XmString mstr = XmStringCreateLocalized(str);
  XtVaSetValues(color_chooser, XmNdialogTitle, mstr, NULL);
  XmStringFree(mstr);
  //    XtVaSetValues( color_chooser, XmNtitle, str, NULL);
}

//----------------------------------------------------------------------------
//
// default callback for rgb_changed - do nothing
//

void default_rgb_changed_cb(ColorChooser *, XtPointer) { return; }

//----------------------------------------------------------------------------
//
// default callback for close - unmanage the chooser
//

void default_close_cb(ColorChooser *cc, XtPointer) { cc->unmanage(); }

//----------------------------------------------------------------------------
//
// callback for close_button
//

void close_cb(Widget, XtPointer ud, XtPointer) {
  ColorChooser *cc = (ColorChooser *)ud;

  (*(cc->close_callback))(cc, cc->close_callback_ud);
}

//----------------------------------------------------------------------------
//
// color set callback
//

void color_set_cb(Widget w, XtPointer ud, XtPointer) {
  ColorChooser *cc = (ColorChooser *)ud;
  short rr, gg, bb;

  SgColorChooserGetColor(w, &rr, &gg, &bb);

  cc->red = rr;
  cc->green = gg;
  cc->blue = bb;

  // call the user defined callback
  (*(cc->rgb_changed_callback))(cc, cc->rgb_changed_callback_ud);
}
