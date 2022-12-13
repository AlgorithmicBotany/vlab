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



#include "overlays.h"
#include "config.h"

#ifdef USE_SGI_OVERLAYS

#include "sgi_intl.c"
#include "sgi_intl.h"
#include "sgi_visual.c"
#include "sgi_visual.h"

int get_overlay_args(Display *dpy, int scr, ArgList args, int *n) {
#ifdef MENUS_IN_OVERLAY
#ifdef OVERLAY_2BIT
  int status = SG_getOverlay2Args(dpy, scr, args, n);
#else
  int status = SG_getOverlay4Args(dpy, scr, args, n);
#endif
  if (status < 0) {
    fprintf(stderr, "overlays problem: returned code = %d.\n", status);
    n = 0;
  }
  return status;
#else
  return 0;
#endif
}

#else

int get_overlay_args(Display *dpy, int scr, ArgList args, int *n) { return 0; }

#endif
