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



#ifndef _SGI_VISUAL_H_
#define _SGI_VISUAL_H_

#include <X11/Intrinsic.h>

/* STATUS RETURNS */
#define SG_VISUAL_SUCCESS 1
#define SG_VISUAL_DEFAULT (SG_VISUAL_SUCCESS + 1)

#define SG_NO_VISUAL (-1)
#define SG_BAD_DISPLAY (SG_NO_VISUAL - 1)
#define SG_NO_TYPE_AND_CLASS (SG_BAD_DISPLAY - 1)
#define SG_NO_SUCH_VISUAL (SG_NO_TYPE_AND_CLASS - 1)

/* Visual types */
#define SG_DEFAULT_PLANES 0 /* matches resource default */
#define SG_UNDERLAY_PLANES 1
#define SG_NORMAL_PLANES 2
#define SG_OVERLAY_PLANES 3
#define SG_POPUP_PLANES 4
#define SG_MAX_TYPES (SG_POPUP_PLANES + 1)

/* External declarations */

int SG_defaultDepthAndTypeResources(Display *display, int screen,
                                    int *requestedC_Class, char *requestedType,
                                    int *requestedTypeV, int *requestedDepth,
                                    Visual **requestedVisual,
                                    Colormap *requestedColormap,
                                    Drawable *requestedDrawable);
Colormap SG_getDefaultColormap(Display *dpy, int scr, Visual *vsl);
int SG_getDefaultDepth(Display *dpy, int scr, int *c_class, int type);
XVisualInfo *SG_getMatchingVisual(Display *dpy, int scr, VisualID vsl,
                                  int *c_class, int type, int depth);
int SG_getMaxDepth(Display *dpy, int scr, int *c_class, int type);
int SG_getNormalArgs(Display *dpy, int scr, ArgList args, int *n);
int SG_getOverlayArgs(Display *dpy, int scr, ArgList args, int *n);
int SG_getOverlay2Args(Display *dpy, int scr, ArgList args, int *n);
int SG_getOverlay4Args(Display *dpy, int scr, ArgList args, int *n);
int SG_getPopupArgs(Display *dpy, int scr, ArgList args, int *n);
int SG_getUnderlayArgs(Display *dpy, int scr, ArgList args, int *n);
int SG_getVisualArgs(Display *dpy, int scr, int dpth, int *c_class, int type,
                     ArgList args, int *n);

#endif
