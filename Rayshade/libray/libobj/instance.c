/*
 * instance.c
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
#include "geom.h"
#include "instance.h"

static Methods *iInstanceMethods = NULL;
static char instanceName[] = "instance";

Instance *InstanceCreate(obj) Geom *obj;
{
  Instance *inst;

  if (obj == (Geom *)NULL) {
    RLerror(RL_WARN, "Instance of NULL?\n");
    return (Instance *)NULL;
  }
  inst = (Instance *)share_malloc(sizeof(Instance));
  inst->obj = obj;
  BoundsCopy(obj->bounds, inst->bounds);
  return inst;
}

char *InstanceName() { return instanceName; }

/*
 * Intersect ray & an instance by calling intersect.
 */
int InstanceIntersect(inst, ray, hitlist, mindist, maxdist) Instance *inst;
Ray *ray;
HitList *hitlist;
Float mindist, *maxdist;
{ return intersect(inst->obj, ray, hitlist, mindist, maxdist); }

Methods *InstanceMethods() {
  /*
   * Instances are special in that there is no
   * "convert" method -- when created, they are passed
   * a pointer to the object being instantiated.
   * This means that you will need to set an instance's
   * 'prims' field by hand (e.g., inst->prims = object->prims).
   */
  if (iInstanceMethods == (Methods *)NULL) {
    iInstanceMethods = MethodsCreate();
    iInstanceMethods->methods = InstanceMethods;
    iInstanceMethods->create = (GeomCreateFunc *)InstanceCreate;
    iInstanceMethods->name = InstanceName;
    iInstanceMethods->intersect = InstanceIntersect;
    iInstanceMethods->bounds = InstanceBounds;
    iInstanceMethods->convert = (voidstar)NULL;
    iInstanceMethods->checkbounds = FALSE;
    iInstanceMethods->closed = TRUE;
  }
  return iInstanceMethods;
}

void InstanceBounds(inst, bounds) Instance *inst;
Float bounds[2][3];
{
  GeomComputeBounds(inst->obj);
  BoundsCopy(inst->obj->bounds, inst->bounds);
  BoundsCopy(inst->bounds, bounds);
}

void InstanceMethodRegister(meth) UserMethodType meth;
{
  if (iInstanceMethods)
    iInstanceMethods->user = meth;
}
