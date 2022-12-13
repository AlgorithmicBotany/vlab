/*
 * light.c
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

Light *LightCreate(light, meth, color) LightRef light;
LightMethods *meth;
Color *color;
{
  Light *ltmp;

  if (light == (LightRef)NULL || meth == (LightMethods *)NULL)
    return (Light *)NULL;

  ltmp = (Light *)share_malloc(sizeof(Light));
  ltmp->light = light;
  ltmp->methods = meth;
  ltmp->color = *color;
  ltmp->next = (Light *)NULL;
  ltmp->cache = (ShadowCache *)NULL;
  ltmp->shadow = TRUE;
  return ltmp;
}

LightMethods *LightMethodsCreate() {
  return (LightMethods *)share_calloc(1, sizeof(LightMethods));
}

/*
 * Compute light color.  Returns FALSE if in full shadow, TRUE otherwise.
 * Computed light color is stored in 'color'.
 */
int LightIntens(lp, ray, dist, noshadow, color) Light *lp;
Ray *ray;
Float dist;
int noshadow;
Color *color;
{
  if (lp->methods->intens)
    return (*lp->methods->intens)(lp->light, &lp->color, lp->cache, ray, dist,
                                  noshadow || !lp->shadow, color);
  RLerror(RL_ABORT, "Cannot compute light intensity!\n");
  return FALSE;
}

/*
 * Calculate ray and distance from position to light.
 */
int LightDirection(lp, objpos, lray, dist) Light *lp;
Vector *objpos, *lray;
Float *dist;
{
  if (lp->methods->dir) {
    (*lp->methods->dir)(lp->light, objpos, lray, dist);
    return TRUE;
  } else {
    RLerror(RL_ABORT, "Cannot compute light direction!\n");
    return FALSE;
  }
}
