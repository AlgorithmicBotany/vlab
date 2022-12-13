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



#ifndef __COMLINEPARAM_H__
#define __COMLINEPARAM_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
#include <stdlib.h>
#endif

#ifdef LINUX
#include <limits.h>
#endif

#include "maxpth.h"

typedef enum {
  SAVE_RGB,
  SAVE_PNG,
  SAVE_BMP,
  SAVE_GIF,
  SAVE_JPG,
  SAVE_PBM,
  SAVE_TIFF,
  SAVE_RAS,
  SAVE_TGA,
  SAVE_RLE,
  SAVE_RAYSHADE,
  SAVE_POSTSCRIPT,
  SAVE_STRING,
  SAVE_VIEWVOLUME,
  SAVE_STRINGDUMP,
  SAVE_GLS,
  SAVE_OBJ,
  SAVE_COUNT
} eFileFormats;

typedef enum { CONTINUOUS, TRIGGERED, OFF } SavingMode;

#define MAXCONTOURS 32 /* maximal number of different contours */
/* for storing filenames and id's of contour. They have to be processed after
 the viewfile is read because the parameter cylinder_sides may change */

#ifdef WIN32
#define MaxCntrFname _MAX_FNAME + 1
#else
#define MaxCntrFname PATH_MAX
#endif

#define NUMTEXTURES 20 /* maximum number of different textures */

/* MATERIALS and COLORMAPS */
#define MAXCOLORMAPS 12
#define MAXINCOLORMAP 256

struct COMLINEPARAM {
  char *programname;
  char *lfilename;
  char *vfilename;
  char *afilename;


#ifdef CPFG_ENVIRONMENT
  char *efilename;
  char communication; /* NONE, LOCAL (single cpfg) or DISTRIBUTED
                         when cpfg acts as aserver of drawing program */
#endif
#if CPFG_VERSION >= 3200
  int server_socket; /* socket number where text commands are expected*/
#endif
  char *titlename;
  char savedir[256];
  char *savefilename[SAVE_COUNT];
  char textureFiles[MaxCntrFname][NUMTEXTURES];
  int initTexture;
  int initSurface;
  FILE *savefp[SAVE_COUNT];
  unsigned maxstringsize;
  char *preprocessor;
  int colormap;
  char verbose;
  char warnings;
  char debug;
  char checkEnvironment;
  float xsize; /* window resolution */
  float ysize;
  float xpos; /* position of the drawing window */
  float ypos;
  char *colormapname[MAXCOLORMAPS];
  char iscolormapfile;
  char *materialname[MAXCOLORMAPS];
  char ismaterialfile;
  char doublebuffer;
  char pixmap;
  char overlay_menus;
  char graphics_output;
  char menu_bar;
  char stringIO_homo;
  SavingMode savingMode;
  int filesFullyLoaded; // boolean that is false if a file has not been fully
                        // loaded
  // Saving contours files
  char contourFileName[MaxCntrFname][MAXCONTOURS];
  int con_stored;
  /* if 1, strings are output after application of homomorphism */
  char noborder;
  char batch_save; // allow to save image format from command line
#ifdef WIN32
  char nomsglog;
  /*
  When xypos and xysize are given as relative numbers
  (using -wr -wpr) then they are assumed to include
  the window frame, title etc. If -w -wp are given
  they are assumed to apply to the client area only.
  */
  char compensateforframe;
#endif
};

typedef struct COMLINEPARAM COMLINEPARAM;

extern COMLINEPARAM clp;

#ifdef __cplusplus
}
#endif

#else
#error File already included
#endif
