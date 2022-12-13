/*
 * sphere.c
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
#include "sphere.h"

static Methods *iSphereMethods = NULL;
static char sphereName[] = "sphere";

unsigned long SphTests, SphHits;

/*
 * Create & return reference to a sphere.
 */
Sphere *SphereCreate(r, pos) Float r;
Vector *pos;
{
  Sphere *sphere;

  if (r < EPSILON) {
    RLerror(RL_WARN, "Degenerate sphere.\n");
    return (Sphere *)NULL;
  }

  sphere = (Sphere *)share_malloc(sizeof(Sphere));
  /*
   * sphere->rsq holds the square of the radius.
   */
  sphere->r = r;
  sphere->rsq = r * r;
  sphere->x = pos->x;
  sphere->y = pos->y;
  sphere->z = pos->z;

  return sphere;
}

Methods *SphereMethods() {
  if (iSphereMethods == (Methods *)NULL) {
    iSphereMethods = MethodsCreate();
    iSphereMethods->create = (GeomCreateFunc *)SphereCreate;
    iSphereMethods->methods = SphereMethods;
    iSphereMethods->name = SphereName;
    iSphereMethods->intersect = SphereIntersect;
    iSphereMethods->normal = SphereNormal;
    iSphereMethods->uv = SphereUV;
    iSphereMethods->enter = SphereEnter;
    iSphereMethods->bounds = SphereBounds;
    iSphereMethods->stats = SphereStats;
    iSphereMethods->checkbounds = TRUE;
    iSphereMethods->closed = TRUE;
  }
  return iSphereMethods;
}

/*
 * Ray/sphere intersection test.
 */
int SphereIntersect(sph, ray, mindist, maxdist) Sphere *sph;
Ray *ray;
Float mindist, *maxdist;
{
  Float xadj, yadj, zadj;
  Float b, t, s;

  SphTests++;
  /*
   * Translate ray origin to object space and negate everything.
   * (Thus, we translate the sphere into ray space, which saves
   * us a couple of negations below.)
   */
  xadj = sph->x - ray->pos.x;
  yadj = sph->y - ray->pos.y;
  zadj = sph->z - ray->pos.z;

  /*
   * Solve quadratic equation.
   */
  b = xadj * ray->dir.x + yadj * ray->dir.y + zadj * ray->dir.z;
  t = b * b - xadj * xadj - yadj * yadj - zadj * zadj + sph->rsq;
  if (t < 0.)
    return FALSE;
  t = (Float)sqrt((double)t);
  s = b - t;
  if (s > mindist) {
    if (s < *maxdist) {
      *maxdist = s;
      SphHits++;
      return TRUE;
    }
    return FALSE;
  }
  s = b + t;
  if (s > mindist && s < *maxdist) {
    *maxdist = s;
    SphHits++;
    return TRUE;
  }
  return FALSE;
}

/*
 * Compute normal to sphere at pos
 */
int SphereNormal(sphere, pos, nrm, gnrm) Sphere *sphere;
Vector *pos, *nrm, *gnrm;
{
  nrm->x = (pos->x - sphere->x) / sphere->r;
  nrm->y = (pos->y - sphere->y) / sphere->r;
  nrm->z = (pos->z - sphere->z) / sphere->r;
  *gnrm = *nrm;
  return FALSE;
}

/*
 * Determine if ray enters (TRUE) or leaves (FALSE) sphere at pos
 */
int SphereEnter(Sphere *sphere, Ray *ray, Float mind,
                __attribute__((unused)) Float hitd) {
  Vector pos;

  VecAddScaled(ray->pos, mind, ray->dir, &pos);
  pos.x -= sphere->x;
  pos.y -= sphere->y;
  pos.z -= sphere->z;

  return dotp(&pos, &pos) > sphere->rsq;
}

/*ARGSUSED*/
void SphereUV(Sphere *sphere, Vector *pos, __attribute__((unused)) Vector *norm,
              Vec2d *uv, Vector *dpdu, Vector *dpdv) {
  Float phi, theta;
  Vector realnorm;

  realnorm.x = pos->x - sphere->x;
  realnorm.y = pos->y - sphere->y;
  realnorm.z = pos->z - sphere->z;
  VecNormalize(&realnorm);
  if (realnorm.z > 1.) /* roundoff */
    phi = PI;
  else if (realnorm.z < -1.)
    phi = 0;
  else
    phi = acos(-realnorm.z);

  uv->v = phi / PI;

  if (fabs(uv->v) < EPSILON || equal(uv->v, 1.))
    uv->u = 0.;
  else {
    theta = realnorm.x / sin(phi);
    if (theta > 1.)
      theta = 0.;
    else if (theta < -1.)
      theta = 0.5;
    else
      theta = acos(theta) / TWOPI;

    if (realnorm.y > 0)
      uv->u = theta;
    else
      uv->u = 1 - theta;
  }
  if (dpdu != (Vector *)0) {
    dpdu->x = -realnorm.y;
    dpdu->y = realnorm.x;
    dpdu->z = 0.;
    (void)VecNormalize(dpdu);
    (void)VecNormCross(&realnorm, dpdu, dpdv);
  }
}

void SphereBounds(s, bounds) Sphere *s;
Float bounds[2][3];
{
  bounds[LOW][X] = s->x - s->r;
  bounds[HIGH][X] = s->x + s->r;
  bounds[LOW][Y] = s->y - s->r;
  bounds[HIGH][Y] = s->y + s->r;
  bounds[LOW][Z] = s->z - s->r;
  bounds[HIGH][Z] = s->z + s->r;
}

char *SphereName() { return sphereName; }

void SphereStats(tests, hits) unsigned long *tests, *hits;
{
  *tests = SphTests;
  *hits = SphHits;
}

void SphereMethodRegister(meth) UserMethodType meth;
{
  if (iSphereMethods)
    iSphereMethods->user = meth;
}
