/*
 * jittered.c
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
#include "jittered.h"
#include "shadow.h"
static LightMethods *iJitteredMethods = NULL;

Jittered *JitteredCreate(pos, e1, e2) Vector *pos, *e1, *e2;
{
  Jittered *j;

  j = (Jittered *)share_malloc(sizeof(Jittered));

  j->pos = *pos;
  j->e1 = *e1;
  j->e2 = *e2;

  return j;
}

LightMethods *JitteredMethods() {
  if (iJitteredMethods == (LightMethods *)NULL) {
    iJitteredMethods = LightMethodsCreate();
    iJitteredMethods->intens = JitteredIntens;
    iJitteredMethods->dir = JitteredDirection;
  }
  return iJitteredMethods;
}

int JitteredIntens(__attribute__((unused)) Jittered *jit, Color *lcolor,
                   ShadowCache *cache, Ray *ray, Float dist, int noshadow,
                   Color *color) {
  return !Shadowed(color, lcolor, cache, ray, dist, noshadow);
}

void JitteredDirection(lp, pos, dir, dist) Jittered *lp;
Vector *pos, *dir;
Float *dist;
{
  /*
   * Choose a location with the area define by corner, e1
   * and e2 at which this sample will be taken.
   */
  VecAddScaled(lp->pos, nrand(), lp->e1, &lp->curpos);
  VecAddScaled(lp->curpos, nrand(), lp->e2, &lp->curpos);
  VecSub(lp->curpos, *pos, dir);
  *dist = VecNormalize(dir);
}

void JitteredMethodRegister(UserMethodType meth) {
  if (iJitteredMethods)
    iJitteredMethods->user = meth;
}
