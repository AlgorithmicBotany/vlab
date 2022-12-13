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



#ifndef __FONT_CHOOSER_H__
#define __FONT_CHOOSER_H__

#include <X11/Intrinsic.h>

Widget FedCreateFontChooserDialog(
    Widget parent, char *name, ArgList arglist, Cardinal argcount,
    XtCallbackProc okButtonCB, XtPointer okData, XtCallbackProc applyButtonCB,
    XtPointer applyData, XtCallbackProc cancelButtonCB, XtPointer cancelData,
    XtCallbackProc helpButtonCB, XtPointer helpData);
char *FedFontChooserGetFontName(Widget w);
void FedFontChooserSetFontName(Widget w, char *fontName);

#endif
