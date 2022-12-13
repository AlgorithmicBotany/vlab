/*
 * setup.c
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
#include "setup.h"
#include "rayshade.h"
#include "defaults.h"
#include "rayshade/version.h"
#include "libsurf/surface.h"
#include "libsurf/atmosphere.h"
#include "liblight/light.h"
#include "liblight/infinite.h"
#include "libobj/list.h"
#include "libcommon/sampling.h"
#include "objdef.h"
#include "options.h"
#include "stats.h"
#include "viewing.h"
#include "picture.h"
#include "lightdef.h"
#include "misc.h"

#ifdef MULTIMAX
#include <parallel.h>
#define SHARED_BYTES 23 /* 2^23 bytes of shared memory */
#endif

extern GeomList *Defstack;

/*
 * Set default parameters
 */
void RSSetup() {
  extern SurfList *CurSurf;
  extern Medium TopMedium;
  extern void NoiseInit();
#ifdef MULTIMAX
  unsigned int bytes;

  /*
   * Initialize shared memory stuff.
   */
  bytes = 1 << SHARED_BYTES;
  if (share_malloc_init(bytes) == -1) {
    RLerror(RL_PANIC, "Cannot share_malloc %d bytes.\n", bytes);
  } else
    fprintf(fstats, "Malloced %d bytes of shared memory.\n", bytes);
#endif

  Camera.hfov = HFOV;
  Camera.vfov = UNSET;
  Camera.pos.x = EYEX;
  Camera.pos.y = EYEY;
  Camera.pos.z = EYEZ;
  Camera.lookp.x = LOOKX;
  Camera.lookp.y = LOOKY;
  Camera.lookp.z = LOOKZ;
  Camera.up.x = UPX;
  Camera.up.y = UPY;
  Camera.up.z = UPZ;
  Camera.focaldist = UNSET;
  Camera.aperture = 0.;

  Screen.xres = Screen.yres = UNSET;

  Options.cpp = TRUE;
  Options.maxdepth = MAXDEPTH;
  Options.report_freq = REPORTFREQ;
  Options.jitter = TRUE;
  Options.samples = UNSET;
  Options.gaussian = GAUSSIAN;
  Options.filterwidth = UNSET;
  Options.contrast.r = UNSET;
  Options.ambient.r = Options.ambient.g = Options.ambient.b = 1.0;
  Options.cutoff.r = UNSET;
  Options.cache = TRUE;
  Options.shadowtransp = TRUE;
  Options.crop[LOW][X] = Options.crop[LOW][Y] = 0.;
  Options.crop[HIGH][X] = Options.crop[HIGH][Y] = 1.;
  Stats.fstats = stderr;
  Options.pictfile = stdout;
#ifdef URT
  Options.alpha = TRUE;
  Options.exp_output = FALSE;
#endif
  Options.gamma = GAMMA;
  Options.eyesep = UNSET;
#ifdef LINDA
  Options.workers = WORKERS;
#endif

  Options.totalframes = 1;
  Options.startframe = 0;
  Options.starttime = 0.;
  Options.framelength = 1.;
  Options.shutterspeed = 0.;

  TopMedium.index = DEFAULT_INDEX;
  TopMedium.statten = 1.0;
  NoiseInit(); /* Initialize values for Noise() */

  /*
   * Top of object definition stack points to the World object.
   * The World object is always a list.
   */
  Defstack = GeomStackPush(GeomListCreate(), (GeomList *)NULL);
  Defstack->obj->name = strsave("World");
  /* Initialize surface stack */
  CurSurf = SurfPush((Surface *)NULL, (SurfList *)NULL);
}

/*
 * cleanup()
 *
 * Initialize options/variables not set on command line or in input file.
 * Perform sanity checks on widow dimension, maxdepth, etc.
 */
void RSCleanup() {
  extern Light *Lights;
  extern void OpenStatsFile();
  extern FILE *yyin;

  yyin = (FILE *)NULL; /* mark that we're done reading input */

  if (Options.samples == UNSET)
    Options.samples = DEFSAMPLES;

  if (Options.filterwidth == UNSET) {
    if (Options.gaussian)
      Options.filterwidth = FILTERWIDTH;
    else
      /* Default box filter width of 1.0 */
      Options.filterwidth = 1.0;
  }

  Options.endframe = Options.startframe + Options.totalframes - 1;

  OpenStatsFile();

  ViewingSetup();

  if (Options.cutoff.r == UNSET)
    Options.cutoff.r = Options.cutoff.g = Options.cutoff.b = DEFCUTOFF;

  /*
   * Set contrast.
   */
  if (Options.contrast.r == UNSET) {
    Options.contrast.r = DEFREDCONT;
    Options.contrast.g = DEFGREENCONT;
    Options.contrast.b = DEFBLUECONT;
  }

  /*
   * Image gamma is inverse of display gamma.
   */
  if (fabs(Options.gamma) > EPSILON)
    Options.gamma = 1. / Options.gamma;
  else
    Options.gamma = FAR_AWAY;

  if (Options.maxdepth < 0)
    Options.maxdepth = 0;

  LightSetup();
}

void RSStartFrame(frame) int frame;
{
  /*
   * Set the frame start time
   */
  Options.framenum = frame;
  Options.framestart =
      Options.starttime + Options.framenum * Options.framelength;
  SamplingSetTime(Options.framestart, Options.shutterspeed, Options.framenum);
  /*
   * Set up viewing parameters.
   * Can't animate camera yet; when possible, this will
   * need to be much smarter.
   * RSViewing();
   */

  /*
   * Initialize world
   */
  WorldSetup();
}

/*
 * Initialize non-time-varying goodies.
 */
void RSInitialize(argc, argv) int argc;
char **argv;
{
  /*
   * Initialize variables, etc.
   */
  RSSetup();
  /*
   * Parse options from command line.
   */
  RSOptionsSet(argc, argv);
  /*
   * Process input file.
   */
  if (Options.verbose) {
    VersionPrint();
    fprintf(Stats.fstats, "Reading input file...\n");
    (void)fflush(Stats.fstats);
  }
  RSReadInputFile();
  /*
   * Set variables that weren't set on command line
   * or in input file.
   */
  RSCleanup();
  /*
   * Set sampling options.
   */
  SamplingSetOptions(Options.samples, Options.gaussian, Options.filterwidth);
  /*
   * Camera is currently static; initialize it here.
   */
  RSViewing();
}
