/*
 * list.h
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
#ifndef LIST_H
#define LIST_H
#include "intersect.h"

#define GeomListCreate() GeomCreate((GeomRef)ListCreate(), ListMethods())

/*
 * List object
 */
typedef struct {
  struct Geom *list;      /* List of prims/objs. in object */
  struct Geom *unbounded; /* List of unbounded prims. */
  Float bounds[2][3];     /* Bounding box of object */
} List;

extern char *ListName();
extern int ListIntersect(), ListConvert();
extern void ListBounds();
extern List *ListCreate();
extern Methods *ListMethods();

#endif /* LIST_H */
