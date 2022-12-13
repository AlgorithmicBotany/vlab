/*
 * triangle.c
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
#include "triangle.h"

static Methods *iTriangleMethods = NULL;
static char triName[] = "triangle";

unsigned long TriTests, TriHits;

static void TriangleSetdPdUV();

/*
 * Create and return reference to a triangle.
 */
Triangle *TriangleCreate(type, p1, p2, p3, n1, n2, n3, u1, u2, u3,
                         flipflag) int type;
Vector *p1, *p2, *p3, *n1, *n2, *n3;
Vec2d *u1, *u2, *u3;
int flipflag;
{
  Triangle *triangle;
  Vector ptmp, anorm;
  Float d;

  /*
   * Allocate new triangle and primitive to point to it.
   */
  triangle = (Triangle *)share_malloc(sizeof(Triangle));
  triangle->type = type; /* so inttri can tell the difference */

  VecSub(*p2, *p1, &triangle->e[0]);
  VecSub(*p3, *p2, &triangle->e[1]);
  VecSub(*p1, *p3, &triangle->e[2]);

  /* Find plane normal. */
  VecCross(&triangle->e[0], &triangle->e[1], &ptmp);
  triangle->nrm = ptmp;
  if (VecNormalize(&triangle->nrm) == 0.) {
    RLerror(RL_ADVISE, "Degenerate triangle.\n");
    return (Triangle *) NULL;
  }

  if (flipflag)
    VecScale(-1, triangle->nrm, &triangle->nrm);

  triangle->d = dotp(&triangle->nrm, p1);

  triangle->p[0] = *p1;
  triangle->p[1] = *p2;
  triangle->p[2] = *p3;

  if (type == PHONGTRI) {
    if (VecNormalize(n1) == 0. || VecNormalize(n2) == 0. ||
        VecNormalize(n3) == 0.) {
      RLerror(RL_WARN, "Degenerate vertex normal.\n");
      return (Triangle *)NULL;
    }
    triangle->vnorm = (Vector *)Malloc(3 * sizeof(Vector));
    triangle->vnorm[0] = *n1;
    triangle->vnorm[1] = *n2;
    triangle->vnorm[2] = *n3;
    if (flipflag) {
      /* Flip the vertex normals */
      VecScale(-1, triangle->vnorm[0], &triangle->vnorm[0]);
      VecScale(-1, triangle->vnorm[1], &triangle->vnorm[1]);
      VecScale(-1, triangle->vnorm[2], &triangle->vnorm[2]);
    } else if (dotp(&triangle->vnorm[0], &triangle->nrm) < 0.) {
      /*
       * Reverse direction of surface normal on Phong
       * triangle if the surface normal points "away"
       * from the first vertex normal.
       * Note that this means that we trust the vertex
       * normals rather than trust that the user gave the
       * vertices in the correct order.
       */
      RLerror(RL_ADVISE, "Inconsistant triangle normals.\n");
      VecScale(-1., triangle->nrm, &triangle->nrm);
      VecScale(-1., ptmp, &ptmp);
      triangle->d = -triangle->d;
      VecScale(-1., triangle->e[0], &triangle->e[0]);
      VecScale(-1., triangle->e[1], &triangle->e[1]);
      VecScale(-1., triangle->e[2], &triangle->e[2]);
    }
  }

  /*
   * If UV coordinates are given for the vertices, allocate and
   * store them.
   */
  if (u1 && u2 && u3) {
    triangle->uv = (Vec2d *)Malloc(3 * sizeof(Vec2d));
    triangle->uv[0] = *u1;
    triangle->uv[1] = *u2;
    triangle->uv[2] = *u3;
    /* Calculate dpdu and dpdv vectors */
    triangle->dpdu = (Vector *)Malloc(sizeof(Vector));
    triangle->dpdv = (Vector *)Malloc(sizeof(Vector));
    TriangleSetdPdUV(triangle->p, triangle->uv, triangle->dpdu, triangle->dpdv);
  } else {
    triangle->uv = (Vec2d *)NULL;
  }

  /*
   * Find "dominant" part of normal vector.
   */
  anorm.x = fabs(ptmp.x);
  anorm.y = fabs(ptmp.y);
  anorm.z = fabs(ptmp.z);

  /*
   * Scale edges by dominant part of normal.  This makes intersection
   * testing a bit faster.
   */
  if (anorm.x > anorm.y && anorm.x > anorm.z) {
    triangle->index = XNORMAL;
    d = 1. / ptmp.x;
  } else if (anorm.y > anorm.z) {
    triangle->index = YNORMAL;
    d = 1. / ptmp.y;
  } else {
    triangle->index = ZNORMAL;
    d = 1. / ptmp.z;
  }

  VecScale(d, triangle->e[0], &triangle->e[0]);
  VecScale(d, triangle->e[1], &triangle->e[1]);
  VecScale(d, triangle->e[2], &triangle->e[2]);

  return triangle;
}

