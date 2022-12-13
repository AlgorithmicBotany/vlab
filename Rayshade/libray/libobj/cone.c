/*
 * cone.c
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
#include "cone.h"

static Methods *iConeMethods = NULL;
static char coneName[] = "cone";

unsigned long ConeTests, ConeHits;

Cone *ConeCreate(br, bot, ar, apex) Vector *bot, *apex;
Float br, ar;
{
  Cone *cone;
  Float tantheta, lprime, tlen, len, dtmp;
  Vector axis, base, tmp;

  /*
   * The passed basepoint must be closer to the origin of the
   * cone than the apex point, implying that the base radius
   * must be smaller than the apex radius.  If the values passed
   * reflect the opposite, we switch everything.
   */
  if (ar < br) {
    tmp = *bot;
    *bot = *apex;
    *apex = tmp;
    dtmp = br;
    br = ar;
    ar = dtmp;
  } else if (equal(ar, br)) {
    RLerror(RL_WARN, "Cone is a cylinder -- discarding.\n");
    return (Cone *)NULL;
  }
  /*
   * Find the axis and axis length.
   */
  VecSub(*apex, *bot, &axis);
  len = VecNormalize(&axis);
  if (len < EPSILON) {
    RLerror(RL_WARN, "Degenerate cone.\n");
    return (Cone *)NULL;
  }

  cone = (Cone *)share_malloc(sizeof(Cone));

  /*
   * To render a cone, we transform the desired cone into
   * a canonical, Z-axis aligned, unit length, unit radius
   * at the apex cone.
   *
   * Here, we construct the transformation matrix to map
   * from cone<-->world space.
   */

  /*
   * "tantheta" is the change in radius per unit length along
   * the cone axis.
   */
  tantheta = (ar - br) / len;
  /*
   * lprime defines the distance along the axis where the cone starts
   */
  lprime = br / tantheta;
  /*
   * Find the true base (origin) of the cone.
   */
  VecScale(-lprime, axis, &base);
  VecAdd(base, *bot, &base);
  /*
   * tlen is the total length of the cone.
   */
  tlen = lprime + len;
  /*
   * start_dist is the distance from the origin of the canonical
   * cone at which the cone begins
   */
  cone->start_dist = lprime / tlen;
  CoordSysTransform(&base, &axis, ar, tlen, &cone->trans);
  return cone;
}

Methods *ConeMethods() {
  if (iConeMethods == (Methods *)NULL) {
    iConeMethods = MethodsCreate();
    iConeMethods->name = ConeName;
    iConeMethods->create = (GeomCreateFunc *)ConeCreate;
    iConeMethods->methods = ConeMethods;
    iConeMethods->intersect = ConeIntersect;
    iConeMethods->normal = ConeNormal;
    iConeMethods->uv = ConeUV;
    iConeMethods->bounds = ConeBounds;
    iConeMethods->stats = ConeStats;
    iConeMethods->checkbounds = TRUE;
    iConeMethods->closed = FALSE;
  }
  return iConeMethods;
}

/*
 * Ray-cone intersection test.  This routine is far from optimal, but
 * it's straight-forward and it works...
 */
