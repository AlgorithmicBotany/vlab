/*
 * hf.h
 *
 * Copyright (C) 1989, 1991, Craig E. Kolb
 * All rights reserved.
 *
 * This software may be freely copied, modified, and redistributed
 * provided that this copyright notice is preserved on all copies.
 *
 * You may not distribute this software, in whole or in part, as part of
 * any commercial product without the express consent of the authors.
 *
 * There is no warranty or other guarantee of fitness of this software
 * for any purpose.  It is provided solely "as is".
 *
 */
#ifndef HF_H
#define HF_H

#define GeomHfCreate(f) GeomCreate((GeomRef)HfCreate(f), HfMethods())

/*
 * Any height values <= Hf_UNSET is not considered to be part of the
 * height field. Any trianges containing such a vertex will not be
 * rendered.  This allows one to render non-square height fields.
 */
#define HF_UNSET (-1000.)
/*
 * Number of datapoints in a single cell.  If you've got loads of memory,
 * decrease this number.  The 'optimal' number is quite system-dependent,
 * but something around 20 seems to work well. For systems without much
 * memory, this constant should be greater than or equal to the largest
 * height field which will be rendered, thus making the algorithm
 * non-hierarchical.
 */
#define BESTSIZE 16
/*
 * Size of triangle cache.
 */
#define CACHESIZE 6
/*
 * Used to differentiate between the two triangles used to represent a cell:
 *	a------d
 *      |\     |
 *      | \TRI2|	TRI2 == c-->d-->a-->c
 *      |  \   |
 *      |   \  |
 *	|    \ |
 *      |TRI1 \|	TRI1 == c-->a-->b-->c
 *      b------c
 */
#define TRI1 1
#define TRI2 2

typedef struct hfTri {
  Vector v1, v2, v3, norm;
  Float d;
  char type;
  struct hfTri *next, *prev;
} hfTri;

typedef struct {
  int len;
  hfTri *head, *tail;
} TriQueue;

typedef struct {
  float **data; /* Altitude points */
  float minz, maxz;
  int size, *lsize;   /* # of points/side */
  int BestSize;       /* "best" division size */
  float iBestSize;    /* inverse of above (for faster computation) */
  int levels;         /* log base BestSize of size */
  float ***boundsmax; /* high data values at various resolutions. */
  float ***boundsmin;
  float *spacing;
  hfTri hittri, **q;    /* hit triangle and triangle cache */
  int qtail, qsize;     /* end and length of cache */
  Float boundbox[2][3]; /* bounding box of Hf */
} Hf;

extern Hf *HfCreate(char *filename);
extern int HfIntersect(), HfEnter();
int HfNormal(Hf *hf, Vector *pos, Vector *nrm, Vector *gnrm);
void HfUV(Hf *hf, Vector *pos, Vector *norm, Vec2d *uv, Vector *dpdu,
          Vector *dpdv);

extern void HfBounds(), HfStats();
extern char *HfName();
extern Methods *HfMethods();

#endif /* HF_H */
