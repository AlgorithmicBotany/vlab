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




#ifndef	__GL_IMAGE_H__
#define	__GL_IMAGE_H__


/*
 *	Defines for image files . . . .
 *
 *  			Paul Haeberli - 1984
 *      Look in /usr/people/4Dgifts/iristools/imgtools for example code!
 *
 */

#include <stdio.h>
#include <stdint.h>

#ifdef  __cplusplus
extern "C" {
#endif

#define IMAGIC 	0732

/* colormap of images */
#define CM_NORMAL		0	/* file contains rows of values which 
					 * are either RGB values (zsize == 3) 
					 * or greyramp values (zsize == 1) */
#define CM_DITHERED		1
#define CM_SCREEN		2	/* file contains data which is a screen
					 * image; getrow returns buffer which 
					 * can be displayed directly with 
					 * writepixels */
#define CM_COLORMAP		3	/* a colormap file */

#define TYPEMASK		0xff00
#define BPPMASK			0x00ff
#define ITYPE_VERBATIM		0x0000
#define ITYPE_RLE		0x0100
#define ISRLE(type)		(((type) & 0xff00) == ITYPE_RLE)
#define ISVERBATIM(type)	(((type) & 0xff00) == ITYPE_VERBATIM)
#define BPP(type)		((type) & BPPMASK)
#define RLE(bpp)		(ITYPE_RLE | (bpp))
#define VERBATIM(bpp)		(ITYPE_VERBATIM | (bpp))
#define	IBUFSIZE(pixels)	((pixels+(pixels>>6))<<2)
#define	RLE_NOP			0x00

#define	ierror(p)		(((p)->flags&_IOERR)!=0)
#define	ifileno(p)		((p)->file)
#define	getpix(p)		(--(p)->cnt>=0 ? *(p)->ptr++ : ifilbuf(p))
#define putpix(p,x)		(--(p)->cnt>=0 \
				    ? ((int)(*(p)->ptr++=(unsigned)(x))) \
				    : iflsbuf(p,(unsigned)(x)))

typedef struct {
    uint16_t	imagic;		/* stuff saved on disk . . */
    uint16_t 	type;
    uint16_t 	dim;
    uint16_t 	xsize;
    uint16_t 	ysize;
    uint16_t 	zsize;
    uint32_t 	min;
    uint32_t 	max;
    uint32_t	wastebytes;	
    char 		name[80];
    uint32_t	colormap;

    FILE 	*file;		/* stuff used in core only */
    uint16_t 	flags;
    int16_t		dorev;
    int16_t		x;
    int16_t		y;
    int16_t		z;
    int16_t		cnt;
    uint16_t	*ptr;
    uint16_t	*base;
    uint16_t	*tmpbuf;
    uint32_t	offset;
    uint32_t	rleend;		/* for rle images */
    uint32_t	*rowstart;	/* for rle images */
    int32_t		*rowsize;	/* for rle images */
    unsigned char*      tmpbuf2;
} IMAGE;

IMAGE *iopen(const char *file, const char *mode, unsigned int type, unsigned int dim,
	     unsigned int xsize, unsigned int ysize, unsigned int zsize);

/*IMAGE *icreate(...);*/

int getrow(IMAGE*,uint16_t*,unsigned int,unsigned int);
int putrow(IMAGE*,uint16_t*,unsigned int,unsigned int);
void isetname(IMAGE*, char *);
int iclose(IMAGE *);

#define IMAGEDEF		/* for backwards compatibility */

#ifdef  __cplusplus
}
#endif


#endif	/* !__GL_IMAGE_H__ */
