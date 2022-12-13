/*
 * sampling.h
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
 */
#ifndef SAMPLING_H
#define SAMPLING_H

/*
 * # of subsamples to take within each of the totsamples slots when
 * computing 'animated' bounding boxes.
 */
#define TIME_SUB_SAMPLES 10

typedef struct SampleInfo {
  int totsamples,  /* # of samples/pixel */
      sidesamples, /* sqrt(samples) */
      gaussian,    /* gaussian filter? */
      framenum,    /* current frame numer */
      timemagic;   /* # of time samples per screen... */
  Float weight,    /* 1. / totsamples */
      spacing,     /* 1. / sidesamples  */
      filterwidth, /* total width of filter */
      filterdelta, /* filterwidth * spacing */
      **filter,    /* pixel filter, sidesamp by sidesamp */
      starttime,   /* start time */
      shutter;     /* length of time shutter is open */
} SampleInfo;

extern SampleInfo Sampling;

extern void SamplingSetOptions(), UnitCirclePoint();
extern void SamplingSetTime(Float starttime, Float shutter, int frame);
#endif /* SAMPLING */
