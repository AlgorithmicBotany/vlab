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
#include "xstring.h"
#include <Xm/XmAll.h>
#include <stdio.h>

#define SLIDER_SIZE 80

//######################################################################
// constructor

ColorChooserDialog::ColorChooserDialog(Widget parent, CC_callback user_callback,
                                       void *user_data) {
  this->user_callback = user_callback;
  this->user_data = user_data;

  // create a dialog shell
  dialog_shell = XtVaCreateWidget("x11ColorChooser", xmDialogShellWidgetClass,
                                  parent, XmNdeleteResponse, XmUNMAP, NULL);
  // create a form
  form = XtVaCreateWidget("form", xmFormWidgetClass, dialog_shell, NULL);

  // close button
  Widget close_button = XtVaCreateManagedWidget(
      "close_button", xmPushButtonWidgetClass, form, XmNleftAttachment,
      XmATTACH_POSITION, XmNleftPosition, 30, XmNrightAttachment,
      XmATTACH_POSITION, XmNrightPosition, 70, XmNtopAttachment, XmATTACH_NONE,
      XmNbottomAttachment, XmATTACH_FORM, NULL);
  XtAddCallback(close_button, XmNactivateCallback, close_cb, (XtPointer)this);
  // bottom separator
  Widget separator = XtVaCreateManagedWidget(
      "separator", xmSeparatorWidgetClass, form, XmNtopAttachment,
      XmATTACH_NONE, XmNleftAttachment, XmATTACH_FORM, XmNrightAttachment,
      XmATTACH_FORM, XmNbottomAttachment, XmATTACH_WIDGET, XmNbottomWidget,
      close_button, XmNorientation, XmHORIZONTAL, NULL);
  // form for the sliders
  Widget form2 = XtVaCreateWidget(
      "form2", xmFormWidgetClass, form, XmNleftAttachment, XmATTACH_NONE,
      XmNrightAttachment, XmATTACH_FORM, XmNtopAttachment, XmATTACH_FORM,
      XmNbottomAttachment, XmATTACH_WIDGET, XmNbottomWidget, separator, NULL);
  // red controls (slider & label)
  Widget red_label = XtVaCreateManagedWidget(
      "red_label", xmLabelWidgetClass, form2, XmNleftAttachment, XmATTACH_FORM,
      XmNtopAttachment, XmATTACH_POSITION, XmNtopPosition, 0,
      XmNrightAttachment, XmATTACH_NONE, XmNbottomAttachment, XmATTACH_POSITION,
      XmNbottomPosition, 33, XmNalignment, XmALIGNMENT_END, NULL);
  red_slider = XtVaCreateManagedWidget(
      "red_slider", xmScrollBarWidgetClass, form2, XmNleftAttachment,
      XmATTACH_WIDGET, XmNleftWidget, red_label, XmNtopAttachment,
      XmATTACH_OPPOSITE_WIDGET, XmNtopWidget, red_label, XmNbottomAttachment,
      XmATTACH_OPPOSITE_WIDGET, XmNbottomWidget, red_label, XmNrightAttachment,
      XmATTACH_NONE, XmNorientation, XmHORIZONTAL, XmNsliderSize, SLIDER_SIZE,
      XmNminimum, 0, XmNmaximum, 255 + SLIDER_SIZE, NULL);
  XtAddCallback(red_slider, XmNvalueChangedCallback, update_callback_cb,
                (XtPointer)this);
  XtAddCallback(red_slider, XmNdragCallback, update_callback_cb,
                (XtPointer)this);
  red_value_label = XtVaCreateManagedWidget(
      "red_value_label", xmLabelWidgetClass, form2, XmNleftAttachment,
      XmATTACH_WIDGET, XmNleftWidget, red_slider, XmNrightAttachment,
      XmATTACH_FORM, XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET, XmNtopWidget,
      red_slider, XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
      XmNbottomWidget, red_slider, XmNalignment, XmALIGNMENT_BEGINNING,
      XmNrecomputeSize, False, NULL);

  // green controls
  Widget green_label = XtVaCreateManagedWidget(
      "green_label", xmLabelWidgetClass, form2, XmNleftAttachment,
      XmATTACH_FORM, XmNtopAttachment, XmATTACH_POSITION, XmNtopPosition, 34,
      XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET, XmNrightWidget, red_label,
      XmNbottomAttachment, XmATTACH_POSITION, XmNbottomPosition, 66,
      XmNalignment, XmALIGNMENT_END, NULL);
  green_slider = XtVaCreateManagedWidget(
      "green_slider", xmScrollBarWidgetClass, form2, XmNleftAttachment,
      XmATTACH_WIDGET, XmNleftWidget, green_label, XmNtopAttachment,
      XmATTACH_OPPOSITE_WIDGET, XmNtopWidget, green_label, XmNbottomAttachment,
      XmATTACH_OPPOSITE_WIDGET, XmNbottomWidget, green_label,
      XmNrightAttachment, XmATTACH_NONE, XmNorientation, XmHORIZONTAL,
      XmNsliderSize, SLIDER_SIZE, XmNminimum, 0, XmNmaximum, 255 + SLIDER_SIZE,
      NULL);
  XtAddCallback(green_slider, XmNvalueChangedCallback, update_callback_cb,
                (XtPointer)this);
  XtAddCallback(green_slider, XmNdragCallback, update_callback_cb,
                (XtPointer)this);
  green_value_label = XtVaCreateManagedWidget(
      "green_value_label", xmLabelWidgetClass, form2, XmNleftAttachment,
      XmATTACH_WIDGET, XmNleftWidget, green_slider, XmNrightAttachment,
      XmATTACH_FORM, XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET, XmNtopWidget,
      green_slider, XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
      XmNbottomWidget, green_slider, XmNalignment, XmALIGNMENT_BEGINNING,
      XmNrecomputeSize, False, NULL);

  // blue controls
  Widget blue_label = XtVaCreateManagedWidget(
      "blue_label", xmLabelWidgetClass, form2, XmNleftAttachment, XmATTACH_FORM,
      XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET, XmNrightWidget, green_label,
      XmNtopAttachment, XmATTACH_POSITION, XmNtopPosition, 67,
      XmNbottomAttachment, XmATTACH_POSITION, XmNbottomPosition, 100,
      XmNalignment, XmALIGNMENT_END, NULL);
  blue_slider = XtVaCreateManagedWidget(
      "blue_slider", xmScrollBarWidgetClass, form2, XmNleftAttachment,
      XmATTACH_WIDGET, XmNleftWidget, blue_label, XmNrightAttachment,
      XmATTACH_NONE, XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET, XmNtopWidget,
      blue_label, XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
      XmNbottomWidget, blue_label, XmNorientation, XmHORIZONTAL, XmNsliderSize,
      SLIDER_SIZE, XmNminimum, 0, XmNmaximum, 255 + SLIDER_SIZE, NULL);
  XtAddCallback(blue_slider, XmNvalueChangedCallback, update_callback_cb,
                (XtPointer)this);
  XtAddCallback(blue_slider, XmNdragCallback, update_callback_cb,
                (XtPointer)this);
  blue_value_label = XtVaCreateManagedWidget(
      "blue_value_label", xmLabelWidgetClass, form2, XmNleftAttachment,
      XmATTACH_WIDGET, XmNleftWidget, blue_slider, XmNrightAttachment,
      XmATTACH_FORM, XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET, XmNtopWidget,
      blue_slider, XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
      XmNbottomWidget, blue_slider, XmNalignment, XmALIGNMENT_BEGINNING,
      XmNrecomputeSize, False, NULL);
  XtManageChild(form2);

  // create frame and a label inside it
  Widget frame = XtVaCreateManagedWidget(
      "frame", xmFrameWidgetClass, form, XmNleftAttachment, XmATTACH_FORM,
      XmNtopAttachment, XmATTACH_FORM, XmNbottomAttachment, XmATTACH_WIDGET,
      XmNbottomWidget, separator, XmNrightAttachment, XmATTACH_WIDGET,
      XmNrightWidget, form2, NULL);
  example_label =
      XtVaCreateManagedWidget("example_label", xmLabelWidgetClass, frame, NULL);

  // set the last_color_valid to false, so that we don't try to free that
  // pixel
  last_color_valid = False;

  update_labels();
}

