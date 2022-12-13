/*
 * main.c
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
 *
 */

char rcsid[] = "$Id: main.c,v 1.1.1.1 2005/01/07 20:51:06 laneb Exp $";

#include "rayshade.h"
#include "options.h"
#include "stats.h"
#include "viewing.h"
#include "picture.h"
#include "misc.h"
#include "render.h"
#include "setup.h"
#include "libcommon/sampling.h"

// [PASCAL] cf comments at the bottom, following functions are implemented in libshade
//static void RSInitialize(), RSStartFrame();

int main(argc, argv) int argc;
char **argv;
{
  Float utime, stime;

  /*
   * Initialize variables, etc.
   */
  RSInitialize(argc, argv);
  RSStartFrame(Options.startframe);
  /*
   * Print more information than we'll ever need to know...
   */
  if (Options.verbose) {
    extern Geom *World;
    /* World object info. */
    AggregatePrintInfo(World, Stats.fstats);
    /* Print info about rendering options and the like. */
    RSOptionsList();
  }
  /*
   * Print preprocessing time.
   */
  RSGetCpuTime(&utime, &stime);
  fprintf(Stats.fstats, "Preprocessing time:\t");
  fprintf(Stats.fstats, "%2.2fu  %2.2fs\n", utime, stime);
  fprintf(Stats.fstats, "Starting trace.\n");
  (void)fflush(Stats.fstats);
  /*
   * Render the image.
   */
  Render(argc, argv);
  StatsPrint();
  return 0;
}

// [PASCAL] Removed duplicate implementation of RSStartFrame and RSInitialize
// linked instead with libshade / setup
