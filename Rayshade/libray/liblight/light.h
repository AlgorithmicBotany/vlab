/*
 * light.h
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
#ifndef LIGHT_H
#define LIGHT_H

#include "libobj/geom.h"

#define SHADOW_NONE 001
#define SHADOW_TRANSP 002
#define SHADOW_CSG 004
#define SHADOW_CACHE 010
#define SHADOW_BLUR 020

#define NOSHADOWS(f) ((f)&SHADOW_NONE)
#define SHADOWTRANSP(f) ((f)&SHADOW_TRANSP)
#define SHADOWCSG(f) ((f)&SHADOW_CSG)
#define SHADOWCACHE(f) ((f)&SHADOW_CACHE)
#define SHADOWBLUR(f) ((f)&SHADOW_BLUR)

#define SHADOW_EPSILON (4. * EPSILON)

typedef char *LightRef;

typedef struct {
  struct Geom *obj; /* Pointer to cached object */
  RSMatrix trans;   /* World-to-object transformation */
  char dotrans;     /* TRUE if above trans is non-identity */
} ShadowCache;

typedef struct {
  int (*intens)(); /* intensity method */
  void (*dir)(),   /* direction method */
      (*user)();   /* user-defined method */
} LightMethods;

typedef struct Light {
  Color color;           /* Light source color & intensity */
  int shadow;            /* Does light source cast shadows? */
  LightRef light;        /* Pointer to light information */
  LightMethods *methods; /* Light source methods */
  ShadowCache *cache;    /* Shadow cache, if any */
  struct Light *next;    /* Next light in list */
} Light;

extern LightMethods *LightMethodsCreate();
extern Light *LightCreate();
extern void LightAllocateCache(), LightAddToDefined();
extern int LightIntens(), LightDirection();
extern void ShadowSetOptions(), ShadowStats();

#endif /* LIGHT_H */
