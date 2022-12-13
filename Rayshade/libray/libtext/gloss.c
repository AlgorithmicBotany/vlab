/*
 * gloss.c
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
#include "texture.h"
#include "gloss.h"

Gloss *GlossCreate(glossiness) Float glossiness;
{
  Gloss *gloss;

  gloss = (Gloss *)Malloc(sizeof(Gloss));
  gloss->glossy = 1. - glossiness;
  return gloss;
}

void GlossApply(Gloss *gloss, __attribute__((unused)) Geom *prim, Ray *ray,
                __attribute__((unused)) Vector *pos, Vector *norm,
                __attribute__((unused)) Vector *gnorm,
                __attribute__((unused)) Surface *surf) {
  Vector uaxis, vaxis, point, norminc;
  extern void UnitCirclePoint();

  /*
   * Find coordinate system with norm as the Z axis.
   */
  VecCoordSys(norm, &uaxis, &vaxis);
  /*
   * Find point on unit circle based on sample #.
   */
  UnitCirclePoint(&point, ray->sample);
  /*
   * Perturb normal appropriately.
   */
  VecComb(gloss->glossy * point.x, uaxis, gloss->glossy * point.y, vaxis,
          &norminc);
  VecAdd(*norm, norminc, norm);
  /*
   * Renormalize.
   */
  (void)VecNormalize(norm);
}
