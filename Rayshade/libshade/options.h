/*
 * options.h
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
#ifndef OPTIONS_H
#define OPTIONS_H

/*
 * Constants for Stereo mode
 */
#define LEFT 1
#define RIGHT 2

/*
 * Options
 */
typedef struct RSOptions {
  int stereo,          /* Stereo mode? */
      verbose,         /* Babbling mode? */
      quiet,           /* Don't complain? */
      jitter,          /* use jittered sampling? */
      samples,         /* Sqrt of # of samples */
      maxdepth,        /* Maximum ray tree depth */
      report_freq,     /* Frequency, in lines, of report */
      no_shadows,      /* Trace shadow rays? */
      shadowtransp,    /* ... through transparent objects? */
      cache,           /* Cache shadowing info? */
      appending,       /* Append to image file? */
      resolution_set,  /* resolution set on command line */
      contrast_set,    /* contrast overridden ... */
      samples_set,     /* samples overridden ... */
      cutoff_set,      /* cutoff ... */
      maxdepth_set,    /* adaptive depth ... */
      window_set,      /* subwindow ... */
      crop_set,        /* crop window ... */
      freq_set,        /* report frequency ... */
      jitter_set,      /* use jittering */
      eyesep_set,      /* eye separation ... */
      csg,             /* CSG object someplace in world */
      flipnorm,        /* flip normals of polygonal objs */
      samplemap,       /* output sample map? */
      gaussian,        /* Use gaussian pixel filter? */
      framenum,        /* current frame number */
      startframe,      /* Starting frame number. */
      endframe,        /* ending frame number */
      totalframes,     /* total # of frames */
      totalframes_set, /* set on command line? */
      cpp;             /* run CPP? */
#ifdef URT
  int alpha;      /* Write alpha channel? */
  int exp_output; /* Write exponential RLE file? */
#endif
  Float eyesep,     /* Eye separation (for Stereo mode) */
      gamma,        /* Gamma value (0 == no correction) */
      starttime,    /* Think about it ... */
      shutterspeed, /* time shutter is open */
      framestart,   /* start time of the current frame */
      framelength,  /* length of the current frame */
      filterwidth;  /* Pixel filter width. */
  Color contrast,   /* Max. allowable contrast */
      cutoff,       /* Ray tree depth control */
      ambient;      /* Ambient light multiplier */
  char *progname,   /* argv[0] */
      *statsname,   /* Name of stats file. */
      *imgname,     /* Name of output image file */
      *inputname,   /* Name of input file, NULL == stdin */
      *cppargs;     /* arguments to pass to cpp */
  int window[2][2]; /* Subwindow corners */
  Float crop[2][2]; /* Crop window, lo/hi normalized */
#ifdef LINDA
  int workers,        /* # of worker processes */
      workernum,      /* worker #, 0 == supervisor */
      verbose_worker; /* Babble while you work? */
#endif
  FILE *pictfile; /* output file pointer */
} RSOptions;

extern RSOptions Options;
extern void OptionsList(), OptionsSet();
void RSOptionsSet(int argc, char **argv);
void RSOptionsList();
#endif /* OPTIONS_H */
