/*
 * surface.c
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
#include "atmosphere.h"
#include "surface.h"

#define blend(a, b, p, q) (a * p + b * q)

Color Black = {0., 0., 0.}, White = {1., 1., 1.};

/*
 * Create and return pointer to surface with given properties.
 */
Surface *SurfaceCreate() {
  Surface *stmp;

  stmp = (Surface *)Malloc(sizeof(Surface));

  stmp->amb = stmp->diff = stmp->spec = stmp->translu = Black;

  stmp->body = White;

  stmp->srexp = stmp->stexp = DEFAULT_PHONGPOW;
  stmp->statten = 1.; /* No attenuation by default */

  stmp->reflect = stmp->transp = 0.;

  stmp->noshadow = FALSE;

  stmp->index = DEFAULT_INDEX;

  stmp->name = (char *)NULL;
  stmp->next = (Surface *)NULL;

  return stmp;
}

Surface *SurfaceCopy(surf) Surface *surf;
{
  Surface *res;

  if (!surf)
    return (Surface *)NULL;

  res = SurfaceCreate();
  *res = *surf;
  res->next = (Surface *)NULL;
  res->name = (char *)NULL;
  return res;
}

/*
 * Compute combination of two surfaces. Resulting surface is copied into surf1.
 */
void SurfaceBlend(surf1, surf2, p, q) Surface *surf1, *surf2;
Float p, q;
{
  /*
   * P is weight of surf1.  q is weight of surf2.
   * Result is placed in surf1.
   */
  if (q < EPSILON)
    return; /* keep surf1 as-is */

  ColorBlend(&surf1->amb, &surf2->amb, p, q);
  ColorBlend(&surf1->diff, &surf2->diff, p, q);
  ColorBlend(&surf1->spec, &surf2->spec, p, q);
  ColorBlend(&surf1->translu, &surf2->translu, p, q);
  ColorBlend(&surf1->body, &surf2->body, p, q);

  surf1->srexp = blend(surf1->srexp, surf2->srexp, p, q);
  surf1->stexp = blend(surf1->stexp, surf2->stexp, p, q);

  surf1->reflect = blend(surf1->reflect, surf2->reflect, p, q);
  surf1->transp = blend(surf1->transp, surf2->transp, p, q);
  surf1->translucency = blend(surf1->translucency, surf2->translucency, p, q);
  /*
   * Questionable...
   */
  surf1->statten = blend(surf1->statten, surf2->statten, p, q);
  surf1->index = blend(surf1->index, surf2->index, p, q);

  if (p < EPSILON) {
    surf1->noshadow = surf2->noshadow;
  } else {
    /* else there's a blend of some kind... */
    surf1->noshadow = (surf1->noshadow && surf2->noshadow);
  }
}

/*
 * Blend two colors.  Result is placed in color1.
 */
void ColorBlend(color1, color2, p, q) Color *color1, *color2;
Float p, q;
{
  color1->r = blend(color1->r, color2->r, p, q);
  color1->g = blend(color1->g, color2->g, p, q);
  color1->b = blend(color1->b, color2->b, p, q);
}

SurfList *SurfPop(list) SurfList *list;
{
  SurfList *stmp = list->next;

  free((voidstar)list);
  return stmp;
}

SurfList *SurfPush(surf, list) Surface *surf;
SurfList *list;
{
  SurfList *stmp;

  stmp = (SurfList *)Malloc(sizeof(SurfList));
  stmp->surf = surf;
  stmp->next = list;
  return stmp;
}
