/*
 * sphere.h
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
#ifndef SPHERE_H
#define SPHERE_H

#define GeomSphereCreate(r, p)                                                 \
  GeomCreate((GeomRef)SphereCreate(r, p), SphereMethods())

/*
 * Sphere
 */
typedef struct {
  Float r, rsq;  /* radius, radius squared */
  Float x, y, z; /* position */
} Sphere;

extern Sphere *SphereCreate();
extern Methods *SphereMethods();
extern int SphereIntersect(), SphereNormal();
extern void SphereBounds(), SphereStats();
extern char *SphereName();

int SphereEnter(Sphere *sphere, Ray *ray, Float mind, Float hitd);
void SphereUV(Sphere *sphere, Vector *pos, Vector *norm, Vec2d *uv,
              Vector *dpdu, Vector *dpdv);
#endif /* SPHERE_H */
