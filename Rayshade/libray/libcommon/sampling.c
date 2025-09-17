/*
 * sampling.c
 *
 * Copyright (C) 1989, 1991, Craig E. Kolb, Rod G. Bogart
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
 */
#include "common.h"
#include "sampling.h"

SampleInfo Sampling; /* sampling information */

/*
 * Set sampling options.
 */
void SamplingSetOptions(n, gaussian, width) int n, gaussian;
Float width;
{
  Float norm, u, v;
  int x, y;

  Sampling.sidesamples = n;
  Sampling.totsamples = n * n;
  Sampling.weight = 1. / (Float)Sampling.totsamples;
  Sampling.spacing = 1. / (Float)Sampling.sidesamples;
  Sampling.filterwidth = width;
  Sampling.filterdelta = Sampling.filterwidth * Sampling.spacing;
  Sampling.gaussian = gaussian;

  Sampling.filter = (Float **)Malloc(Sampling.sidesamples * sizeof(Float *));
  for (y = 0; y < Sampling.sidesamples; y++) {
    Sampling.filter[y] = (Float *)Malloc(Sampling.sidesamples * sizeof(Float));
  }
  if (Sampling.gaussian) {
    norm = 0.;
    u = -0.5 * Sampling.filterwidth +
        0.5 * Sampling.filterwidth * Sampling.spacing;
    for (x = 0; x < Sampling.sidesamples; x++) {
      v = -0.5 * Sampling.filterwidth +
          0.5 * Sampling.filterwidth * Sampling.spacing;
      for (y = 0; y < Sampling.sidesamples; y++) {
        Sampling.filter[x][y] = exp(-0.5 * (u * u + v * v));
        norm += Sampling.filter[x][y];
        v += Sampling.spacing * Sampling.filterwidth;
      }
      u += Sampling.spacing * Sampling.filterwidth;
    }

    for (x = 0; x < Sampling.sidesamples; x++)
      for (y = 0; y < Sampling.sidesamples; y++)
        Sampling.filter[x][y] /= norm;
  } else {
    /* Box filter.  Yawn. */
    for (x = 0; x < Sampling.sidesamples; x++)
      for (y = 0; y < Sampling.sidesamples; y++)
        Sampling.filter[x][y] = Sampling.weight;
  }
}

/*
 * Set start time and duration of frame.
 */
void SamplingSetTime(starttime, shutter, frame) Float starttime, shutter;
int frame;
{
  Sampling.starttime = starttime;
  Sampling.shutter = shutter;
  Sampling.framenum = frame;
  TimeSet(Sampling.starttime);
  FrameSet((Float)frame);
}

/*
 * Find a point on a unit circle that is separated from other random
 * points by some jitter spacing.
 *
 * It should do the above, but the temporary hack below just finds a
 * jittered point in a unit square.
 */
void UnitCirclePoint(pnt, sample) Vector *pnt;
int sample;
{
  /*
   * This picks a random point on a -1 to 1 square.  The jitter stuff
   * is correct enough to avoid excessive noise.  An extremely blurry
   * bright highlight will look squarish, not roundish.  Sorry.
   */
  Float jit;

  if (sample >= 0) {
    jit = 2. * Sampling.spacing;

    pnt->x = nrand() * jit - 1.0 + (sample % Sampling.sidesamples) * jit;
    pnt->y = nrand() * jit - 1.0 + (sample / Sampling.sidesamples) * jit;
    pnt->z = 0.0;
  } else {
    pnt->x = nrand() * 2.0 - 1.0;
    pnt->y = nrand() * 2.0 - 1.0;
    pnt->z = 0.0;
  }
}
