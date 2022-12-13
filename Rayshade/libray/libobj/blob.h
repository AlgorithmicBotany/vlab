/*
 * blob.h
 *
 * Copyright (C) 1990, 1991, Mark Polipec, Craig E. Kolb
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
 *
 */
#ifndef BLOB_H
#define BLOB_H

#define GeomBlobCreate(t, m, n)                                                \
  GeomCreate((GeomRef)BlobCreate(t, m, n), BlobMethods())

#define R_START 1
#define R_END 0

/*
 * Blob
 */
typedef struct {
  Float rs; /* radius */
  Float c4; /* coeefficient */
  Float c2; /* coeefficient */
  Float c0; /* coeefficient */
  Float x;  /* x position */
  Float y;  /* y position */
  Float z;  /* z position */
} MetaVector;

typedef struct {
  int type, pnt;
  Float bound;
  Float c[5];
} MetaInt;

typedef struct {
  Float T;          /* Threshold   */
  int num;          /* number of points */
  MetaVector *list; /* list of points */
  MetaInt *ilist, **iarr;

} Blob;

typedef struct MetaList {
  MetaVector mvec;
  struct MetaList *next;
} MetaList;

extern int BlobIntersect(), BlobNormal();
extern void BlobBounds(), BlobStats();
extern Blob *BlobCreate();
extern char *BlobName();
extern Methods *BlobMethods();

#endif /* BLOB_H */
