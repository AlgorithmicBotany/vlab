/*
 * blotch.c
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
#include "blotch.h"

/*
 * Create and return a reference to a "blotch" texture.
 */
Blotch *BlotchCreate(mix, surf) Float mix;
Surface *surf;
{
  Blotch *blotch;

  blotch = (Blotch *)Malloc(sizeof(Blotch));
  blotch->mix = mix;
  blotch->surf = surf;
  return blotch;
}

/*
 * Apply "blotch" texture.
 */
/*ARGSUSED*/
void BlotchApply(Blotch *blotch, __attribute__((unused)) Geom *prim,
                 __attribute__((unused)) Ray *ray, Vector *pos,
                 __attribute__((unused)) Vector *norm,
                 __attribute__((unused)) Vector *gnorm, Surface *surf) {
  Float val;

  /*
   * "mix" represents the 'average' noise value at a point.
   */
  val = Noise3(pos);
  if (val > blotch->mix) {
    val = (val - blotch->mix) / (1. - blotch->mix);
    SurfaceBlend(surf, blotch->surf, 1. - val, val);
  }
}
