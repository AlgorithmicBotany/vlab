/*
 * list.c
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
#include "list.h"

static Methods *iListMethods = NULL;
static char listName[] = "list";

List *ListCreate() { return (List *)share_calloc(1, sizeof(List)); }

char *ListName() { return listName; }

/*
 * Take a list whose DATA field points to a linked list of objects and
 * turn it into a List.
 */
int ListConvert(list, objlist) List *list;
Geom *objlist;
{
  int num;

  /*
   * Find the unbounded objects on the list as well as the
   * bounding box of the list.
   */
  list->list = objlist;
  for (num = 0; objlist; objlist = objlist->next)
    num += objlist->prims;
  return num;
}

/*
 * Intersect ray & list of objects.
 */
int ListIntersect(list, ray, hitlist, mindist, maxdist) List *list;
Ray *ray;
HitList *hitlist;
Float mindist, *maxdist;
{
  Geom *objlist;
  Vector vtmp;
  Float s;
  int hit;

  hit = FALSE;
  /*
   * Intersect with unbounded objects.
   */
  for (objlist = list->unbounded; objlist; objlist = objlist->next) {
    if (intersect(objlist, ray, hitlist, mindist, maxdist))
      hit = TRUE;
  }

  /*
   * Check for intersection with bounding box.
   */
  s = *maxdist; /* So maxdist won't be reset. */
  VecAddScaled(ray->pos, mindist, ray->dir, &vtmp);
  if (OutOfBounds(&vtmp, list->bounds) &&
      !BoundsIntersect(ray, list->bounds, mindist, &s))
    /*
     * Ray never hit list.
     */
    return hit;
  /*
   * Else the ray enters list-space before it hits an
   * unbounded object. Intersect with objects on list.
   */
  for (objlist = list->list; objlist; objlist = objlist->next) {
    if (intersect(objlist, ray, hitlist, mindist, maxdist))
      hit = TRUE;
  }

  return hit;
}

Methods *ListMethods() {
  if (iListMethods == (Methods *)NULL) {
    iListMethods = MethodsCreate();
    iListMethods->methods = ListMethods;
    iListMethods->create = (GeomCreateFunc *)ListCreate;
    iListMethods->name = ListName;
    iListMethods->intersect = ListIntersect;
    iListMethods->bounds = ListBounds;
    iListMethods->convert = ListConvert;
    iListMethods->checkbounds = FALSE;
    iListMethods->closed = TRUE;
  }
  return iListMethods;
}

void ListBounds(list, bounds) List *list;
Float bounds[2][3];
{

  BoundsInit(list->bounds);
  /*
   * For each object on the list,
   * compute its bounds...
   */
  list->unbounded =
      GeomComputeAggregateBounds(&list->list, list->unbounded, list->bounds);
  BoundsCopy(list->bounds, bounds);
}

void ListMethodRegister(meth) UserMethodType meth;
{
  if (iListMethods)
    iListMethods->user = meth;
}
