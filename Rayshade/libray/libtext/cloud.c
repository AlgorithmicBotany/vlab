/*
 * cloud.c
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
#include "cloud.h"

/*
 * Gardner-style textured ellipsoid.  Designed to be used on unit spheres
 * centered at the origin.  (Of course, the spheres may be transformed
 * into ellipsoids, translated, etc.)
 */
CloudText *CloudTextCreate(scale, h, lambda, octaves, cthresh, lthresh,
                           transcale) Float scale,
    h, lambda, cthresh, lthresh, transcale;
int octaves;
{
  CloudText *cloud;

  cloud = (CloudText *)Malloc(sizeof(CloudText));
  cloud->beta = 1. + 2 * h;
  cloud->omega = pow(lambda, -0.5 * cloud->beta);
  cloud->lambda = lambda;
  cloud->scale = scale;
  cloud->cthresh = cthresh;
  cloud->range = lthresh - cthresh;
  cloud->transcale = transcale;
  cloud->maxval = 1. / (1. - cloud->beta);
  cloud->octaves = octaves;
  return cloud;
}

void CloudTextApply(void *p_cloud, __attribute__((unused)) Geom *prim,
                    Ray *ray, Vector *pos, __attribute__((unused)) Vector *norm,
                    __attribute__((unused)) Vector *gnorm, Surface *surf) {
  Ray pray;
  Float alpha, beta, It, dsquared, d, limb;
  CloudText *cloud = (CloudText *)p_cloud;
  /*
   * Transform ray to prim. space.
   */
  pray = *ray;
  (void)TextRayToPrim(&pray);
  dsquared = dotp(&pray.pos, &pray.pos);
  if (fabs(dsquared) < 1. + EPSILON) {
    surf->transp = 1.;
    surf->amb.r = surf->amb.g = surf->amb.b = 0.;
    surf->diff.r = surf->diff.g = surf->diff.b = 0.;
    return;
  }
  It = fBm(pos, cloud->omega, cloud->lambda, cloud->octaves);
  It = (cloud->maxval + It) * 0.5 / cloud->maxval;
  if (It < 0.)
    It = 0;
  else if (It > 1.)
    It = 1;
  d = sqrt(dsquared);
  beta = sqrt(dsquared - 1) / d;
  alpha = -dotp(&pray.pos, &pray.dir) / d;
  limb = (alpha - beta) / (1 - beta);
  /*
   * limb is 0 on the limb, 1 at the center, < 1 outside.
   */
  surf->transp = 1. - (It - cloud->cthresh - cloud->range * (1. - limb)) /
                          cloud->transcale;

  if (surf->transp > 1)
    surf->transp = 1.;
  if (surf->transp < 0)
    surf->transp = 0.;

  ColorScale((1. - surf->transp) * (1. - cloud->scale + cloud->scale * It),
             surf->diff, &surf->diff);
  ColorScale(1. - surf->transp, surf->amb, &surf->amb);
}
