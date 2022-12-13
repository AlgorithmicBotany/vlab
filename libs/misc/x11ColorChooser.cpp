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



#include "x11ColorChooser.h"

// prototypes
void red_slider_cb(Widget, XtPointer, XtPointer);
void green_slider_cb(Widget, XtPointer, XtPointer);
void blue_slider_cb(Widget, XtPointer, XtPointer);
void close_button_cb(Widget, XtPointer, XtPointer);
void default_rgb_changed_cb(ColorChooser *, XtPointer);
void default_close_cb(ColorChooser *, XtPointer);

//----------------------------------------------------------------------------
//
// constructor
//

ColorChooser::ColorChooser(Widget p, int r, int g, int b) {
  parent = p;
  shell = XtVaCreateWidget("colorChooser", xmDialogShellWidgetClass, parent,
                           XmNdeleteResponse, XmDO_NOTHING, NULL);

  XmString str;

  form = XtVaCreateWidget("form", xmFormWidgetClass, shell, NULL);

  frame = XtVaCreateManagedWidget(
      "frame", xmFrameWidgetClass, form, XmNleftAttachment, XmATTACH_FORM,
      XmNleftOffset, 15, XmNrightAttachment, XmATTACH_FORM, XmNrightOffset, 15,
      XmNtopAttachment, XmATTACH_FORM, XmNtopOffset, 15, XmNbottomAttachment,
      XmATTACH_NONE, NULL);

  str = XmStringCreateLocalized("test color");
  example_label =
      XtVaCreateManagedWidget("example_label", xmLabelWidgetClass, frame,
                              XmNlabelString, str, XmNheight, 50, NULL);
  XmStringFree(str),

      separator1 = XtVaCreateManagedWidget(
          "separator1", xmSeparatorWidgetClass, form, XmNleftAttachment,
          XmATTACH_FORM, XmNrightAttachment, XmATTACH_FORM, XmNtopAttachment,
          XmATTACH_WIDGET, XmNtopWidget, example_label, XmNtopOffset, 15,
          XmNbottomAttachment, XmATTACH_NONE, NULL);

  // red controls

  str = XmStringCreateLocalized("Red");
  red_label = XtVaCreateManagedWidget(
      "red_label", xmLabelWidgetClass, form, XmNleftAttachment, XmATTACH_FORM,
      XmNleftOffset, 0, XmNtopAttachment, XmATTACH_WIDGET, XmNtopWidget,
      separator1, XmNtopOffset, 15, XmNrightAttachment, XmATTACH_NONE,
      XmNbottomAttachment, XmATTACH_NONE, XmNlabelString, str, XmNwidth, 100,
      XmNalignment, XmALIGNMENT_END, NULL);
  XmStringFree(str);

  red_slider = XtVaCreateManagedWidget(
      "red_slider", xmScrollBarWidgetClass, form, XmNleftAttachment,
      XmATTACH_WIDGET, XmNleftWidget, red_label, XmNtopAttachment,
      XmATTACH_OPPOSITE_WIDGET, XmNtopWidget, red_label, XmNbottomAttachment,
      XmATTACH_OPPOSITE_WIDGET, XmNbottomWidget, red_label, XmNrightAttachment,
      XmATTACH_NONE, XmNorientation, XmHORIZONTAL, XmNminimum, 0, XmNmaximum,
      256, XmNwidth, 200, NULL);
  XtAddCallback(red_slider, XmNvalueChangedCallback, red_slider_cb,
                (XtPointer)this);
  XtAddCallback(red_slider, XmNdragCallback, red_slider_cb, (XtPointer)this);

  red_value_label = XtVaCreateManagedWidget(
      "red_value_label", xmLabelWidgetClass, form, XmNleftAttachment,
      XmATTACH_WIDGET, XmNleftWidget, red_slider, XmNtopAttachment,
      XmATTACH_OPPOSITE_WIDGET, XmNtopWidget, red_slider, XmNbottomAttachment,
      XmATTACH_OPPOSITE_WIDGET, XmNbottomWidget, red_slider, XmNrightAttachment,
      XmATTACH_FORM, XmNrightOffset, 15, XmNalignment, XmALIGNMENT_BEGINNING,
      NULL);

  // green controls

  str = XmStringCreateLocalized("Green");
  green_label = XtVaCreateManagedWidget(
      "green_label", xmLabelWidgetClass, form, XmNleftAttachment, XmATTACH_FORM,
      XmNtopAttachment, XmATTACH_WIDGET, XmNtopWidget, red_label, XmNtopOffset,
      15, XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET, XmNrightWidget,
      red_label, XmNbottomAttachment, XmATTACH_NONE, XmNlabelString, str,
      XmNalignment, XmALIGNMENT_END, NULL);
  XmStringFree(str);

  green_slider = XtVaCreateManagedWidget(
      "green_slider", xmScrollBarWidgetClass, form, XmNleftAttachment,
      XmATTACH_WIDGET, XmNleftWidget, green_label, XmNtopAttachment,
      XmATTACH_OPPOSITE_WIDGET, XmNtopWidget, green_label, XmNbottomAttachment,
      XmATTACH_OPPOSITE_WIDGET, XmNbottomWidget, green_label,
      XmNrightAttachment, XmATTACH_NONE, XmNorientation, XmHORIZONTAL,
      XmNminimum, 0, XmNmaximum, 256, XmNwidth, 200, NULL);
  XtAddCallback(green_slider, XmNvalueChangedCallback, green_slider_cb,
                (XtPointer)this);
  XtAddCallback(green_slider, XmNdragCallback, green_slider_cb,
                (XtPointer)this);

  green_value_label = XtVaCreateManagedWidget(
      "green_value_label", xmLabelWidgetClass, form, XmNleftAttachment,
      XmATTACH_WIDGET, XmNleftWidget, green_slider, XmNtopAttachment,
      XmATTACH_OPPOSITE_WIDGET, XmNtopWidget, green_slider, XmNbottomAttachment,
      XmATTACH_OPPOSITE_WIDGET, XmNbottomWidget, green_slider,
      XmNrightAttachment, XmATTACH_FORM, XmNrightOffset, 15, XmNalignment,
      XmALIGNMENT_BEGINNING, NULL);

  // blue controls

  str = XmStringCreateLocalized("Blue");
  blue_label = XtVaCreateManagedWidget(
      "blue_label", xmLabelWidgetClass, form, XmNleftAttachment, XmATTACH_FORM,
      XmNtopAttachment, XmATTACH_WIDGET, XmNtopWidget, green_label,
      XmNtopOffset, 15, XmNrightAttachment, XmATTACH_NONE, XmNlabelString, str,
      XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET, XmNrightWidget, green_label,
      XmNalignment, XmALIGNMENT_END, XmNbottomAttachment, XmATTACH_NONE,
      XmNbottomOffset, 15, NULL);
  XmStringFree(str);

  blue_slider = XtVaCreateManagedWidget(
      "blue_slider", xmScrollBarWidgetClass, form, XmNleftAttachment,
      XmATTACH_WIDGET, XmNleftWidget, blue_label, XmNtopAttachment,
      XmATTACH_OPPOSITE_WIDGET, XmNtopWidget, blue_label, XmNbottomAttachment,
      XmATTACH_OPPOSITE_WIDGET, XmNbottomWidget, blue_label, XmNrightAttachment,
      XmATTACH_NONE, XmNorientation, XmHORIZONTAL, XmNminimum, 0, XmNmaximum,
      256, XmNwidth, 200, NULL);
  XtAddCallback(blue_slider, XmNvalueChangedCallback, blue_slider_cb,
                (XtPointer)this);
  XtAddCallback(blue_slider, XmNdragCallback, blue_slider_cb, (XtPointer)this);

  blue_value_label = XtVaCreateManagedWidget(
      "blue_value_label", xmLabelWidgetClass, form, XmNleftAttachment,
      XmATTACH_WIDGET, XmNleftWidget, blue_slider, XmNtopAttachment,
      XmATTACH_OPPOSITE_WIDGET, XmNtopWidget, blue_slider, XmNbottomAttachment,
      XmATTACH_OPPOSITE_WIDGET, XmNbottomWidget, blue_slider,
      XmNrightAttachment, XmATTACH_FORM, XmNrightOffset, 15, XmNalignment,
      XmALIGNMENT_BEGINNING, NULL);

  // bottom separator
  separator2 = XtVaCreateManagedWidget(
      "separator2", xmSeparatorWidgetClass, form, XmNleftAttachment,
      XmATTACH_FORM, XmNrightAttachment, XmATTACH_FORM, XmNtopAttachment,
      XmATTACH_WIDGET, XmNtopWidget, blue_label, XmNtopOffset, 15,
      XmNbottomAttachment, XmATTACH_NONE, NULL);

  // close button
  str = XmStringCreateLocalized("Close");
  close_button = XtVaCreateManagedWidget(
      "close_button", xmPushButtonWidgetClass, form, XmNlabelString, str,
      XmNleftAttachment, XmATTACH_POSITION, XmNleftPosition, 30,
      XmNrightAttachment, XmATTACH_POSITION, XmNrightPosition, 70,
      XmNtopAttachment, XmATTACH_WIDGET, XmNtopWidget, separator2, XmNtopOffset,
      15, XmNbottomAttachment, XmATTACH_FORM, XmNbottomOffset, 15, NULL);
  XmStringFree(str);
  XtRemoveAllCallbacks(close_button, XmNactivateCallback);
  XtAddCallback(close_button, XmNactivateCallback, close_button_cb,
                (XtPointer)this);

  // set the last_color_valid to false, so that we don't try to free that
  // pixel
  last_color_valid = False;
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

void ColorChooser::manage(void) { XtManageChild(form); }

//----------------------------------------------------------------------------
//
// un-manages the color chooser
//

void ColorChooser::unmanage(void) { XtUnmanageChild(form); }

//----------------------------------------------------------------------------
//
// change the color
//

void ColorChooser::setRGB(int r, int g, int b, Boolean notify) {
  red = r;
  green = g;
  blue = b;

  XmString str;
  char buff[4096];

  sprintf(buff, "%d", red);
  str = XmStringCreateLocalized(buff);
  XtVaSetValues(red_value_label, XmNlabelString, str, NULL);
  XmStringFree(str);

  sprintf(buff, "%d", green);
  str = XmStringCreateLocalized(buff);
  XtVaSetValues(green_value_label, XmNlabelString, str, NULL);
  XmStringFree(str);

  sprintf(buff, "%d", blue);
  str = XmStringCreateLocalized(buff);
  XtVaSetValues(blue_value_label, XmNlabelString, str, NULL);
  XmStringFree(str);

  XmScrollBarSetValues(red_slider, red, 1, 1, 10, False);
  XmScrollBarSetValues(green_slider, green, 1, 1, 10, False);
  XmScrollBarSetValues(blue_slider, blue, 1, 1, 10, False);

  update_test_color();

  if (notify)
    (*rgb_changed_callback)(this, rgb_changed_callback_ud);
}

//----------------------------------------------------------------------------
//
// get the rgb values
//

void ColorChooser::getRGB(int &r, int &g, int &b) {
  r = red;
  g = green;
  b = blue;
}

//----------------------------------------------------------------------------
//
// update the color of the test label
//

void ColorChooser::update_test_color(void) {
  Colormap cmap;
  XColor col;

  cmap = DefaultColormapOfScreen(XtScreen(example_label));

  // free the old color
  if (last_color_valid) {
    XFreeColors(XtDisplay(example_label), cmap, &last_pixel, 1, 0);
  }

  // allocate new entry
  col.red = red << 8;
  col.green = green << 8;
  col.blue = blue << 8;

  if (!XAllocColor(XtDisplay(example_label), cmap, &col)) {
    last_color_valid = False;
  } else {
    XmChangeColor(example_label, col.pixel);
    last_pixel = col.pixel;
    last_color_valid = True;
  }
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
   XtVaSetValues(shell, XmNtitle, str, NULL);
}

//----------------------------------------------------------------------------
//
// callbacks for the sliders
//

void red_slider_cb(Widget, XtPointer ud, XtPointer cd) {
  int value = ((XmScrollBarCallbackStruct *)cd)->value;
  ColorChooser *cc = (ColorChooser *)ud;

  cc->red = value;
  char buff[4096];
  sprintf(buff, "%d", value);
  XmString str = XmStringCreateLocalized(buff);
  XtVaSetValues(cc->red_value_label, XmNlabelString, str, NULL);
  XmStringFree(str);
  cc->update_test_color();
  // call the user callback with new values
  (*(cc->rgb_changed_callback))(cc, cc->rgb_changed_callback_ud);
}

void green_slider_cb(Widget, XtPointer ud, XtPointer cd) {
  int value = ((XmScrollBarCallbackStruct *)cd)->value;
  ColorChooser *cc = (ColorChooser *)ud;

  cc->green = value;
  char buff[4096];
  sprintf(buff, "%d", value);
  XmString str = XmStringCreateLocalized(buff);
  XtVaSetValues(cc->green_value_label, XmNlabelString, str, NULL);
  XmStringFree(str);
  cc->update_test_color();
  // call the user callback with new values
  (*(cc->rgb_changed_callback))(cc, cc->rgb_changed_callback_ud);
}

void blue_slider_cb(Widget, XtPointer ud, XtPointer cd) {
  int value = ((XmScrollBarCallbackStruct *)cd)->value;
  ColorChooser *cc = (ColorChooser *)ud;

  cc->blue = value;
  char buff[4096];
  sprintf(buff, "%d", value);
  XmString str = XmStringCreateLocalized(buff);
  XtVaSetValues(cc->blue_value_label, XmNlabelString, str, NULL);
  XmStringFree(str);
  cc->update_test_color();
  // call the user callback with new values
  (*(cc->rgb_changed_callback))(cc, cc->rgb_changed_callback_ud);
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

void close_button_cb(Widget, XtPointer ud, XtPointer) {
  ColorChooser *cc = (ColorChooser *)ud;

  (*(cc->close_callback))(cc, cc->close_callback_ud);
}
