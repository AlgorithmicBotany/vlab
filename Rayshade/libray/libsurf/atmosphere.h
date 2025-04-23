/*
 * atmosphere.h
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
#ifndef ATMOSPHERE_H
#define ATMOSPHERE_H

#include "libcommon/common.h"

typedef char *AtmosRef;

typedef struct Medium {
  Float index, /* Index of refraction */
      statten; /* specular transmission attenuation */
  struct Medium *next;
} Medium;

typedef struct Atmosphere {
  AtmosRef data;           /* Effect info */
  void (*method)();        /* Atmosphere method */
  struct Atmosphere *next; /* Next effect */
} Atmosphere;

extern Medium *MediumPush();
extern Atmosphere *AtmosCreate(char *data, void (*method)(void *fog, Ray *ray, Vector *pos, Float dist, Color *color)), *AtmosphereCopy();
extern void Atmospherics();

#endif /* ATMOSPHERE_H */
