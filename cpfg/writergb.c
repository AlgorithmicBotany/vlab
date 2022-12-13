/*  based on:
 *
 *   	fastimg -
 *		Faster reading and writing of image files.
 *
 *      This code should work on machines with any byte order.
 *
 *	Could someone make this run real fast using multiple processors
 *	or how about using memory mapped files to speed it up?
 *
 *				Paul Haeberli - 1991
 */

#ifdef WIN32
#include "warningset.h"

#pragma warning(error : 4013)
#endif

#include <memory.h>

#include "writergb.h"

#define PIC8 1
#define PIC24 2

#define F_IRIS 100
#define F_GREYSCALE 101
#define F_FULLCOLOR 102

#define IMAGIC 0732

#define BPPMASK 0x00ff
#define ITYPE_VERBATIM 0x0000
#define ITYPE_RLE 0x0100
#define ISRLE(type) (((type)&0xff00) == ITYPE_RLE)
#define ISVERBATIM(type) (((type)&0xff00) == ITYPE_VERBATIM)
#define BPP(type) ((type)&BPPMASK)
#define RLE(bpp) (ITYPE_RLE | (bpp))
#define VERBATIM(bpp) (ITYPE_VERBATIM | (bpp))

#define FERROR(fp) (ferror(fp) || feof(fp))

typedef struct strImage {
  unsigned short imagic; /* stuff saved on disk . . */
  unsigned short type;
  unsigned short dim;
  unsigned short xsize;
  unsigned short ysize;
  unsigned short zsize;
  unsigned long min;
  unsigned long max;
  unsigned long wastebytes;
  char name[80];
  unsigned long colormap;

  long file; /* stuff used in core only */
  unsigned short flags;
  short dorev;
  short x;
  short y;
  short z;
  short cnt;
  unsigned short *ptr;
  unsigned short *base;
  unsigned short *tmpbuf;
  unsigned long offset;
  unsigned long rleend;    /* for rle images */
  unsigned long *rowstart; /* for rle images */
  long *rowsize;           /* for rle images */
} IMAGE;

#define TAGLEN (5)

#define RINTLUM (79)
#define GINTLUM (156)
#define BINTLUM (21)

#define OFFSET_R 3 /* this is byte order dependent */
#define OFFSET_G 2
#define OFFSET_B 1
#define OFFSET_A 0

#define ILUM(r, g, b)                                                          \
  ((int)(RINTLUM * (r) + GINTLUM * (g) + BINTLUM * (b)) >> 8)
#define CHANOFFSET(z) (3 - (z)) /* this is byte order dependent */

static void lumrow(unsigned char *, unsigned char *, int);
static int compressrow(unsigned char *, unsigned char *, int, int);
static void writetab(FILE *, unsigned long *, int);

static void putshort(FILE *, int);
static void putlong(FILE *, unsigned long);

/*************************************************/
/* IRIS image-writing routines                   */
/*************************************************/

/*************************************************/
int WriteIRIS(FILE *fp, unsigned char *pic, int ptype, int w, int h,
              unsigned char *rmap, unsigned char *gmap, unsigned char *bmap,
              int colorstyle) {
  /* writes a greyscale or 24-bit RGB IRIS file to the already open
          stream, rle compressed */

  IMAGE img;
  int i, j, pos, len, tablen, rlebuflen, zsize;
  unsigned long *starttab = NULL, *lengthtab = NULL;
  unsigned char *rlebuf = NULL, *pptr = NULL;
  unsigned char *lumbuf = NULL, *lptr = NULL, *longpic = NULL;

  memset(&img, 0, sizeof(IMAGE));

  /* write header information */
  fwrite(&img, sizeof(IMAGE), (size_t)1, fp);
  fseek(fp, 0L, 0);

  /* load up header */
  img.imagic = IMAGIC;
  img.type = ITYPE_RLE | (1 & BPPMASK); /* RLE, 1 byteperpix */
  img.dim = (unsigned short)((colorstyle == F_FULLCOLOR) ? 3 : 2);
  img.xsize = (unsigned short)w;
  img.ysize = (unsigned short)h;
  img.zsize = (unsigned short)(zsize = (colorstyle == F_FULLCOLOR) ? 3 : 1);
  img.min = 0;
  img.max = 255;

  putshort(fp, img.imagic);
  putshort(fp, img.type);
  putshort(fp, img.dim);
  putshort(fp, img.xsize);
  putshort(fp, img.ysize);
  putshort(fp, img.zsize);
  putlong(fp, img.min);
  putlong(fp, img.max);
  putlong(fp, 0L);
  fwrite("no name", (size_t)8, (size_t)1, fp);

  if (ferror(fp)) {
    fclose(fp);
    return -1;
  }

  /* allocate RLE compression tables & stuff */
  rlebuflen = 2 * w + 10;
  tablen = h * zsize;

  starttab = (unsigned long *)malloc((size_t)tablen * sizeof(long));
  lengthtab = (unsigned long *)malloc((size_t)tablen * sizeof(long));
  rlebuf = (unsigned char *)malloc((size_t)rlebuflen);
  lumbuf = (unsigned char *)malloc((size_t)w * 4);

  if (!starttab || !lengthtab || !rlebuf || !lumbuf)
    goto cleanup; /*FatalError("out of memory in WriteIRIS()");*/

  pos = 512 + 2 * (tablen * 4);
  fseek(fp, (long)pos, 0);

  /* convert image into 4-byte per pix image that the compress routines want */
  longpic = (unsigned char *)malloc((size_t)w * h * 4);
  if (!longpic)
    goto cleanup; /*FatalError("couldn't malloc longpic in WriteIRIS()");*/

  for (i = 0, pptr = pic; i < h; i++) {
    lptr = longpic + ((h - 1) - i) * (w * 4); /* vertical flip */
    if (ptype == PIC8) {
      /* colormapped */
      for (j = 0; j < w; j++, pptr++) {
        *lptr++ = 0xff;
        *lptr++ = bmap[*pptr];
        *lptr++ = gmap[*pptr];
        *lptr++ = rmap[*pptr];
      }
    } else {
      /* PIC24 */
      for (j = 0; j < w; j++, pptr += 3) {
        *lptr++ = 0xff;
        *lptr++ = pptr[0];
        *lptr++ = pptr[1];
        *lptr++ = pptr[2];
      }
    }
  }

  /* compress and write the data */
  lptr = longpic;
  for (i = 0; i < h; i++) {
    for (j = 0; j < zsize; j++) {
      if (zsize == 1) {
        lumrow(lptr, lumbuf, w);
        len = compressrow(lumbuf, rlebuf, CHANOFFSET(j), w);
      } else {
        len = compressrow(lptr, rlebuf, CHANOFFSET(j), w);
      }

      if (len > rlebuflen) {
        exit(1);
      }

      fwrite(rlebuf, (size_t)len, (size_t)1, fp);
      starttab[i + j * h] = pos;
      lengthtab[i + j * h] = len;
      pos += len;
    }
    lptr += (w * 4);
  }

  /* write out line start and length tables */
  fseek(fp, 512L, 0);
  writetab(fp, starttab, tablen);
  writetab(fp, lengthtab, tablen);

cleanup:
  if (NULL != starttab)
    free(starttab);
  if (NULL != lengthtab)
    free(lengthtab);
  if (NULL != rlebuf)
    free(rlebuf);
  if (NULL != lumbuf)
    free(lumbuf);
  if (NULL != longpic)
    free(longpic);

  if (ferror(fp))
    return -1;

  return 0;
}