int ConeIntersect(cone, ray, mindist, maxdist) Cone *cone;
Ray *ray;
Float mindist, *maxdist;
{
  Float t1, t2, a, b, c, disc, zpos, distfact;
  Ray newray;
  Vector nray, npos;
  Float nmin;

  ConeTests++;

  /*
   * Transform ray from world to cone space.
   */
  newray = *ray;
  distfact = RayTransform(&newray, &cone->trans.itrans);
  nray = newray.dir;
  npos = newray.pos;
  nmin = mindist * distfact;

  a = nray.x * nray.x + nray.y * nray.y - nray.z * nray.z;
  b = nray.x * npos.x + nray.y * npos.y - nray.z * npos.z;
  c = npos.x * npos.x + npos.y * npos.y - npos.z * npos.z;

  if (fabs(a) < EPSILON) {
    /*
     * Only one intersection point...
     */
    t1 = -0.5 * c / b;
    zpos = npos.z + t1 * nray.z;
    if (t1 < nmin || zpos < cone->start_dist || zpos > 1.)
      return FALSE;
    t1 /= distfact;
    if (t1 < *maxdist) {
      *maxdist = t1;
      ConeHits++;
      return TRUE;
    }
    return FALSE;
  } else {
    disc = b * b - a * c;
    if (disc < 0.)
      return FALSE; /* No possible intersection */
    disc = sqrt(disc);
    t1 = (-b + disc) / a;
    t2 = (-b - disc) / a;
    /*
     * Clip intersection points.
     */
    zpos = npos.z + t1 * nray.z;
    if (t1 < nmin || zpos < cone->start_dist || zpos > 1.) {
      zpos = npos.z + t2 * nray.z;
      if (t2 < nmin || zpos < cone->start_dist || zpos > 1.)
        return FALSE;
      else
        t1 = t2 / distfact;
    } else {
      zpos = npos.z + t2 * nray.z;
      if (t2 < nmin || zpos < cone->start_dist || zpos > 1.)
        t1 /= distfact;
      else
        t1 = min(t1, t2) / distfact;
    }
    if (t1 < *maxdist) {
      *maxdist = t1;
      ConeHits++;
      return TRUE;
    }
    return FALSE;
  }
}

/*
 * Compute the normal to a cone at a given location on its surface.
 */
int ConeNormal(cone, pos, nrm, gnrm) Cone *cone;
Vector *pos, *nrm, *gnrm;
{
  Vector npos;

  /*
   * Transform intersection point to cone space.
   */
  npos = *pos;
  PointTransform(&npos, &cone->trans.itrans);

  /*
   * The following is equal to
   * (pos X (0, 0, 1)) X pos
   */
  nrm->x = npos.x * npos.z;
  nrm->y = npos.y * npos.z;
  nrm->z = -npos.x * npos.x - npos.y * npos.y;

  /*
   * Transform normal back to world space.
   */
  NormalTransform(nrm, &cone->trans.itrans);
  *gnrm = *nrm;
  return FALSE;
}

void ConeUV(cone, pos, uv, dpdu, dpdv) Cone *cone;
Vector *pos, *dpdu, *dpdv;
Vec2d *uv;
{
  Vector npos;
  Float val;

  npos = *pos;
  PointTransform(&npos, &cone->trans.itrans);

  uv->v = (npos.z - cone->start_dist) / (1. - cone->start_dist);
  if (npos.z < EPSILON)
    uv->u = 0.;
  else {
    val = npos.x / npos.z;
    /*
     * Be careful not to feed |val| > 1 to acos
     */
    if (val > 1.)
      uv->u = 0.;
    else if (val < -1.)
      uv->u = 0.5;
    else {
      uv->u = acos(val) / TWOPI;
      if (npos.y < 0.)
        uv->u = 1. - uv->u;
    }
  }
  /*
   * dpdv = pos
   * dpdu = dpdv X norm = (-norm->y, norm->x, 0.)
   */
  if (dpdu) {
    *dpdv = npos;
    dpdu->x = -npos.y;
    dpdu->y = npos.x;
    dpdu->z = 0.;
    VecTransform(dpdu, &cone->trans.trans);
    VecTransform(dpdu, &cone->trans.trans);
    (void)VecNormalize(dpdu);
    (void)VecNormalize(dpdv);
  }
}

/*
 * Return the extent of a cone.
 */
void ConeBounds(cone, bounds) Cone *cone;
Float bounds[2][3];
{
  bounds[LOW][X] = bounds[LOW][Y] = -1;
  bounds[HIGH][X] = bounds[HIGH][Y] = 1;
  bounds[LOW][Z] = cone->start_dist;
  bounds[HIGH][Z] = 1;
  /*
   * Transform bounding box to world space.
   */
  BoundsTransform(&cone->trans.trans, bounds);
}

char *ConeName() { return coneName; }

void ConeStats(tests, hits) unsigned long *tests, *hits;
{
  *tests = ConeTests;
  *hits = ConeHits;
}

void ConeMethodRegister(meth) UserMethodType meth;
{
  if (iConeMethods)
    iConeMethods->user = meth;
}
