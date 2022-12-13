/*
 * point.c
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
#include "light.h"
#include "point.h"
#include "shadow.h"
static LightMethods *iPointMethods = NULL;

Pointlight *PointCreate(pos) Vector *pos;
{
  Pointlight *p;

  p = (Pointlight *)share_malloc(sizeof(Pointlight));
  p->pos = *pos;
  return p;
}

LightMethods *PointMethods() {
  if (iPointMethods == (LightMethods *)NULL) {
    iPointMethods = LightMethodsCreate();
    iPointMethods->intens = PointIntens;
    iPointMethods->dir = PointDirection;
  }
  return iPointMethods;
}

int PointIntens(__attribute__((unused)) Pointlight *lp, Color *lcolor,
                ShadowCache *cache, Ray *ray, Float dist, int noshadow,
                Color *color) {
  return !Shadowed(color, lcolor, cache, ray, dist, noshadow);
}

void PointDirection(lp, pos, dir, dist) Pointlight *lp;
Vector *pos, *dir;
Float *dist;
{
  /*
   * Calculate dir from position to center of
   * light source.
   */
  VecSub(lp->pos, *pos, dir);
  *dist = VecNormalize(dir);
}

void PointMethodRegister(meth) UserMethodType meth;
{
  if (iPointMethods)
    iPointMethods->user = meth;
}
