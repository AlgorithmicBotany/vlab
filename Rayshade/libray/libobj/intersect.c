/*
 * intersect.c
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

static void AddToHitList();
/*
 * Number of bounding volume tests.
 * External modules have read access via IntersectStats().
 */
static unsigned long BVTests;

/*
 * Intersect object & ray.  Return distance from "pos" along "ray" to
 * intersection point.  Return value <= 0 indicates no intersection.
 */
int intersect( Geom *obj, /* Geom to be tested. */
	       Ray *ray,                         /* Ray origin, direction. */
	       HitList *hitlist,                 /* Intersection path */
	       Float mindist, Float *maxdist)
{
  Ray newray;
  Vector vtmp;
  Trans *curtrans;
  Float distfact, nmindist, nmaxdist;

  /*
   * Check ray/bounding volume intersection, if required.
   */
  if (obj->methods->checkbounds) {
    VecAddScaled(ray->pos, mindist, ray->dir, &vtmp);
    if (OutOfBounds(&vtmp, obj->bounds)) {
      nmaxdist = *maxdist;
      BVTests++;
      if (!BoundsIntersect(ray, obj->bounds, mindist, &nmaxdist))
        return FALSE;
    }
  }

  newray = *ray;
  nmindist = mindist;
  nmaxdist = *maxdist;

  /*
   * Transform the ray if necessary.
   */
  if (obj->trans != (Trans *)0) {
    /*
     * If object's idea of the current time is wrong,
     * update the transformations.
     */
    if (obj->animtrans && !equal(obj->timenow, ray->time)) {
      TransResolveAssoc(obj->trans);
    }

    /*
     * Transforming the ray can change the distance between
     * the ray origin and the point of intersection.
     * We save the amount the ray is "stretched" and later
     * divide the computed distance by this amount.
     */
    distfact = 1.;
    for (curtrans = obj->transtail; curtrans; curtrans = curtrans->prev)
      distfact *= RayTransform(&newray, &curtrans->itrans);
    nmindist *= distfact;
    nmaxdist *= distfact;
  }
  /*
   * Geom has been updated to current time.
   */
  obj->timenow = ray->time;

  /*
   * Call correct intersection routine.
   */
  if (IsAggregate(obj)) {
    /*
     * Aggregate
     */
    if (!(*obj->methods->intersect)(obj->obj, &newray, hitlist, nmindist,
                                    &nmaxdist))
      return FALSE;
  } else {
    /*
     * Primitive
     */
    if (!(*obj->methods->intersect)(obj->obj, &newray, nmindist, &nmaxdist))
      return FALSE;
    hitlist->nodes = 0;
  }

  /*
   * Had a hit -- add ray, distance and object to tail of hitlist.
   */
  AddToHitList(hitlist, &newray, nmindist, nmaxdist, obj);

  /*
   * Set dist to distance to intersection point from the origin
   * of the untransformed ray.
   */
  if (obj->trans != (Trans *)0)
    *maxdist = nmaxdist / distfact;
  else
    *maxdist = nmaxdist;

  return TRUE;
}

static void AddToHitList(hitlist, ray, mind, dist, obj) HitList *hitlist;
Ray *ray;
Float mind, dist;
Geom *obj;
{
  HitNode *np;
  Trans *list;

  np = &hitlist->data[hitlist->nodes++];

  np->ray = *ray;
  np->obj = obj;
  np->mindist = mind;
  np->dist = dist;
  np->enter = 0;

  if (obj->trans) {
    /*
     * Compute total transformation, forward and inverse,
     * for this object, and store in hitlist for use later.
     */
    TransCopy(obj->trans, &np->trans);
    for (list = obj->trans->next; list; list = list->next)
      TransCompose(&np->trans, list, &np->trans);
    np->dotrans = TRUE;
  } else
    np->dotrans = FALSE;
}

/*
 * Return intersection statistics.
 * Currently, this is limited to the # of bounding volume test performed.
 */
void IntersectStats(bvtests) unsigned long *bvtests;
{ *bvtests = BVTests; }
