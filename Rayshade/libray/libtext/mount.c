/*
 * mount.c
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
#include "mount.h"

/*
 * Create and return a reference to a "mount" texture.
 */
Mount *MountCreate(cmap, turb, slope) char *cmap;
Float turb, slope;
{
  Mount *mount;

  mount = (Mount *)Malloc(sizeof(Mount));
  mount->turb = turb;
  mount->slope = slope;
  mount->cmap = ColormapRead(cmap);
  return mount;
}

/*
 * Apply a "mount" texture.
 */
void
MountApply(
	 Mount *mount,
	 __attribute__((unused))Geom *prim,
	 __attribute__((unused))Ray *ray,
	 Vector *pos, Vector *norm, __attribute__((unused))Vector *gnorm,
	 Surface *surf)
{
  int index;
  Float t;

  t = Chaos(pos, 7);
  index = (pos->z + mount->turb * t - mount->slope * (1. - norm->z)) * 256;
  if (index < 0)
    index = 0;
  if (index > 255)
    index = 255;

  ColorMultiply(surf->amb, mount->cmap[index], &surf->amb);
  ColorMultiply(surf->diff, mount->cmap[index], &surf->diff);
  ColorMultiply(surf->spec, mount->cmap[index], &surf->spec);
}
