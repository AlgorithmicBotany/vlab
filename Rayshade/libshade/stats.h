/*
 * stats.h
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
#ifndef STATS_H
#define STATS_H

/*
 * Statistics
 */
typedef struct RSStats {
  unsigned long EyeRays, /* # of eye rays spawned */
      ShadowRays,        /* # of shadow rays spawned */
      ReflectRays,       /* # of reflected rays */
      RefractRays,       /* # of refracted rays */
      HitRays,           /* # of rays that hit something. */
      BVTests,           /* # of bounding volume tests. */
      SuperSampled,      /* # of supersampled pixels. */
      ShadowHits,        /* # of shadow ray hits */
      CacheHits,         /* # of shadow cache hits */
      CacheMisses;       /* # of shadow cache misses */
  Float Utime,           /* User time */
      Stime;             /* System time */
  FILE *fstats;          /* Stats/info file pointer. */
} RSStats;

extern RSStats Stats;
extern void StatsPrint(), StatsAddRep();

#endif /* STATS_H */
