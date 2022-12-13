/*
 * box.c
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
#include "box.h"

static Methods *iBoxMethods = NULL;
static char boxName[] = "box";

unsigned long BoxTests, BoxHits;

Box *BoxCreate(v1, v2) Vector *v1, *v2;
{
  Box *box;
  Vector size;

  VecSub(*v1, *v2, &size);

  if (equal(size.x, 0.) || equal(size.y, 0.) || equal(size.z, 0.)) {
    RLerror(RL_WARN, "Degenerate box.\n");
    return (Box *)NULL;
  }

  box = (Box *)share_malloc(sizeof(Box));
  box->bounds[LOW][X] = min(v1->x, v2->x);
  box->bounds[HIGH][X] = max(v1->x, v2->x);
  box->bounds[LOW][Y] = min(v1->y, v2->y);
  box->bounds[HIGH][Y] = max(v1->y, v2->y);
  box->bounds[LOW][Z] = min(v1->z, v2->z);
  box->bounds[HIGH][Z] = max(v1->z, v2->z);
  return box;
}

Methods *BoxMethods() {
  if (iBoxMethods == (Methods *)NULL) {
    iBoxMethods = MethodsCreate();
    iBoxMethods->create = (GeomCreateFunc *)BoxCreate;
    iBoxMethods->methods = BoxMethods;
    iBoxMethods->name = BoxName;
    iBoxMethods->intersect = BoxIntersect;
    iBoxMethods->normal = BoxNormal;
    iBoxMethods->enter = BoxEnter;
    iBoxMethods->bounds = BoxBounds;
    iBoxMethods->stats = BoxStats;
    iBoxMethods->checkbounds = FALSE;
    iBoxMethods->closed = TRUE;
  }
  return iBoxMethods;
}

int BoxIntersect(box, ray, mindist, maxdist) Box *box;
Ray *ray;
Float mindist, *maxdist;
{
  BoxTests++;
  if (BoundsIntersect(ray, box->bounds, mindist, maxdist)) {
    BoxHits++;
    return TRUE;
  }
  return FALSE;
}

int BoxNormal(box, pos, nrm, gnrm) Vector *pos, *nrm,
    *gnrm; /* point of intersection */
Box *box;
{
  nrm->x = nrm->y = nrm->z = 0.;

  if (equal(pos->x, box->bounds[HIGH][X]))
    nrm->x = 1.;
  else if (equal(pos->x, box->bounds[LOW][X]))
    nrm->x = -1.;
  else if (equal(pos->y, box->bounds[HIGH][Y]))
    nrm->y = 1.;
  else if (equal(pos->y, box->bounds[LOW][Y]))
    nrm->y = -1.;
  else if (equal(pos->z, box->bounds[HIGH][Z]))
    nrm->z = 1.;
  else if (equal(pos->z, box->bounds[LOW][Z]))
    nrm->z = -1.;
  else
    RLerror(RL_WARN, "Confusion in nrmbox!\n");
  *gnrm = *nrm;
  return FALSE;
}

/*
 * Determine if ray enters (TRUE) or leaves (FALSE) box at pos
 */
int BoxEnter(Box *box, Ray *ray, Float mind,
             __attribute__((unused)) Float hitd) {
  Vector pos;

  VecAddScaled(ray->pos, mind, ray->dir, &pos);
  return OutOfBounds(&pos, box->bounds);
}

void BoxBounds(box, bounds) Box *box;
Float bounds[2][3];
{ BoundsCopy(box->bounds, bounds); }

char *BoxName() { return boxName; }

void BoxStats(tests, hits) unsigned long *tests, *hits;
{
  *tests = BoxTests;
  *hits = BoxHits;
}

void BoxMethodRegister(meth) UserMethodType meth;
{
  if (iBoxMethods)
    iBoxMethods->user = meth;
}
