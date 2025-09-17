/*
 * fogdeck.c
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
#include "fogdeck.h"
#include "surface.h"

Fogdeck *FogdeckCreate(alt, offset, scale, chaoscale, octaves, color,
                       trans) Float alt,
    offset, chaoscale;
Vector *scale;
int octaves;
Color *color, *trans;
{
  Fogdeck *fogdeck;
  /*	static void ComputeFogdeck();*/

  fogdeck = (Fogdeck *)Malloc(sizeof(Fogdeck));

  fogdeck->alt = alt;
  fogdeck->octaves = octaves;
  fogdeck->scale = *scale;
  fogdeck->chaoscale = chaoscale;
  fogdeck->offset = offset;

  if (color == (Color *)NULL)
    fogdeck->color.r = fogdeck->color.g = fogdeck->color.b = 0.;
  else
    fogdeck->color = *color;
  if (trans == (Color *)NULL)
    fogdeck->trans.r = fogdeck->trans.g = fogdeck->trans.b = FAR_AWAY;
  else {
    fogdeck->trans = *trans;
  }
  return fogdeck;
}

/*
 * Add fogdeck to the given color.
 */
void FogdeckApply(void *p_fogdeck, Ray *ray,
                  __attribute__((unused)) Vector *pos, Float dist,
                  Color *color) {
  Float atten, hitdist, density;
  Color trans;
  Vector endp;
  extern Float ExpAtten(), PAChaos();
  Fogdeck *fogdeck = (Fogdeck *)p_fogdeck;
  /*
   * Find distance from origin at which ray strikes
   * z = fogdeck->alt plane
   */
  if (fabs(ray->dir.z) < EPSILON)
    return;
  hitdist = (fogdeck->alt - ray->pos.z) / ray->dir.z;
  if (hitdist < EPSILON || hitdist > dist)
    return;
  /*
   * Compute ray endpoint
   */
  VecAddScaled(ray->pos, hitdist, ray->dir, &endp);

  /*
   * Modify transmissivity based on point of
   * intersection.
   */
  endp.x *= fogdeck->scale.x;
  endp.y *= fogdeck->scale.y;
  endp.z *= fogdeck->scale.z;

  density =
      fogdeck->offset + fogdeck->chaoscale * PAChaos(&endp, fogdeck->octaves);
  if (density < EPSILON)
    density = HUGE;
  else
    density = 1. / density;

  trans = fogdeck->trans;
  ColorScale(density, trans, &trans);

  dist -= hitdist;

  atten = ExpAtten(dist, trans.r);

  if (trans.r == trans.g && trans.r == trans.b) {
    ColorBlend(color, &fogdeck->color, atten, 1. - atten);
    return;
  }
  color->r = atten * color->r + (1. - atten) * fogdeck->color.r;

  atten = ExpAtten(dist, trans.g);
  color->g = atten * color->g + (1. - atten) * fogdeck->color.g;
  atten = ExpAtten(dist, trans.b);
  color->b = atten * color->b + (1. - atten) * fogdeck->color.b;
}