/*************************************/
static void lumrow(unsigned char *rgbptr, unsigned char *lumptr, int n) {
  lumptr += CHANOFFSET(0);
  while (n--) {
    *lumptr = (unsigned char)ILUM(rgbptr[OFFSET_R], rgbptr[OFFSET_G],
                                  rgbptr[OFFSET_B]);
    lumptr += 4;
    rgbptr += 4;
  }
}

/*************************************/
static int compressrow(unsigned char *lbuf, unsigned char *rlebuf, int z,
                       int cnt) {
  unsigned char *iptr, *ibufend, *sptr, *optr;
  short todo, cc;
  long count;

  lbuf += z;
  iptr = lbuf;
  ibufend = iptr + cnt * 4;
  optr = rlebuf;

  while (iptr < ibufend) {
    sptr = iptr;
    iptr += 8;
    while ((iptr < ibufend) &&
           ((iptr[-8] != iptr[-4]) || (iptr[-4] != iptr[0])))
      iptr += 4;
    iptr -= 8;

    count = (iptr - sptr) / 4;

    while (count) {
      todo = (short)(count > 126 ? 126 : count);
      count -= todo;
      *optr++ = (unsigned char)(0x80 | todo);
      while (todo > 8) {
        optr[0] = sptr[0 * 4];
        optr[1] = sptr[1 * 4];
        optr[2] = sptr[2 * 4];
        optr[3] = sptr[3 * 4];
        optr[4] = sptr[4 * 4];
        optr[5] = sptr[5 * 4];
        optr[6] = sptr[6 * 4];
        optr[7] = sptr[7 * 4];
        optr += 8;
        sptr += 8 * 4;
        todo -= 8;
      }

      while (todo--) {
        *optr++ = *sptr;
        sptr += 4;
      }
    }

    sptr = iptr;
    cc = *iptr;
    iptr += 4;
    while ((iptr < ibufend) && ((char)(*iptr) == cc))
      iptr += 4;

    count = (iptr - sptr) / 4;
    while (count) {
      todo = (short)(count > 126 ? 126 : count);
      count -= todo;
      *optr++ = (unsigned char)todo;
      *optr++ = (unsigned char)cc;
    }
  }

  *optr++ = 0;
  return (optr - rlebuf);
}

/****************************************************/
static void writetab(FILE *outf, unsigned long *tab, int n) {
  while (n) {
    putlong(outf, *tab++);
    n--;
  }
}

/* byte order independent write of shorts and longs. */

static void putshort(FILE *outf, int val) {
  unsigned char buf[2];
  buf[0] = (unsigned char)(val >> 8);
  buf[1] = (unsigned char)(val >> 0);
  fwrite(buf, (size_t)2, (size_t)1, outf);
}

static void putlong(FILE *outf, unsigned long val) {
  unsigned char buf[4];
  buf[0] = (unsigned char)(val >> 24);
  buf[1] = (unsigned char)(val >> 16);
  buf[2] = (unsigned char)(val >> 8);
  buf[3] = (unsigned char)(val >> 0);
  fwrite(buf, (size_t)4, (size_t)1, outf);
}