//######################################################################
// - destructor - destroy the dialog widget

ColorChooserDialog::~ColorChooserDialog() {
  XtDestroyWidget(dialog_shell);
}

//######################################################################
//   manage()

void ColorChooserDialog::manage(void) {
  XtManageChild(form);
  XtMapWidget(dialog_shell);
}

//######################################################################
//   unmanage()

void ColorChooserDialog::unmanage(void) { XtUnmanageChild(form); }

//######################################################################
// change the color

void ColorChooserDialog::set_color(const char *color) {
  // find out the RGB values for this color
  Colormap cmap = DefaultColormapOfScreen(XtScreen(example_label));
  XColor col;
  if (!XParseColor(XtDisplay(example_label), cmap, color, &col)) {
    fprintf(stderr, "ColorChooserDialog::set_color(): color '%s' bad format.\n",
            color);
    return;
  }

  red = col.red >> 8;
  green = col.green >> 8;
  blue = col.blue >> 8;

  // now set the sliders
  XtVaSetValues(red_slider, XmNvalue, red, NULL);
  XtVaSetValues(green_slider, XmNvalue, green, NULL);
  XtVaSetValues(blue_slider, XmNvalue, blue, NULL);

  // and update the rest of components
  update_labels();
}

//######################################################################
// get the current color
//

