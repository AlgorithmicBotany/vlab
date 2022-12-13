/*
 * windy.c
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
#include "windy.h"
#include "textaux.h"

/*
 * Create and return a reference to a "windy" texture.
 */
WindyText *WindyCreate(scale, wscale, cscale, bscale, octaves, tscale, hscale,
                       offset) Float scale,
    wscale, cscale, bscale, tscale, hscale, offset;
int octaves;
{
  WindyText *windy;

  windy = (WindyText *)Malloc(sizeof(WindyText));
  windy->scale = scale;
  windy->windscale = wscale;
  windy->chaoscale = cscale;
  windy->bumpscale = bscale;
  windy->tscale = tscale;
  windy->hscale = hscale;
  windy->offset = offset;
  windy->octaves = octaves;
  return windy;
}

/*
 * Apply a "windy" texture.
 */
void
WindyApply(
	 WindyText *windy,
	 __attribute__((unused))  Geom *prim,
	 __attribute__((unused))  Ray *ray,
	 Vector *pos, Vector *norm, __attribute__((unused)) Vector *gnorm,
	 __attribute__((unused))  Surface *surf)
{
  Vector bump;

  Windy(pos, windy->windscale, windy->chaoscale, windy->bumpscale,
        windy->octaves, windy->tscale, windy->hscale, windy->offset, &bump);

  norm->x += windy->scale * bump.x;
  norm->y += windy->scale * bump.y;
  norm->z += windy->scale * bump.z;
  VecNormalize(norm);
}
