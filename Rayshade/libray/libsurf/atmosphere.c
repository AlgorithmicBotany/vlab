/*
 * atmosphere.c
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

Atmosphere *AtmosCreate(data, method) char *data;
void (*method)(void *fog, Ray *ray, Vector *pos, Float dist, Color *color);
{
  Atmosphere *ef;

  ef = (Atmosphere *)Malloc(sizeof(Atmosphere));
  ef->data = data;
  ef->method = method;
  ef->next = (Atmosphere *)0;
  return ef;
}

Atmosphere *AtmosphereCopy(atmos) Atmosphere *atmos;
{
  Atmosphere *res;

  if (atmos == (Atmosphere *)NULL)
    return (Atmosphere *)NULL;
  res = AtmosCreate(atmos->data, atmos->method);
  res->next = AtmosphereCopy(atmos->next);
  return res;
}

Medium *MediumPush(index, statten, media) Float index, statten;
Medium *media;
{
  Medium *new;

  new = (Medium *)Malloc(sizeof(Medium));
  new->index = index;
  new->statten = statten;
  new->next = media;

  return new;
}

void Atmospherics(effects, ray, dist, pos, color) Atmosphere *effects;
Ray *ray;
Float dist;
Vector *pos;
Color *color;
{
  Atmosphere *etmp;

  for (etmp = effects; etmp; etmp = etmp->next)
    (*etmp->method)(etmp->data, ray, pos, dist, color);
}

Float ExpAtten(dist, trans) Float dist, trans;
{
  Float atten;

  if (trans < EPSILON)
    return 0.;
  atten = LNHALF * dist / trans;
  return (atten < -10. ? 0. : exp(atten));
}
