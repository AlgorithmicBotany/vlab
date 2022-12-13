/*
 * torus.c
 *
 * Copyright (C) 1990, 1991, Mark Podlipec, Craig E. Kolb
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
#include "torus.h"
#include "roots.h"
static Methods *iTorusMethods = NULL;
static char torusName[] = "torus";
unsigned long TorusTests, TorusHits;

/*
 * Create & return reference to a torus.
 */
Torus *TorusCreate(a, b, pos, norm) Float a, b;
Vector *pos, *norm;
{
  Torus *torus;
  Vector tmpnrm;

  if ((a < EPSILON) || (b < EPSILON)) {
    RLerror(RL_WARN, "Degenerate torus.\n");
    return (Torus *)NULL;
  }

  tmpnrm = *norm;
  if (VecNormalize(&tmpnrm) == 0.) {
    RLerror(RL_WARN, "Degenerate torus normal.\n");
    return (Torus *)NULL;
  }

  torus = (Torus *)share_malloc(sizeof(Torus));

  /*
   * torus->aa holds the square of the swept radius.
   * torus->bb holds the square of the tube radius.
   */
  torus->a = a;
  torus->b = b;
  torus->aa = a * a;
  torus->bb = b * b;
  CoordSysTransform(pos, &tmpnrm, 1., 1., &torus->trans);

  return torus;
}

/*
 * Ray/torus intersection test.
 */
int TorusIntersect(torus, inray, mindist, maxdist) Torus *torus;
Ray *inray;
Float mindist, *maxdist;
{
  Vector pos, ray;
  double c[5], s[4], dist, nmin;
  Float distfactor;
  register int num, i;

  TorusTests++;

  /* Transform ray into toroid space */
  {
    Ray tmpray;
    tmpray = *inray;
    distfactor = RayTransform(&tmpray, &torus->trans.itrans);
    ray = tmpray.dir;
    pos = tmpray.pos;
    nmin = mindist * distfactor;
  }

  /*
   * Original Equations for Toroid with position of (0,0,0) and axis (0,0,1)
   *
   * Equation for two circles of radius b centered at (-a,0,0) and (a,0,0)
   *
   *      ((R-a)^2 + z*2 - b*b) * ((R+a)^2 + z*z - b*b) = 0
   *
   *       a         is swept radius
   *       b         is tube  radius
   *
   * subsitute R*R = x*x + y*y  to rotate about z-axis
   *
   * and substitute the parametric ray equations:
   *
   *       x = x0 + t * x1;
   *       y = y0 + t * y1;
   *       z = z0 + t * z1;
   *
   * to get a Quartic in t.
   *
   *       c4*t^4 + c3*t^3 + c2*t^2 + c1*t + c0 = 0
   *
   * where the coefficients are:
   *
   *       c4 =   (x1s + y1s + z1s) * (x1s + y1s + z1s);
   *       c3 =   4.0 * (tx + ty + tz) * (x1s + y1s + z1s);
   *       c2 =   2.0 * (x1s + y1s + z1s) * (x0s + y0s + z0s - as - bs)
   *            + 4.0 * (tx + ty + tz)    * (tx + ty + tz)
   *            + 4.0 * as * z1s;
   *       c1 =   4.0 * (tx + ty + tz) * (x0s + y0s + z0s - as - bs)
   *            + 8.0 * as * tz;
   *       c0 =   (x0s + y0s + z0s - as - bs) * (x0s + y0s + z0s - as - bs)
   *            + 4.0 * as * (z0s - bs);
   *
   *       as        is swept radius squared
   *       bs        is tube  radius squared
   *      (x0,y0,z0) is origin of ray to be tested
   *      (x1,y1,z1) is direction vector of ray to be tested
   *       tx        is x0 * x1
   *       ty        is y0 * y1
   *       tz        is z0 * z1
   *
   *   Since the direction vector (x1,y1,z1) is normalized:
   *              (x1s + y1s + z1s) = 1.0
   *
   *   Also let     g2s = (x1 * x0) + (y1 * y0) + (z1 * z0)
   *    and let     g0s = (x0 * x0) * (y0 * y0) + (z0 * z0) - as - bs
   *    since these terms are used fairly often
   */
  {
    register Float g0s, g2s;
    register Float as, bs;
    register Float z0s, z1s, tz;

    as = torus->aa;
    bs = torus->bb;
    z0s = pos.z * pos.z;
    z1s = ray.z * ray.z;
    tz = pos.z * ray.z;
    g0s = pos.x * pos.x + pos.y * pos.y + z0s - as - bs;
    g2s = pos.x * ray.x + pos.y * ray.y + tz;

    c[4] = 1.0;
    c[3] = 4.0 * g2s;
    c[2] = 2.0 * (g0s + 2.0 * g2s * g2s + 2.0 * as * z1s);
    c[1] = 4.0 * (g2s * g0s + 2.0 * as * tz);
    c[0] = g0s * g0s + 4.0 * as * (z0s - bs);
  }

  /* use GraphGem's Solve Quartic to find roots */
  num = SolveQuartic(c, s);

  /* no roots - return 0. */
  if (num == 0)
    return FALSE;

  /* of roots return the smallest root > EPSILON */
  dist = 0.0;
  for (i = 0; i < num; i++) {
    /* if root is in front of ray origin */
    if (s[i] > nmin) {
      /* first valid root */
      if (dist == 0.0)
        dist = s[i];
      /* else update only if it's closer to ray origin */
      else if (s[i] < dist)
        dist = s[i];
    }
  }
  dist /= distfactor;
  if (dist > mindist && dist < *maxdist) {
    *maxdist = dist;
    TorusHits++;
    return TRUE;
  }
  return FALSE;
}