Methods *TriangleMethods() {
  if (iTriangleMethods == (Methods *)NULL) {
    iTriangleMethods = MethodsCreate();
    iTriangleMethods->create = (GeomCreateFunc *)TriangleCreate;
    iTriangleMethods->methods = TriangleMethods;
    iTriangleMethods->name = TriangleName;
    iTriangleMethods->intersect = TriangleIntersect;
    iTriangleMethods->normal = TriangleNormal;
    iTriangleMethods->uv = TriangleUV;
    iTriangleMethods->bounds = TriangleBounds;
    iTriangleMethods->stats = TriangleStats;
    iTriangleMethods->checkbounds = TRUE;
    iTriangleMethods->closed = FALSE;
  }
  return iTriangleMethods;
}

/*
 * Intersect ray with triangle.  This is an optimized version of the
 * intersection routine from Snyder and Barr's '87 SIGGRAPH paper.
 */
int TriangleIntersect(tri, ray, mindist, maxdist) Triangle *tri;
Ray *ray;
Float mindist, *maxdist;
{
  Float qi1, qi2, s, k, b0, b1, b2;
  Vector pos, dir;

  TriTests++;
  pos = ray->pos;
  dir = ray->dir;
  /*
   * Plane intersection.
   */
  k = dotp(&tri->nrm, &dir);
  if (fabs(k) < EPSILON)
    return FALSE;
  s = (tri->d - dotp(&tri->nrm, &pos)) / k;
  if (s < mindist || s > *maxdist)
    return FALSE;

  if (tri->index == XNORMAL) {
    qi1 = pos.y + s * dir.y;
    qi2 = pos.z + s * dir.z;
    b0 = tri->e[1].y * (qi2 - tri->p[1].z) - tri->e[1].z * (qi1 - tri->p[1].y);
    if (b0 < 0. || b0 > 1.)
      return FALSE;
    b1 = tri->e[2].y * (qi2 - tri->p[2].z) - tri->e[2].z * (qi1 - tri->p[2].y);
    if (b1 < 0. || b1 > 1.)
      return FALSE;
    b2 = tri->e[0].y * (qi2 - tri->p[0].z) - tri->e[0].z * (qi1 - tri->p[0].y);
    if (b2 < 0. || b2 > 1.)
      return FALSE;
  } else if (tri->index == YNORMAL) {
    qi1 = pos.x + s * dir.x;
    qi2 = pos.z + s * dir.z;
    b0 = tri->e[1].z * (qi1 - tri->p[1].x) - tri->e[1].x * (qi2 - tri->p[1].z);
    if (b0 < 0. || b0 > 1.)
      return FALSE;
    b1 = tri->e[2].z * (qi1 - tri->p[2].x) - tri->e[2].x * (qi2 - tri->p[2].z);
    if (b1 < 0. || b1 > 1.)
      return FALSE;
    b2 = tri->e[0].z * (qi1 - tri->p[0].x) - tri->e[0].x * (qi2 - tri->p[0].z);
    if (b2 < 0. || b2 > 1.)
      return FALSE;
  } else {
    qi1 = pos.x + s * dir.x;
    qi2 = pos.y + s * dir.y;
    b0 = tri->e[1].x * (qi2 - tri->p[1].y) - tri->e[1].y * (qi1 - tri->p[1].x);
    if (b0 < 0. || b0 > 1.)
      return FALSE;
    b1 = tri->e[2].x * (qi2 - tri->p[2].y) - tri->e[2].y * (qi1 - tri->p[2].x);
    if (b1 < 0. || b1 > 1.)
      return FALSE;
    b2 = tri->e[0].x * (qi2 - tri->p[0].y) - tri->e[0].y * (qi1 - tri->p[0].x);
    if (b2 < 0. || b2 > 1.)
      return FALSE;
  }

  tri->b[0] = b0;
  tri->b[1] = b1;
  tri->b[2] = b2;

  TriHits++;
  *maxdist = s;
  return TRUE;
}

