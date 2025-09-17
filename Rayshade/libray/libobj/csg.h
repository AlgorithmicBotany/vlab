/*
 * csg.h
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
#ifndef CSG_H
#define CSG_H

#define GeomCsgCreate(t) GeomCreate((GeomRef)CsgCreate(t), CsgMethods())

/*
 * CSG (Constructive Solid Geometry)
 */
#define CSG_UNION 0
#define CSG_INTERSECT 1
#define CSG_DIFFERENCE 2

typedef struct Csg {
  char my_operator; /* Union, Intersect, or Difference */
  struct Geom *obj1, *obj2;
  int (*intmeth)(); /* Intersection method. */
  float bounds[2][3];
} Csg;

extern char *CsgName();
extern Csg *CsgCreate(int op);
extern int CsgIntersect(), CsgConvert();
extern void CsgBounds();
int FirstCSGGeom(HitList *hitlist);
extern Methods *CsgMethods();

#endif /* CSG_H */
