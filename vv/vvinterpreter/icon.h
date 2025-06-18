#ifndef __ICON_H__
#define __ICON_H__

#include <stdio.h>
#include <stdlib.h>
#ifdef IRIX5
#include <sys/bsd_types.h>
#endif
#ifdef SUN4
#include <sys/types.h>
#endif

typedef unsigned char byte;

/*
typedef unsigned char  u_char;
typedef unsigned short u_short;
typedef unsigned int   u_int;
typedef unsigned long  u_long;
*/

typedef struct { byte * pic;	             /* image data */
		 int   w, h;                 /* pic size */
		 int   type;                 /* PIC8 or PIC24 */

		 byte  r[256],g[256],b[256];
		                             /* colormap, if PIC8 */

		 int   normw, normh;         /* 'normal size' of image file
					        (normally eq. w,h, except when
						doing 'quick' load for icons */

		 int   frmType;              /* def. Format type to save in */
		 int   colType;              /* def. Color type to save in */
		 char  fullInfo[128];        /* Format: field in info box */
		 char  shrtInfo[128];        /* short format info */
		 char *comment;              /* comment text */

		 int   numpages;             /* # of page files, if >1 */
		 char  pagebname[64];        /* basename of page files */
	       } PICINFO;

#define PIC8  1
#define PIC24 2

#define F_IRIS 100
#define F_GREYSCALE 101
#define F_FULLCOLOR 102

/* strings in the INFOBOX (used in SetISTR and GetISTR) */
#define NISTR         10    /* number of ISTRs */
#define ISTR_INFO     0
#define ISTR_WARNING  1
#define ISTR_FILENAME 2
#define ISTR_FORMAT   3
#define ISTR_RES      4
#define ISTR_CROP     5
#define ISTR_EXPAND   6
#define ISTR_SELECT   7
#define ISTR_COLOR    8
#define ISTR_COLOR2   9


void flipIcon( PICINFO * icon);

int LoadIRIS( const char * fname, 
	      PICINFO * pinfo);

int WriteIRIS(FILE * fp, 
	      byte * pic,
	      int ptype,
	      int w,
	      int h, 
	      byte * rmap,
	      byte * gmap,
	      byte * bmap,
	      int colorstyle);

#endif