int TriangleNormal(Triangle *tri, __attribute__((unused)) Vector *pos,
                   Vector *nrm, Vector *gnrm) {
  *gnrm = tri->nrm;

  if (tri->type == FLATTRI) {
    *nrm = tri->nrm;
    return FALSE;
  }

  /*
   * Interpolate normals of Phong-shaded triangles.
   */
  nrm->x = tri->b[0] * tri->vnorm[0].x + tri->b[1] * tri->vnorm[1].x +
           tri->b[2] * tri->vnorm[2].x;
  nrm->y = tri->b[0] * tri->vnorm[0].y + tri->b[1] * tri->vnorm[1].y +
           tri->b[2] * tri->vnorm[2].y;
  nrm->z = tri->b[0] * tri->vnorm[0].z + tri->b[1] * tri->vnorm[1].z +
           tri->b[2] * tri->vnorm[2].z;
  (void)VecNormalize(nrm);
  return TRUE;
}

/*ARGSUSED*/
void TriangleUV(Triangle *tri, Vector *pos,
                __attribute__((unused)) Vector *norm, Vec2d *uv, Vector *dpdu,
                Vector *dpdv)

{
  Float d;

  /*
   * Normalize barycentric coordinates.
   */
  d = tri->b[0] + tri->b[1] + tri->b[2];

  tri->b[0] /= d;
  tri->b[1] /= d;
  tri->b[2] /= d;

  if (dpdu) {
    if (tri->uv == (Vec2d *)NULL) {
      *dpdu = tri->e[0];
      (void)VecNormalize(dpdu);
      VecSub(tri->p[0], *pos, dpdv);
      (void)VecNormalize(dpdv);
    } else {
      *dpdu = *tri->dpdu;
      *dpdv = *tri->dpdv;
    }
  }

  if (tri->uv == (Vec2d *)NULL) {
    uv->v = tri->b[2];
    if (equal(uv->v, 1.))
      uv->u = 0.;
    else
      uv->u = tri->b[1] / (tri->b[0] + tri->b[1]);
  } else {
    /*
     * Compute UV by taking weighted sum of UV coordinates.
     */
    uv->u = tri->b[0] * tri->uv[0].u + tri->b[1] * tri->uv[1].u +
            tri->b[2] * tri->uv[2].u;
    uv->v = tri->b[0] * tri->uv[0].v + tri->b[1] * tri->uv[1].v +
            tri->b[2] * tri->uv[2].v;
  }
}

void TriangleBounds(tri, bounds) Triangle *tri;
Float bounds[2][3];
{
  bounds[LOW][X] = bounds[HIGH][X] = tri->p[0].x;
  bounds[LOW][Y] = bounds[HIGH][Y] = tri->p[0].y;
  bounds[LOW][Z] = bounds[HIGH][Z] = tri->p[0].z;

  if (tri->p[1].x < bounds[LOW][X])
    bounds[LOW][X] = tri->p[1].x;
  if (tri->p[1].x > bounds[HIGH][X])
    bounds[HIGH][X] = tri->p[1].x;
  if (tri->p[2].x < bounds[LOW][X])
    bounds[LOW][X] = tri->p[2].x;
  if (tri->p[2].x > bounds[HIGH][X])
    bounds[HIGH][X] = tri->p[2].x;

  if (tri->p[1].y < bounds[LOW][Y])
    bounds[LOW][Y] = tri->p[1].y;
  if (tri->p[1].y > bounds[HIGH][Y])
    bounds[HIGH][Y] = tri->p[1].y;
  if (tri->p[2].y < bounds[LOW][Y])
    bounds[LOW][Y] = tri->p[2].y;
  if (tri->p[2].y > bounds[HIGH][Y])
    bounds[HIGH][Y] = tri->p[2].y;

  if (tri->p[1].z < bounds[LOW][Z])
    bounds[LOW][Z] = tri->p[1].z;
  if (tri->p[1].z > bounds[HIGH][Z])
    bounds[HIGH][Z] = tri->p[1].z;
  if (tri->p[2].z < bounds[LOW][Z])
    bounds[LOW][Z] = tri->p[2].z;
  if (tri->p[2].z > bounds[HIGH][Z])
    bounds[HIGH][Z] = tri->p[2].z;
}

