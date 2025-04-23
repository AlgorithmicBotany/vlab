/*
 * geom.h
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
#ifndef GEOM_H
#define GEOM_H

#include "libcommon/common.h"
#include "libcommon/transform.h"
#include "bounds.h"

/*
 * Constants for enter flag in HitNode.
 */
#define EXITING 1
#define ENTERING 2

#define MAXMODELDEPTH 1024 /* Maximum height of DAG. */

typedef char *GeomRef;
typedef GeomRef GeomCreateFunc();

/*
 * If the object has a normal method, it's a primitive
 * otherwise it's an aggregate (or an instance)
 */
#define IsAggregate(o) ((o)->methods->normal == NULL)

/*
 * Geom methods.
 * (p) means applies only to primitive objects
 * (a) means applies only to aggregate objects
 */
typedef struct Methods {
  char *(*name)();              /* Geom name */
  GeomRef (*create)();          /* Create and return ref */
  int (*intersect)(),           /* Ray/obj intersection */
      (*normal)(),              /* Geom normal (p) */
      (*enter)(),               /* Ray enter or exit? (p) */
      (*convert)();             /* Convert from list (a) */
  void (*uv)(),                 /* 2D mapping (p) */
      (*stats)(),               /* Statistics */
      (*bounds)(),              /* Bounding volume */
      (*user)();                /* User-defined method */
  struct Methods *(*methods)(); /* object methods func. */
  char checkbounds,             /* check bbox before int.? */
      closed;                   /* properly closed? */
} Methods;

typedef void (*UserMethodType)();

/*
 * Geom definition
 */
typedef struct Geom {
  char *name;  /* Geom name, if any. */
  GeomRef obj; /* Pointer to object info. */
  Methods *methods;
  unsigned long prims;     /* sum of # primitive objects */
  Float bounds[2][3];      /* Bounding box */
  Float timenow;           /* Geom's idea of what time it is */
  short int animtrans;     /* transformation is animated */
  short int frame;         /* frame for which obj is inited */
  struct Surface *surf;    /* surface, if any */
  struct Trans *trans;     /* Transformation information */
  struct Trans *transtail; /* Double linked list end */
  struct Texture *texture; /* Texture mapping info. */
#ifdef SHAREDMEM
  unsigned long *counter; /* Geoms are shared, counters aren't */
#else
  unsigned long counter; /* "mailbox" for grid intersection */
#endif
  struct Geom *next; /* Next object. */
} Geom;

/*
 * Linked list of pointers to objects.
 */
typedef struct GeomList {
  Geom *obj;
  struct GeomList *next;
} GeomList;

/*
 * Array of hit information.  Stores a path through an object DAG,
 * as well as the ray in 'model' (object) space and the distance from
 * the ray origin to the point of intersection.
 */
typedef struct HitNode {
  Geom *obj;     /* Geom hit */
  Ray ray;       /* Ray */
  Float mindist; /* Amount of ray to ignore */
  Float dist;    /* Distance from ray origin to hit */
  short enter,   /* Enter (TRUE) or Leave (FALSE) obj */
      dotrans;   /* transformations non-identity? */
  Trans trans;   /* parent-->obj and inverse trans */
} HitNode;

/*
 * Structure holding a list of HitNodes.  A maximum of MAXMODELDEPTH
 * nodes can be referenced.
 */
typedef struct HitList {
  int nodes;
  HitNode data[MAXMODELDEPTH];
} HitList;

extern char *GeomName();

extern Geom *GeomCreate(GeomRef objptr, Methods *methods), *GeomCopy(), *GeomCopyNamed(),
    *GeomComputeAggregateBounds();

extern GeomList *GeomStackPush(Geom *obj, GeomList *list), *GeomStackPop(GeomList *list);

extern void PrimUV(), AggregatePrintInfo(), IntersectStats();

extern int AggregateConvert(Geom *obj, Geom *objlist), PrimNormal(),
    TraceRay(); /* application-provided */

extern Methods *MethodsCreate();
extern void GeomComputeBounds();
extern int PrimEnter();
extern int FirstAnimatedGeom();
#endif /* OBJECT_H */
