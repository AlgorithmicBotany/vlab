/*
 * marble.c
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
#include "marble.h"

MarbleText *MarbleCreate(mapname) char *mapname;
{
  MarbleText *marble;

  marble = (MarbleText *)Malloc(sizeof(MarbleText));
  if (mapname)
    marble->colormap = ColormapRead(mapname);
  else
    marble->colormap = (Color *)NULL;
  return marble;
}

void
MarbleApply(
	 MarbleText *marble,
	 __attribute__((unused)) Geom *prim,
	 __attribute__((unused))  Ray *ray,
	 Vector *pos, __attribute__((unused)) Vector *norm, __attribute__((unused)) Vector *gnorm,
	 Surface *surf)
{
  Float val;
  int index;

  val = Marble(pos);
  if (marble->colormap) {
    index = (int)(255. * val);
    surf->diff.r *= marble->colormap[index].r;
    surf->diff.g *= marble->colormap[index].g;
    surf->diff.b *= marble->colormap[index].b;
    surf->amb.r *= marble->colormap[index].r;
    surf->amb.g *= marble->colormap[index].g;
    surf->amb.b *= marble->colormap[index].b;
  } else {
    ColorScale(val, surf->amb, &surf->amb);
    ColorScale(val, surf->diff, &surf->diff);
  }
}
