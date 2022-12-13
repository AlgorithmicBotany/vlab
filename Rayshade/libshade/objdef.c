
/*
 * objdef.c
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
#include "libobj/geom.h"
#include "libobj/intersect.h"
#include "rayshade.h"
#include "options.h"
#include "stats.h"
#include "objdef.h"

static Geom *Objects = NULL; /* named objects */
Geom *World;                 /* top-level object */

/*
 * Return pointer to named object, NULL if no such object has been defined.
 */
Geom *GeomGetNamed(name) char *name;
{
  Geom *otmp;
  for (otmp = Objects; otmp; otmp = otmp->next)
    if (strcmp(name, otmp->name) == 0)
      return otmp;
  return (Geom *)NULL;
}

/*
 * Add object to list of defined objects.  At this point, the object has
 * been converted to the correct type, and obj->next is either NULL or
 * garbage.
 */
void GeomAddToDefined(obj) Geom *obj;
{
  obj->next = Objects;
  Objects = obj;
  if (Options.verbose)
    AggregatePrintInfo(obj, Stats.fstats);
  else
    AggregatePrintInfo(obj, (FILE *)NULL);
}

/*
 * Return a copy of the named object.
 */
Geom *GeomCopyNamed(name) char *name;
{
  Geom *child;

  child = GeomGetNamed(name);
  if (child == (Geom *)NULL)
    RLerror(RL_PANIC, "There is no object named \"%s\".", name);
  child = GeomCopy(child);
  return child;
}

void WorldSetup() {
  extern GeomList *Defstack;

  /*
   * Define World object, if not done previously.
   */
  if (World == (Geom *)NULL) {
    World = Defstack->obj; /* World is topmost object on stack */
    if (Defstack->next)
      RLerror(RL_ABORT, "Geom def stack is screwey.\n");
    World->prims = AggregateConvert(World, World->next);
  }

  GeomComputeBounds(World);

  /*
   * Complain if there are no primitives to be rendered.
   */
  if (World->prims == 0) {
    RLerror(RL_PANIC, "Nothing to be rendered.\n");
  }
}

/*
 * Main ray-spwaning routine required by libray
 */
int TraceRay(ray, hitlist, mindist, maxdist) Ray *ray;
HitList *hitlist;
Float mindist, *maxdist;
{ return intersect(World, ray, hitlist, mindist, maxdist); }
