/*
 * poly.h
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
 *
 */
#ifndef POLY_H
#define POLY_H

#define GeomPolygonCreate(r, p, f)                                             \
  GeomCreate((GeomRef)PolygonCreate(r, p, f), PolygonMethods())

/*
 * Polygon
 */
typedef struct {
  Vector norm;    /* Normal to polygon */
  Float d;        /* Plane constant */
  char index;     /* Which normal coord is "dominant"? */
  Vector *points; /* Array of vertices */
  int npoints;    /* Number of vertices */
} Polygon;

extern Polygon *PolygonCreate();
extern Methods *PolygonMethods();
extern int PolygonIntersect(), PolygonEnter();
extern void PolygonBounds(), PolygonStats();
extern char *PolygonName();

int PolygonNormal(Polygon *poly, Vector *pos, Vector *nrm, Vector *gnrm);
void PolygonUV(Polygon *poly, Vector *pos, Vector *norm, Vec2d *uv,
               Vector *dpdu, Vector *dpdv);

#endif /* POLY_H */