/*
 * Compute the normal to a torus at a given location on its surface
 */
int TorusNormal(torus, rawpos, nrm, gnrm) Torus *torus;
Vector *rawpos, *nrm, *gnrm;
{
  Vector pos;
  register Float dist, posx, posy, xm, ym;

  /* Transform intersection point to torus space. */
  pos = *rawpos;
  PointTransform(&pos, &torus->trans.itrans);

  /*
   *  The code for the toroid is simpified by always having the axis
   *  be the z-axis and then transforming information to and from
   *  toroid space.
   *
   *  Flatten toroid by ignoring z. Now imagine a knife cutting from
   *  center of toroid to the ray intersection point(x,y). The point
   *  on the tube axis(a circle about the origin with radius 'a')
   *  where the knife cuts is (xm,ym,zm=0). Unflattening the toroid,
   *  the normal at the point [x,y,z] is (x-xm,y-ym,z). Of course, we
   *  must transform the normal back into world coordinates.
   *  Instead of messing with tan-1,sin and cos, we can find (xm,ym)
   *  by using the proportions:
   *
   *     xm     x           ym     y
   *    ---- = ----   and  ---- = ----
   *     a     dist         a     dist
   *
   *       a         is the swept radius
   *    [x,y,z]      is the point on the toroids surface
   *      dist       is the distance from the z-axis (x*x + y*y).
   *    [xm,ym,zm=0] is the point on the tube's axis
   *
   */

  /* find distance from axis */
  posx = pos.x;
  posy = pos.y;
  dist = sqrt(posx * posx + posy * posy);

  if (dist > EPSILON) {
    xm = torus->a * posx / dist;
    ym = torus->a * posy / dist;
  } else /* ERROR - dist should not be < EPSILON (should never happen)*/
  {
    xm = 0.0;
    ym = 0.0;
  }

  /* normal is vector from [xm,ym,zm] to [x,y,z] */
  nrm->x = posx - xm;
  nrm->y = posy - ym;
  nrm->z = pos.z; /* note by default zm is 0 */

  /* Transform normal back to world space. */
  NormalTransform(nrm, &torus->trans.itrans);
  *gnrm = *nrm;
  return FALSE;
}

void TorusUV(torus, pos, norm, uv, dpdu, dpdv) Torus *torus;
Vector *pos, *norm, *dpdu, *dpdv;
Vec2d *uv;
{
  Vector npos;
  Float costheta, sintheta, rad, cosphi;

  npos = *pos;
  PointTransform(&npos, &torus->trans.itrans);
  /*
   * u = theta / 2PI
   */
  rad = sqrt(npos.x * npos.x + npos.y * npos.y);
  costheta = npos.x / rad;
  sintheta = npos.y / rad;
  if (costheta > 1.) /* roundoff */
    uv->u = 0.;
  else if (costheta < -1.)
    uv->u = 0.5;
  else
    uv->u = acos(costheta) / TWOPI;
  if (sintheta < 0.)
    uv->u = 1. - uv->u;
  if (dpdu) {
    dpdu->x = -npos.y;
    dpdu->y = npos.x;
    dpdu->z = 0.;
    VecTransform(dpdu, &torus->trans.trans);
    (void)VecNormalize(dpdu);
  }
  /*
   * sinphi = npos.z / tor->b;
   * cosphi = rad - tor->a;
   * cosphi is negated in order to make texture 'seam'
   * occur on the interior of the torus.
   */
  cosphi = -(rad - torus->a) / torus->b;
  if (cosphi > 1.)
    uv->v = 0.;
  else if (cosphi < -1.)
    uv->v = 0.5;
  else
    uv->v = acos(cosphi) / TWOPI;
  if (npos.z > 0.) /* if sinphi > 0... */
    uv->v = 1. - uv->v;
  /*
   * dpdv = norm X dpdu
   */
  if (dpdv) {
    VecCross(norm, dpdu, dpdv);
    VecTransform(dpdv, &torus->trans.trans);
    (void)VecNormalize(dpdv);
  }
}

/*
 * Return the extent of a torus.
 */
void TorusBounds(torus, bounds) Torus *torus;
Float bounds[2][3];
{
  bounds[LOW][X] = bounds[LOW][Y] = -(torus->a + torus->b);
  bounds[HIGH][X] = bounds[HIGH][Y] = torus->a + torus->b;
  bounds[LOW][Z] = -torus->b;
  bounds[HIGH][Z] = torus->b;
  /*
   * Transform bounding box to world space.
   */
  BoundsTransform(&torus->trans.trans, bounds);
}

char *TorusName() { return torusName; }

void TorusStats(tests, hits) unsigned long *tests, *hits;
{
  *tests = TorusTests;
  *hits = TorusHits;
}

Methods *TorusMethods() {
  if (iTorusMethods == NULL) {
    iTorusMethods = MethodsCreate();
    iTorusMethods->create = (GeomCreateFunc *)TorusCreate;
    iTorusMethods->methods = TorusMethods;
    iTorusMethods->name = TorusName;
    iTorusMethods->intersect = TorusIntersect;
    iTorusMethods->bounds = TorusBounds;
    iTorusMethods->normal = TorusNormal;
    iTorusMethods->uv = TorusUV;
    iTorusMethods->stats = TorusStats;
    iTorusMethods->checkbounds = TRUE;
    iTorusMethods->closed = TRUE;
  }
  return iTorusMethods;
}

void TorusMethodRegister(meth) UserMethodType meth;
{
  if (iTorusMethods)
    iTorusMethods->user = meth;
}
