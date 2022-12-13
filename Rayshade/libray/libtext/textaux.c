/*
 * textaux.c
 *
 * Copyright (C) 1989, 1991, Craig E. Kolb, Rod G. Bogart, Robert F. Skinner
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
#include "textaux.h"
Color *ColormapRead(filename) char *filename;
{
  FILE *fp;
  Color *map;
  char buf[BUFSIZ];
  int r, g, b, i, num;

  fp = fopen(filename, "r");
  if (fp == (FILE *)NULL) {
    RLerror(RL_ABORT, "Cannot open colormap file \"%s\".\n", filename);
    return (Color *)NULL;
  }

  map = (Color *)Calloc(256, sizeof(Color));

  for (i = 0; fgets(buf, BUFSIZ, fp) != NULL && i < 256; i++) {
    num = sscanf(buf, "%d %d %d", &r, &g, &b);
    if (num != 3) {
      RLerror(RL_ABORT, "%s, line %d:  Bad color specification.\n", filename,
              i + 1);
      return (Color *)NULL;
    }
    map[i].r = (Float)r;
    map[i].g = (Float)g;
    map[i].b = (Float)b;
    ColorScale(1. / 255., map[i], &map[i]);
  }
  (void)fclose(fp);
  return map;
}

Float Marble(vec) Vector *vec;
{
  Float i;

  i = sin(8. * Chaos(vec, 6) + 7. * vec->z) + 1;

  return pow(0.5 * i, 0.77);
}

Float PAChaos(vec, octaves) Vector *vec;
int octaves;
{
  Float s, t, tmp;
  Vector tp;

  s = 1.0;
  t = 0.;
  tp = *vec;

  while (octaves--) {
    tmp = Noise3(&tp) * s;
    t += fabs(tmp);
    s *= 0.5;
    tp.x *= 2.;
    tp.y *= 2.;
    tp.z *= 2.;
  }

  return t;
}

Float Chaos(vec, octaves) Vector *vec;
int octaves;
{
  Float s, t;
  Vector tp;

  s = 1.0;
  t = 0.;
  tp = *vec;

  while (octaves--) {
    t += Noise3(&tp) * s;
    s *= 0.5;
    tp.x *= 2.;
    tp.y *= 2.;
    tp.z *= 2.;
  }

  return t;
}

void VfBm(vec, omega, lambda, octaves, ans) Vector *vec, *ans;
Float omega, lambda;
int octaves;
{
  Float o;
  Vector tp, n;

  ans->x = ans->y = ans->z = 0.;
  tp = *vec;
  o = 1.;

  while (octaves--) {
    DNoise3(&tp, &n);
    ans->x += o * n.x;
    ans->y += o * n.y;
    ans->z += o * n.z;
    o *= omega;
    if (o < EPSILON)
      break;
    tp.x *= lambda;
    tp.y *= lambda;
    tp.z *= lambda;
  }
}

Float fBm(vec, omega, lambda, octaves) register Vector *vec;
Float omega, lambda;
int octaves;
{
  Float a, o;
  Vector tp;

  a = 0;
  o = 1.;
  tp = *vec;
  while (octaves--) {
    a += o * Noise3(&tp);
    tp.x *= lambda;
    tp.y *= lambda;
    tp.z *= lambda;
    o *= omega;
  }
  return a;
}

int TileValue(tileu, tilev, u, v) Float tileu, tilev, u, v;
{
  /*
   * If both tileu and tilev are zero, the pattern is repeated infinitly.
   *   XXXXXX
   *   XXXXXX   tileu=0  tilev=0
   *   XXXXXX
   *   XXXXXX
   * If one is positive and the other is zero, the pattern is infinite
   * in one direction and limited in the other.
   *   ++++++
   *   XXXXXX   tileu=0  tilev=1
   *   ++++++
   *   ++++++
   * If both are positive, the pattern is limited in both directions.
   *   ++++++
   *   +++XX+   tileu=2  tilev=1
   *   ++++++
   *   ++++++
   * If one is negative and the other is zero, the pattern is the
   * inverse of the positive/zero case.
   *   XXXXXX
   *   ++++++   tileu=0  tilev=-1
   *   XXXXXX
   *   XXXXXX
   * If one is negative and the other is positive, the pattern is like
   * negative/zero, but limited in one direction.
   *   +++XX+
   *   ++++++   tileu=2  tilev=-1
   *   +++XX+
   *   +++XX+
   * If both are negative, the pattern is the inverse of the
   * positive/positive case (a rectangular hole).
   *   XXXXXX
   *   XXX++X   tileu=-2  tilev=-1
   *   XXXXXX
   *   XXXXXX
   */
  if ((tileu < 0.001) && (tileu > -0.001) && (tilev < 0.001) &&
      (tilev > -0.001))
    /* zero/zero */
    return FALSE;
  if ((tileu < -0.001) && (tilev < -0.001) &&
      ((u > -tileu) || (u < 0) || (v > -tilev) || (v < 0)))
    /* negative/negative */
    return FALSE;
  if ((tileu > 0.001) && ((u > tileu) || (u < 0)))
    /* positive/whatever */
    return TRUE;
  if ((tilev > 0.001) && ((v > tilev) || (v < 0)))
    /* whatever/positive */
    return TRUE;
  if ((tileu < -0.001) && (u < -tileu) && (u > 0))
    /* negative/whatever */
    return TRUE;
  if ((tilev < -0.001) && (v < -tilev) && (v > 0))
    /* whatever/negative */
    return TRUE;

  return FALSE;
}

void Wrinkled(pos, lambda, omega, octaves, res) Vector *pos, *res;
Float lambda, omega;
int octaves;
{
  Float s;
  Vector point, tmp;

  res->x = res->y = res->z = 0.;
  s = 1.;
  point = *pos;
  while (octaves--) {
    DNoise3(&point, &tmp);
    point.x *= lambda;
    point.y *= lambda;
    point.z *= lambda;
    res->x += tmp.x * s;
    res->y += tmp.y * s;
    res->z += tmp.z * s;
    s *= omega;
  }
}

void Windy(Vector *pos,
  Float windscale, Float chaoscale, Float bumpscale, int octaves, Float tscale, Float hscale, Float offset, Vector *res)
{
  Vector spoint, tmp;
  Float windfield, f, scalar;

  spoint = *pos;
  spoint.x *= windscale;
  spoint.y *= windscale;
  spoint.z *= windscale;
  if (chaoscale)
    windfield = chaoscale * Chaos(&spoint, 7);
  else
    windfield = 1.;

  DNoise3(pos, &tmp);
  res->x = bumpscale * tmp.x;
  res->y = bumpscale * tmp.y;
  res->z = bumpscale * tmp.z;

  f = 1.;
  scalar = windfield;
  while (octaves--) {
    f *= tscale;
    spoint.x = f * pos->x;
    spoint.y = f * pos->y;
    spoint.z = f * pos->z;
    DNoise3(&spoint, &tmp);
    res->x += scalar * tmp.x;
    res->y += scalar * tmp.y;
    res->z += scalar * tmp.z;
    scalar *= hscale;
  }
  res->x *= windfield + offset;
  res->y *= windfield + offset;
  res->z *= windfield + offset;
}
