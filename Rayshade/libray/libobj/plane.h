/*
 * plane.h
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
#ifndef PLANE_H
#define PLANE_H

#define GeomPlaneCreate(p, n)                                                  \
  GeomCreate((GeomRef)PlaneCreate(p, n), PlaneMethods())

/*
 * Plane
 */
typedef struct {
  Vector norm, pos; /* Plane normal, point on plane */
  Float d;          /* Plane constant. */
} Plane;

extern Plane *PlaneCreate(Vector *pos, Vector *norm);
extern int PlaneIntersect();
int PlaneNormal(Plane *plane, Vector *pos, Vector *nrm, Vector *gnrm);
extern void PlaneUV(), PlaneStats();
void PlaneBounds(Plane *plane, Float bounds[2][3]);

extern char *PlaneName();
extern Methods *PlaneMethods();

#endif /* PLANE_H */
