/*
 * infinite.c
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
#include "infinite.h"
#include "shadow.h"

static LightMethods *iInfMethods = NULL;

Infinite *InfiniteCreate(dir) Vector *dir;
{
  Infinite *inf;

  inf = (Infinite *)share_malloc(sizeof(Infinite));
  inf->dir = *dir;
  if (VecNormalize(&inf->dir) == 0.) {
    RLerror(RL_ABORT, "Invalid directional light.\n");
    return (Infinite *)NULL;
  }
  return inf;
}

LightMethods *InfiniteMethods() {
  if (iInfMethods == (LightMethods *)NULL) {
    iInfMethods = LightMethodsCreate();
    iInfMethods->intens = InfiniteIntens;
    iInfMethods->dir = InfiniteDirection;
  }
  return iInfMethods;
}

int InfiniteIntens(__attribute__((unused)) Infinite *inf, Color *lcolor,
                   ShadowCache *cache, Ray *ray, Float dist, int noshadow,
                   Color *color) {
  return !Shadowed(color, lcolor, cache, ray, dist, noshadow);
}

void InfiniteDirection(Infinite *lp, __attribute__((unused)) Vector *pos,
                       Vector *dir, Float *dist) {
  *dir = lp->dir;
  *dist = FAR_AWAY;
}

void InfiniteMethodRegister(meth) UserMethodType meth;
{
  if (iInfMethods)
    iInfMethods->user = meth;
}
