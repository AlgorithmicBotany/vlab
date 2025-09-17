/*
 * instance.h
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
#ifndef INSTANCE_H
#define INSTANCE_H
#include "intersect.h"

#define GeomInstanceCreate(o)                                                  \
  GeomCreate((GeomRef)InstanceCreate(o), InstanceMethods())

/*
 * Instance object
 */
typedef struct {
  struct Geom *obj;   /* Geom being instantiated */
  Float bounds[2][3]; /* Bounding box of object */
} Instance;

extern char *InstanceName();
extern int InstanceIntersect(), InstanceConvert();
extern void InstanceBounds();
extern Instance *InstanceCreate(Geom *obj);
extern Methods *InstanceMethods();

#endif /* INSTANCE_H */
