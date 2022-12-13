/*
 * extended.c
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
#include "libcommon/sampling.h"
#include "extended.h"
#include "shadow.h"

static LightMethods *iExtendedMethods = NULL;

Extended *ExtendedCreate(r, pos) Float r;
Vector *pos;
{
  Extended *e;

  e = (Extended *)share_malloc(sizeof(Extended));
  e->pos = *pos;
  e->radius = r;

  return e;
}

LightMethods *ExtendedMethods() {
  if (iExtendedMethods == (LightMethods *)NULL) {
    iExtendedMethods = LightMethodsCreate();
    iExtendedMethods->intens = ExtendedIntens;
    iExtendedMethods->dir = ExtendedDirection;
  }
  return iExtendedMethods;
}

/*
 * Compute intensity ('color') of extended light source 'lp' from 'pos'.
 */
/*static*/ int ExtendedIntens(lp, lcolor, cache, ray, noshadow,
                              color) Extended *lp;
Color *lcolor, *color;
ShadowCache *cache;
Ray *ray;
int noshadow;
{
  Float jit, vpos, upos, lightdist;
  Ray newray;
  Vector Uaxis, Vaxis, ldir;

  if (noshadow) {
    *color = *lcolor;
    return TRUE;
  }

  newray = *ray;
  /*
   * Determinte two orthoganal vectors that lay in the plane
   * whose normal is defined by the vector from the center
   * of the light source to the point of intersection and
   * passes through the center of the light source.
   */
  VecSub(lp->pos, ray->pos, &ldir);
  VecCoordSys(&ldir, &Uaxis, &Vaxis);

  jit = 2. * lp->radius * Sampling.spacing;

  /*
   * Sample a single point, determined by SampleNumber,
   * on the extended source.
   */
  vpos = -lp->radius + (ray->sample % Sampling.sidesamples) * jit;
  upos = -lp->radius + (ray->sample / Sampling.sidesamples) * jit;
  vpos += nrand() * jit;
  upos += nrand() * jit;
  VecComb(upos, Uaxis, vpos, Vaxis, &newray.dir);
  VecAdd(ldir, newray.dir, &newray.dir);
  lightdist = VecNormalize(&newray.dir);

  return !Shadowed(color, lcolor, cache, &newray, lightdist, noshadow);
}

void ExtendedDirection(lp, pos, dir, dist) Extended *lp;
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

void ExtendedMethodRegister(meth) UserMethodType meth;
{
  if (iExtendedMethods)
    iExtendedMethods->user = meth;
}
