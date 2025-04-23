/*
 * mapping.h
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
#ifndef MAPPING_H
#define MAPPING_H

#include "libobj/geom.h"

#define PRIMSPACE 01 /* Mapping uses data in prim space */
#define OBJSPACE 02  /* Mapping uses data in object space */

typedef struct Mapping {
  int flags; /* mapping flags */
  Vector center, uaxis, vaxis, norm;
  Float dist;
  RSMatrix m;
  void (*method)();
} Mapping;

extern Mapping *UVMappingCreate(), 
    *SphereMappingCreate(Vector *center, Vector *norm, Vector *uaxis), 
    *CylMappingCreate(Vector *center, Vector *norm, Vector *uaxis),
    *LinearMappingCreate(Vector *center, Vector *vaxis, Vector *uaxis);

void UVMapping(Mapping *map, Geom *obj, Vector *pos, Vector *norm, Vec2d *uv,
               Vector *dpdu, Vector *dpdv);
void SphereMapping(Mapping *map, Geom *obj, Vector *pos, Vector *norm,
                   Vec2d *uv, Vector *dpdu, Vector *dpdv);
void CylinderMapping(Mapping *map, Geom *obj, Vector *pos, Vector *norm,
                     Vec2d *uv, Vector *dpdu, Vector *dpdv);

void LinearMapping(Mapping *map, Geom *obj, Vector *pos, Vector *norm,
                   Vec2d *uv, Vector *dpdu, Vector *dpdv);

#endif /* MAPPING_H */
