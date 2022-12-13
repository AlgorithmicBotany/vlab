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



/*
 * This procedure will ensure that, if a dialog window is being mapped,
 * its contents become visible before returning.  It is intended to be
 * used just before a bout of computing that doesn't service the display.
 * You should still call XmUpdateDisplay() at intervals during this
 * computing if possible.
 *
 * The monitoring of window states is necessary because attempts to map
 * the dialog are redirected to the window manager (if there is one) and
 * this introduces a significant delay before the window is actually mapped
 * and exposed.  This code works under mwm, twm, uwm, and no-wm.  It
 * doesn't work (but doesn't hang) with olwm if the mainwindow is iconified.
 *
 * The argument to ForceDialog is any widget in the dialog (often it
 * will be the BulletinBoard child of a DialogShell).
 */

#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <stdio.h>

#include "forceDialog.h"

void forceDialog(Widget w) {
  Widget diashell, topshell;
  Window diawindow, topwindow;
  Display *dpy;
  XWindowAttributes xwa;
  XEvent event;
  XtAppContext cxt;

  /* Locate the shell we are interested in.  In a particular instance, you
   * may know these shells already.
   */

  for (diashell = w; !XtIsShell(diashell); diashell = XtParent(diashell))
    ;

  /* Locate its primary window's shell (which may be the same) */

  for (topshell = diashell; !XtIsTopLevelShell(topshell);
       topshell = XtParent(topshell))
    ;

  if (XtIsRealized(diashell) && XtIsRealized(topshell)) {
    dpy = XtDisplay(topshell);
    diawindow = XtWindow(diashell);
    topwindow = XtWindow(topshell);
    cxt = XtWidgetToApplicationContext(diashell);

    /* Wait for the dialog to be mapped.
     * It's guaranteed to become so unless... */

    while (XGetWindowAttributes(dpy, diawindow, &xwa),
           xwa.map_state != IsViewable) {

      /* ...if the primary is (or becomes) unviewable or unmapped, it's
       * probably iconified, and nothing will happen. */

      if (XGetWindowAttributes(dpy, topwindow, &xwa),
          xwa.map_state != IsViewable)
        break;

      /* At this stage, we are guaranteed there will be an event of
       * some kind. Beware; we are presumably in a callback,
       * so this can recurse. */

      XtAppNextEvent(cxt, &event);
      XtDispatchEvent(&event);
    }
  }

  /* The next XSync() will get an expose event if the dialog was unmapped. */

  XmUpdateDisplay(topshell);
}
