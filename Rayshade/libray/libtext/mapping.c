/*
 * mapping.c
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
#include "mapping.h"

void UVMapping(), SphereMapping(), CylinderMapping(), LinearMapping();

Mapping *UVMappingCreate() {
  Mapping *res;

  res = (Mapping *)Malloc(sizeof(Mapping));
  res->flags = PRIMSPACE;
  res->method = UVMapping;
  return res;
}

Mapping *SphereMappingCreate(center, norm, uaxis) Vector *center, *norm, *uaxis;
{
  Mapping *res;

  res = (Mapping *)Malloc(sizeof(Mapping));
  res->flags = OBJSPACE;
  res->method = SphereMapping;
  if (center)
    res->center = *center;
  else
    res->center.x = res->center.y = res->center.z = 0.;
  if (norm && uaxis) {
    res->norm = *norm;
    if (VecNormalize(&res->norm) == 0.) {
      RLerror(RL_ABORT, "Degenerate mapping vector.\n");
      return (Mapping *)NULL;
    }
    if (VecNormCross(norm, uaxis, &res->vaxis) == 0.) {
      RLerror(RL_ABORT, "Degenerate mapping vector.\n");
      return (Mapping *)NULL;
    }
    (void)VecNormCross(&res->vaxis, norm, &res->uaxis);
  } else {
    res->norm.x = res->norm.y = res->uaxis.y = res->uaxis.z = res->vaxis.x =
        res->vaxis.z = 0.;
    res->norm.z = res->uaxis.x = res->vaxis.y = 1.;
  }
  return res;
}

Mapping *CylMappingCreate(center, norm, uaxis) Vector *center, *norm, *uaxis;
{
  Mapping *res;

  res = (Mapping *)Malloc(sizeof(Mapping));
  res->flags = OBJSPACE;
  res->method = CylinderMapping;
  if (center)
    res->center = *center;
  else
    res->center.x = res->center.y = res->center.z = 0.;
  if (norm && uaxis) {
    res->norm = *norm;
    if (VecNormalize(&res->norm) == 0.) {
      RLerror(RL_ABORT, "Degenerate mapping vector.\n");
      return (Mapping *)NULL;
    }
    /*
     * Here, uaxis indicates where theta (u) = 0.
     */
    if (VecNormCross(norm, uaxis, &res->vaxis) == 0.) {
      RLerror(RL_ABORT, "Degenerate mapping vector.\n");
      return (Mapping *)NULL;
    }
    (void)VecNormCross(&res->vaxis, norm, &res->uaxis);
  } else {
    res->norm.x = res->norm.y = res->uaxis.y = res->uaxis.z = res->vaxis.x =
        res->vaxis.z = 0.;
    res->norm.z = res->uaxis.x = res->vaxis.y = 1.;
  }
  return res;
}

Mapping *LinearMappingCreate(center, vaxis, uaxis) Vector *center, *vaxis,
    *uaxis;
{
  Mapping *res;
  RSMatrix m;
  Vector n;

  res = (Mapping *)Malloc(sizeof(Mapping));
  res->flags = OBJSPACE;
  res->method = LinearMapping;

  if (center)
    res->center = *center;
  else
    res->center.x = res->center.y = res->center.z = 0.;

  if (uaxis && vaxis) {
    VecCross(uaxis, vaxis, &n);
    /* this is wrong, since uaxis and vaxis
     * give U and V in world space, and we
     * need the inverse.
     */
    ArbitraryMatrix(uaxis->x, uaxis->y, uaxis->z, vaxis->x, vaxis->y, vaxis->z,
                    n.x, n.y, n.z, res->center.x, res->center.y, res->center.z,
                    &m);
    MatrixInvert(&m, &res->m);
    res->uaxis = *uaxis;
    res->vaxis = *vaxis;
    VecNormalize(&res->uaxis);
    VecNormalize(&res->vaxis);
  } else {
    VecScale(-1., res->center, &n);
    TranslationMatrix(n.x, n.y, n.z, &res->m);
    res->uaxis.x = res->vaxis.y = 1.;
    res->uaxis.y = res->uaxis.z = res->vaxis.x = res->vaxis.z = 0.;
  }
  return res;
}

void UVMapping(__attribute__((unused)) Mapping *map, Geom *obj, Vector *pos,
               Vector *norm, Vec2d *uv, Vector *dpdu, Vector *dpdv) {
  PrimUV(obj, pos, norm, uv, dpdu, dpdv);
}

void SphereMapping(Mapping *map, __attribute__((unused)) Geom *obj, Vector *pos,
                   __attribute__((unused))  Vector *norm, Vec2d *uv, __attribute__((unused)) Vector *dpdu,
                   __attribute__((unused)) Vector *dpdv) {
  Vector vtmp;
  Float nx, ny, nz, phi, theta;

  VecSub(*pos, map->center, &vtmp);
  if (VecNormalize(&vtmp) == 0.) {
    /*
     * Point is coincident with origin of sphere.  Punt.
     */
    uv->u = uv->v = 0.;
    return;
  }

  /*
   * Find location of point projected onto unit sphere
   * in the sphere's coordinate system.
   */
  nx = dotp(&map->uaxis, &vtmp);
  ny = dotp(&map->vaxis, &vtmp);
  nz = dotp(&map->norm, &vtmp);

  if (nz > 1.) /* roundoff */
    phi = PI;
  else if (nz < -1.)
    phi = 0;
  else
    phi = acos(-nz);

  uv->v = phi / PI;

  if (fabs(uv->v) < EPSILON || equal(uv->v, 1.))
    uv->u = 0.;
  else {
    theta = nx / sin(phi);
    if (theta > 1.)
      theta = 0.;
    else if (theta < -1.)
      theta = 0.5;
    else
      theta = acos(theta) / TWOPI;

    if (ny > 0)
      uv->u = theta;
    else
      uv->u = 1 - theta;
  }
}

void CylinderMapping(Mapping *map, __attribute__((unused)) Geom *obj,
                     Vector *pos, __attribute__((unused)) Vector *norm,
                     Vec2d *uv, Vector *dpdu, Vector *dpdv) {
  Vector vtmp;
  Float nx, ny, r;

  VecSub(*pos, map->center, &vtmp);
  nx = dotp(&map->uaxis, &vtmp);
  ny = dotp(&map->vaxis, &vtmp);
  uv->v = dotp(&map->norm, &vtmp);

  r = sqrt(nx * nx + ny * ny);

  if (r < EPSILON) {
    uv->u = 0.;
    return;
  }

  nx /= r;
  ny /= r;

  if (fabs(nx) > 1.)
    uv->u = 0.5;
  else
    uv->u = acos(nx) / TWOPI;
  if (ny < 0.)
    uv->u = 1. - uv->u;

  if (dpdv)
    *dpdv = map->norm;
  if (dpdu)
    (void)VecNormCross(&map->norm, pos, dpdu);
}

void LinearMapping(Mapping *map, __attribute__((unused)) Geom *obj, Vector *pos,
                   __attribute__((unused)) Vector *norm, Vec2d *uv,
                   Vector *dpdu, Vector *dpdv) {
  Vector vtmp;

  vtmp = *pos;
  VecTransform(&vtmp, &map->m);
  uv->u = vtmp.x;
  uv->v = vtmp.y;

  if (dpdu) {
    *dpdu = map->uaxis;
  }
  if (dpdv) {
    *dpdv = map->vaxis;
  }
}