char *ColorChooserDialog::get_color(void) {
  char buff[256];

  sprintf(buff, "#%lx%lx%lx%lx%lx%lx", (red / 16) % 16, (red) % 16,
          (green / 16) % 16, (green) % 16, (blue / 16) % 16, (blue) % 16);

  return xstrdup(buff);
}

//######################################################################
//
// get the current color (in RGB format)
//

void ColorChooserDialog::get_rgb_color(long &pred, long &pgreen, long &pblue) {
  pred = red;
  pgreen = green;
  pblue = blue;
}

//######################################################################
// callback for close_button
//

void ColorChooserDialog::close_cb(Widget, XtPointer ud, XtPointer) {
  ColorChooserDialog *cc = (ColorChooserDialog *)ud;
  XtUnmanageChild(cc->form);
}

//######################################################################
// color set callback

void ColorChooserDialog::update_callback_cb(Widget, XtPointer ud, XtPointer) {
  ColorChooserDialog *cc = (ColorChooserDialog *)ud;
  cc->update_labels();
  cc->user_callback(cc, cc->user_data);
}

//######################################################################
// update the interface (labels)
//
//     - read the current values from 3 scrollbars
//     - update the corresponding labels
//     - update the color of the label

void ColorChooserDialog::update_labels(void) {
  // extract values from sliders into rr, gg, bb
  int rr, gg, bb, t1, t2, t3;
  XmScrollBarGetValues(red_slider, &rr, &t1, &t2, &t3);
  XmScrollBarGetValues(green_slider, &gg, &t1, &t2, &t3);
  XmScrollBarGetValues(blue_slider, &bb, &t1, &t2, &t3);

  // store values into red, green, blue
  red = rr;
  green = gg;
  blue = bb;

  // update corresponding labels
  char buff[4096];
  XmString xmstr;
  sprintf(buff, "%ld", red);
  xmstr = XmStringCreateLocalized(buff);
  XtVaSetValues(red_value_label, XmNlabelString, xmstr, NULL);
  XmStringFree(xmstr);
  sprintf(buff, "%ld", green);
  xmstr = XmStringCreateLocalized(buff);
  XtVaSetValues(green_value_label, XmNlabelString, xmstr, NULL);
  XmStringFree(xmstr);
  sprintf(buff, "%ld", blue);
  xmstr = XmStringCreateLocalized(buff);
  XtVaSetValues(blue_value_label, XmNlabelString, xmstr, NULL);
  XmStringFree(xmstr);

  // update the test color:
  // ----------------------

  Colormap cmap = DefaultColormapOfScreen(XtScreen(example_label));

  // free the old color
  if (last_color_valid)
    XFreeColors(XtDisplay(example_label), cmap, &last_pixel, 1, 0);

  // allocate new entry for the new color
  XColor col;
  col.red = red << 8;
  col.green = green << 8;
  col.blue = blue << 8;
  if (!XAllocColor(XtDisplay(example_label), cmap, &col)) {
    // could not allocate an entry for this color :-(
    last_color_valid = False;
  } else {
    // entry succesfuly allocated - change the color of the label
    XmChangeColor(example_label, col.pixel);
    last_pixel = col.pixel;
    last_color_valid = True;
  }
}

//######################################################################
//
// set_title() -- will set the title of the dialog
//

void ColorChooserDialog::set_title(const char *title) {
  XtVaSetValues(dialog_shell, XmNtitle, title, NULL);
}
