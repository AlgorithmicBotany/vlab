/*
 * surfdef.c
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
#include "rayshade.h"
#include "libsurf/surface.h"

static Surface *Surfaces; /* Named surfaces */

Surface DefaultSurface = {
    "DeFault",       /* name */
    {0.1, 0.1, 0.1}, /* ambient */
    {0.6, 0.6, 0.6}, /* diffuse */
    {0.5, 0.5, 0.5}, /* specular */
    {0.0, 0.0, 0.0}, /* Diffuse transmission 'curve' */
    {1.0, 1.0, 1.0}, /* Specular transmission 'curve' */
    12.,             /* reflected Phong coef */
    12.,             /* transmitted Phong coef */
    1.,              /* spec. transmitted attenuation */
    DEFAULT_INDEX,   /* index of refr */
    0.,              /* reflectivity */
    0.,              /* transparency */
    0.,              /* translucency */
    FALSE,           /* noshadow */
    NULL,            /* next */
};

Surface *SurfaceGetNamed(char *name), *SurfaceFetchNamed(char *name);

/*
 * Add surf to the list of defined surfaces.
 */
void SurfaceAddToDefined(surf) Surface *surf;
{
  /*
   * Make sure index of refraction isn't bogus.
   */
  if (surf->transp > EPSILON && surf->index <= 0.)
    RLerror(RL_PANIC, "Index of refraction must be positive.\n");

  if (surf->name == (char *)NULL || *surf->name == (char)NULL)
    RLerror(RL_PANIC, "Surface with NULL name defined.\n");

  if (SurfaceFetchNamed(surf->name) != (Surface *)NULL)
    RLerror(RL_WARN, "Redefinition of \"%s\" surface.", surf->name);

  surf->next = Surfaces;
  Surfaces = surf;
}

/*
 * Search for surface with given name.  If not found, complain and exit.
 */
Surface *SurfaceGetNamed(name) char *name;
{
  Surface *stmp;

  stmp = SurfaceFetchNamed(name);
  if (stmp == (Surface *)NULL)
    RLerror(RL_PANIC, "Undefined surface \"%s\".", name);

  return stmp;
}

/*
 * Return pointer to surface with given name, NULL if no such surface.
 */
Surface *SurfaceFetchNamed(name) char *name;
{
  Surface *stmp;

  for (stmp = Surfaces; stmp; stmp = stmp->next)
    if (strcmp(name, stmp->name) == 0)
      return stmp;
  /*
   * No surface named "name".
   */
  return (Surface *)NULL;
}

/*
 * Traverse the given hitlist to find the "bottom-most" surface.
 * If no surface is found, use the default.
 */
Surface *GetShadingSurf(hitlist) HitList *hitlist;
{
  int i;

  /*
   * -1 here because the World always has a NULL surface
   * (DefaultSurf is used instead)
   */
  for (i = 0; i < hitlist->nodes - 1; i++) {
    if (hitlist->data[i].obj->surf)
      return hitlist->data[i].obj->surf;
  }
  /*
   * No suface found -- use the default.
   */
  return &DefaultSurface;
}
