/*
 * cylinder.h
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
#ifndef CYLINDER_H
#define CYLINDER_H

#define GeomCylinderCreate(c, a, r)                                            \
  GeomCreate((GeomRef)CylinderCreate(c, a, r), CylinderMethods())

/*
 * Cylinder
 */
typedef struct cylinder {
  Trans trans;
} Cylinder;

extern Cylinder *CylinderCreate();
extern int CylinderIntersect(), CylinderEnter(), CylinderNormal();
extern void CylinderBounds(), CylinderStats();
extern char *CylinderName();
extern Methods *CylinderMethods();
void CylinderUV(Cylinder *cyl, Vector *pos, Vector *norm, Vec2d *uv,
                Vector *dpdu, Vector *dpdv);
#endif /* CYLINDER_H */