char *TriangleName() { return triName; }

void TriangleStats(tests, hits) unsigned long *tests, *hits;
{
  *tests = TriTests;
  *hits = TriHits;
}

/*
 * Given three vertices of a triangle and the uv coordinates associated
 * with each, compute directions of u and v axes.
 */
static void TriangleSetdPdUV(p, t, dpdu,
                             dpdv) Vector p[3]; /* Triangle vertices */
Vec2d t[3];          /* uv coordinates for each vertex */
Vector *dpdu, *dpdv; /* u and v axes (return values) */
{
  Float scale;
  int hi, mid, lo;
  Vector base;

  /* sort u coordinates */
  if (t[2].u > t[1].u) {
    if (t[1].u > t[0].u) {
      hi = 2;
      mid = 1;
      lo = 0;
    } else if (t[2].u > t[0].u) {
      hi = 2;
      mid = 0;
      lo = 1;
    } else {
      hi = 0;
      mid = 2;
      lo = 1;
    }
  } else {
    if (t[2].u > t[0].u) {
      hi = 1;
      mid = 2;
      lo = 0;
    } else if (t[1].u > t[0].u) {
      hi = 1;
      mid = 0;
      lo = 2;
    } else {
      hi = 0;
      mid = 1;
      lo = 2;
    }
  }
  if (fabs(t[hi].u - t[lo].u) < EPSILON) {
    /* degenerate axis */
    dpdv->x = dpdv->y = dpdv->z = 0.;
  } else {
    /*
     * Given u coordinates of vertices forming the
     * 'long' edge, find where 'middle'
     * vertex falls on that edge given its u coordinate.
     */
    scale = (t[mid].u - t[lo].u) / (t[hi].u - t[lo].u);
    VecComb(1.0 - scale, p[lo], scale, p[hi], &base);
    /*
     * v axis extends from computed basepoint to
     * middle vertex -- but in which direction?
     */
    if (t[mid].v < ((1.0 - scale) * t[lo].v + scale * t[hi].v))
      VecSub(base, p[mid], dpdv);
    else
      VecSub(p[mid], base, dpdv);
    (void)VecNormalize(dpdv);
  }

  /* sort v coordinates */
  if (t[2].v > t[1].v) {
    if (t[1].v > t[0].v) {
      hi = 2;
      mid = 1;
      lo = 0;
    } else if (t[2].v > t[0].v) {
      hi = 2;
      mid = 0;
      lo = 1;
    } else {
      hi = 0;
      mid = 2;
      lo = 1;
    }
  } else {
    if (t[2].v > t[0].v) {
      hi = 1;
      mid = 2;
      lo = 0;
    } else if (t[1].v > t[0].v) {
      hi = 1;
      mid = 0;
      lo = 2;
    } else {
      hi = 0;
      mid = 1;
      lo = 2;
    }
  }
  if (fabs(t[hi].v - t[lo].v) < EPSILON) {
    /* degenerate axis */
    dpdu->x = dpdu->y = dpdu->z = 0.;
  } else {
    /*
     * Given v coordinates of vertices forming the
     * 'long' edge, find where 'middle'
     * vertex falls on that edge given its v coordinate.
     */
    scale = (t[mid].v - t[lo].v) / (t[hi].v - t[lo].v);
    VecComb(1.0 - scale, p[lo], scale, p[hi], &base);
    /*
     * u axis extends from computed basepoint to
     * middle vertex -- but in which direction?
     */
    if (t[mid].u < ((1.0 - scale) * t[lo].u + scale * t[hi].u))
      VecSub(base, p[mid], dpdu);
    else
      VecSub(p[mid], base, dpdu);
    (void)VecNormalize(dpdu);
  }
}

void TriangleMethodRegister(meth) UserMethodType meth;
{
  if (iTriangleMethods)
    iTriangleMethods->user = meth;
}
