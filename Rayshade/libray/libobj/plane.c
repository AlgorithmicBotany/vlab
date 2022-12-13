/*
 * plane.c
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
#include "plane.h"

static Methods *iPlaneMethods = NULL;
static char planeName[] = "plane";

unsigned long PlaneTests, PlaneHits;

/*
 * create plane primitive
 */
Plane *PlaneCreate(pos, norm) Vector *pos, *norm;
{
  Plane *plane;
  Vector tmpnrm;

  tmpnrm = *norm;
  if (VecNormalize(&tmpnrm) == 0.) {
    RLerror(RL_WARN, "Degenerate plane normal.\n");
    return (Plane *)NULL;
  }
  plane = (Plane *)share_malloc(sizeof(Plane));
  plane->norm = tmpnrm;
  plane->pos = *pos;
  plane->d = dotp(&plane->norm, pos);

  return plane;
}

Methods *PlaneMethods() {
  if (iPlaneMethods == (Methods *)NULL) {
    iPlaneMethods = MethodsCreate();
    iPlaneMethods->name = PlaneName;
    iPlaneMethods->create = (GeomCreateFunc *)PlaneCreate;
    iPlaneMethods->methods = PlaneMethods;
    iPlaneMethods->intersect = PlaneIntersect;
    iPlaneMethods->normal = PlaneNormal;
    iPlaneMethods->uv = PlaneUV;
    iPlaneMethods->bounds = PlaneBounds;
    iPlaneMethods->stats = PlaneStats;
    iPlaneMethods->checkbounds = FALSE;
    iPlaneMethods->closed = FALSE;
  }
  return iPlaneMethods;
}

int PlaneIntersect(plane, ray, mindist, maxdist) Plane *plane;
Ray *ray;
Float mindist, *maxdist;
{
  Float d;

  PlaneTests++;

  d = dotp(&plane->norm, &ray->dir);
  if (fabs(d) < EPSILON)
    return FALSE;
  d = (plane->d - dotp(&plane->norm, &ray->pos)) / d;

  if (d > mindist && d < *maxdist) {
    *maxdist = d;
    PlaneHits++;
    return TRUE;
  }
  return FALSE;
}

/*ARGSUSED*/
int PlaneNormal(Plane *plane, __attribute__((unused)) Vector *pos, Vector *nrm,
                Vector *gnrm) {
  *gnrm = *nrm = plane->norm;
  return FALSE;
}

void PlaneUV(plane, pos, norm, uv, dpdu, dpdv) Plane *plane;
Vector *pos, *norm, *dpdu, *dpdv;
Vec2d *uv;
{
  Vector vec, du, dv;

  VecCoordSys(norm, &du, &dv);
  VecSub(*pos, plane->pos, &vec);

  uv->u = dotp(&vec, &du);
  uv->v = dotp(&vec, &dv);

  if (dpdu)
    *dpdu = du;
  if (dpdv)
    *dpdv = dv;
}

/*ARGSUSED*/
void PlaneBounds(__attribute__((unused)) Plane *plane, Float bounds[2][3]) {
  /*
   * Planes are unbounded by nature.  minx > maxx signifies
   * this.
   */
  bounds[LOW][X] = 1.0;
  bounds[HIGH][X] = -1.0;
}

char *PlaneName() { return planeName; }

void PlaneStats(tests, hits) unsigned long *tests, *hits;
{
  *tests = PlaneTests;
  *hits = PlaneHits;
}

void PlaneMethodRegister(meth) UserMethodType meth;
{
  if (iPlaneMethods)
    iPlaneMethods->user = meth;
}
