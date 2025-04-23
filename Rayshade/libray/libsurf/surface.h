/*
 * surface.h
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
#ifndef SURFACE_H
#define SURFACE_H

#define DEFAULT_INDEX 1.0     /* Default index of refraction */
#define DEFAULT_PHONGPOW 15.0 /* Default specular highlight exp */

/*
 * Surface definition.
 */
typedef struct Surface {
  char *name;           /* Name */
  struct Color amb,     /* Ambient 'curve' */
      diff,             /* Diffuse reflection 'curve' */
      spec,             /* Specular reflection 'curve' */
      translu,          /* Diffuse transmission 'curve' */
      body;             /* Specular transmission 'curve' */
  Float srexp,          /* Specular reflection exponent */
      stexp,            /* Specular transmission exponent */
      statten,          /* Specular transmission attenuation */
      index,            /* Index of refraction */
      reflect,          /* Specular reflectivity */
      transp,           /* Specular transmittance */
      translucency;     /* Diffuse transmittance */
  char noshadow;        /* No shadowing? */
  struct Surface *next; /* Next surface in list (if any) */
} Surface;

/*
 * Linked list of (linked list of) surfaces.
 */
typedef struct SurfList {
  Surface *surf;
  struct SurfList *next;
} SurfList;

extern Surface *SurfaceCreate(), *GetShadingSurf(), /* application-provided */
    *SurfaceCopy(Surface *surf);

extern SurfList *SurfPop(SurfList *list), *SurfPush(Surface *surf, SurfList *list);

extern void SurfaceBlend(), ColorBlend();

extern int ComputeSurfProps();

extern void ColorBlend();

#endif /* SURFACE_H */
