/*
 * options.c
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
#include "rayshade.h"
#include "options.h"
#include "stats.h"
#include "viewing.h"

RSOptions Options;

static void usage();

void RSOptionsSet(argc, argv) int argc;
char **argv;
{
  extern void OpenStatsFile();

  Options.progname = strsave(argv[0]);
  Options.inputname = (char *)NULL;

  while (--argc) {
    argv++;
    if (argv[0][0] != '-') {
      /*
       * Must be the input file name.
       * If already given, complain and then exit.
       */
      if (Options.inputname != (char *)NULL) {
        usage();
        exit(1);
      }
      Options.inputname = strsave(argv[0]);
      continue;
    }
    /* else */
    switch (argv[0][1]) {
    case 'A':
      /*
       * First frame number
       */
      Options.startframe = atoi(argv[1]);
      argv++;
      argc--;
      break;
#ifdef URT
    case 'a':
      Options.alpha = !Options.alpha;
      break;
#endif
    case 'C':
      Options.cutoff.r = atof(argv[1]);
      Options.cutoff.g = atof(argv[2]);
      Options.cutoff.b = atof(argv[3]);
      Options.cutoff_set = TRUE;
      argv += 3;
      argc -= 3;
      break;
#ifdef URT
    case 'c':
      Options.appending = TRUE;
      break;
#endif
    case 'D':
      Options.maxdepth = atoi(argv[1]);
      Options.maxdepth_set = TRUE;
      argv++;
      argc--;
      break;
    case 'E':
      Options.eyesep = atof(argv[1]);
      Options.eyesep_set = TRUE;
      argc--;
      argv++;
      break;
#ifdef URT
    case 'e':
      Options.exp_output = TRUE;
      break;
#endif
    case 'F':
      Options.report_freq = atoi(argv[1]);
      if (Options.report_freq < 1)
        Options.report_freq = 1;
      Options.freq_set = TRUE;
      argv++;
      argc--;
      break;
    case 'f':
      Options.flipnorm = !Options.flipnorm;
      break;
    case 'G':
      Options.gamma = atof(argv[1]);
      argv++;
      argc--;
      break;
    case 'g':
      Options.gaussian = !Options.gaussian;
      break;
    case 'h':
      usage();
      exit(0);
      break;
    case 'j':
      Options.jitter = !Options.jitter;
      Options.jitter_set = TRUE;
      break;
    case 'l':
      Options.stereo = LEFT;
      break;
#ifdef URT
    case 'm':
      Options.samplemap = !Options.samplemap;
      break;
#endif
    case 'N':
      Options.totalframes = atof(argv[1]);
      Options.totalframes_set = TRUE;
      argv++;
      argc--;
      break;
    case 'n':
      Options.no_shadows = !Options.no_shadows;
      break;
    case 'O':
      Options.imgname = strsave(argv[1]);
      argv++;
      argc--;
      break;
    case 'o':
      Options.shadowtransp = !Options.shadowtransp;
      break;
    case 'P':
      Options.cppargs = argv[1];
      argv++;
      argc--;
      break;
    case 'p':
      /*
       * Preview-quality rendering
       * no shadows
       * max depth of 0
       * 1 jittered sample/pixel
       */
      Options.no_shadows = TRUE;
      Options.maxdepth = 0;
      Options.maxdepth_set = TRUE;
      Options.jitter = TRUE;
      Options.jitter_set = TRUE;
      Options.samples = 1;
      Options.samples_set = TRUE;
      break;
    case 'q':
      Options.quiet = TRUE;
      break;
    case 'R':
      Screen.xres = atoi(argv[1]);
      Screen.yres = atoi(argv[2]);
      Options.resolution_set = TRUE;
      argv += 2;
      argc -= 2;
      break;
    case 'r':
      Options.stereo = RIGHT;
      break;
    case 'S':
      Options.samples = atoi(argv[1]);
      if (Options.samples < 1)
        Options.samples = 1;
      Options.samples_set = TRUE;
      argv++;
      argc--;
      break;
    case 's':
      Options.cache = !Options.cache;
      break;
    case 'T':
      Options.contrast.r = atof(argv[1]);
      Options.contrast.g = atof(argv[2]);
      Options.contrast.b = atof(argv[3]);
      Options.contrast_set = TRUE;
      argv += 3;
      argc -= 3;
      break;
    case 'u':
      Options.cpp = !Options.cpp;
      break;
    case 'v':
      Options.verbose = TRUE;
      break;
    case 'V':
      Options.verbose = TRUE;
      if (argv[1][0] == '-') {
        /* User probably blew it, and
         * it's difficult to remove a file
         * that begins with '-'...
         */
        usage();
        exit(2);
      }
      Options.statsname = strsave(argv[1]);
      OpenStatsFile();
      argv++;
      argc--;
      break;
    case 'W':
      Options.window[LOW][X] = atoi(argv[1]);
      Options.window[HIGH][X] = atoi(argv[2]);
      Options.window[LOW][Y] = atoi(argv[3]);
      Options.window[HIGH][Y] = atoi(argv[4]);
      Options.window_set = TRUE;
      argv += 4;
      argc -= 4;
      break;
    case 'X':
      Options.crop[LOW][X] = atof(argv[1]);
      Options.crop[HIGH][X] = atof(argv[2]);
      Options.crop[LOW][Y] = atof(argv[3]);
      Options.crop[HIGH][Y] = atof(argv[4]);
      Options.crop_set = TRUE;
      argv += 4;
      argc -= 4;
      break;
    default:
      RLerror(RL_PANIC, "Bad argument: %s\n", argv[0]);
    }
  }
}

void RSOptionsList() {
  if (Options.totalframes > 1) {
    fprintf(Stats.fstats, "Rendering frames %d through %d.\n",
            Options.startframe, Options.endframe);
  } else {
    fprintf(Stats.fstats, "Rendering frame %d.\n", Options.startframe);
  }

  fprintf(Stats.fstats, "Screen resolution: %d x %d\n", Screen.xres,
          Screen.yres);
  fprintf(Stats.fstats, "Image window: (%d - %d), (%d - %d).\n", Screen.minx,
          Screen.maxx, Screen.miny, Screen.maxy);

  if (Options.jitter)
    fprintf(Stats.fstats, "Using jittered sampling, ");
  fprintf(Stats.fstats, "Max sampling rate %d %s/pixel.\n",
          Options.samples * Options.samples,
          Options.samples == 1 ? "sample" : "samples");

  fprintf(Stats.fstats, "Maximum contrast: %g red, %g green, %g blue.\n",
          Options.contrast.r, Options.contrast.g, Options.contrast.b);
  fprintf(Stats.fstats, "Maximum ray depth: %d.  Cutoff thresh: %g %g %g.\n",
          Options.maxdepth, Options.cutoff.r, Options.cutoff.g,
          Options.cutoff.b);
  if (Options.stereo == LEFT)
    fprintf(Stats.fstats, "Rendering image for left eye.\n");
  else if (Options.stereo == RIGHT)
    fprintf(Stats.fstats, "Rendering image for right eye.\n");
  if (Options.no_shadows) {
    fprintf(Stats.fstats, "No shadows are rendered.\n");
  } else if (Options.shadowtransp) {
    fprintf(Stats.fstats, "Object opacity affects depth of shadowing.\n");
  }
  if (!Options.cache)
    fprintf(Stats.fstats, "Shadow caching is disabled.\n");
  if (Options.totalframes != 1)
    fprintf(Stats.fstats, "Rendering %d frames.\n", Options.totalframes);
}

static void usage() {
  fprintf(stderr, "usage: %s [options] [filename]\n", Options.progname);
  fprintf(stderr, "Where options include:\n");
  fprintf(stderr, "\t-A frame\t(Begin with given frame #.)\n");
#ifdef URT
  fprintf(stderr, "\t-a \t\t(Toggle writing of alpha channel.)\n");
#endif
  fprintf(stderr, "\t-C thresh\t(Set adaptive ray tree cutoff value.)\n");
#ifdef URT
  fprintf(stderr, "\t-c \t\t(Continue interrupted rendering.)\n");
#endif
  fprintf(stderr, "\t-D depth\t(Set maximum ray tree depth.)\n");
  fprintf(stderr, "\t-E eye_sep\t(Set eye separation in stereo pairs.)\n");
#ifdef URT
  fprintf(stderr, "\t-e \t\t(Write exponential RLE file.)\n");
#endif
  fprintf(stderr, "\t-F freq\t\t(Set frequency of status report.)\n");
  fprintf(stderr, "\t-f \t\t(Flip all triangle normals.)\n");
  fprintf(stderr, "\t-G gamma\t(Use given gamma correction exponent.)\n");
  fprintf(stderr, "\t-g \t\t(Use Gaussian pixel filter.)\n");
  fprintf(stderr, "\t-h \t\t(Print this message.)\n");
  fprintf(stderr, "\t-j \t\t(Toggle jittered sampling.)\n");
  fprintf(stderr, "\t-l \t\t(Render image for left eye view.)\n");
#ifdef URT
  fprintf(stderr, "\t-m \t\t(Output sample map in alpha channel.)\n");
#endif
  fprintf(stderr, "\t-N number\t(Render given number of frames.)\n");
  fprintf(stderr, "\t-n \t\t(Do not render shadows.)\n");
  fprintf(stderr, "\t-O outfile \t(Set output file name.)\n");
  fprintf(stderr, "\t-o \t\t(Toggle opacity effect on shadowing.)\n");
  fprintf(stderr, "\t-P cpp-args\t(Options to pass to C pre-processor.\n");
  fprintf(stderr, "\t-p \t\t(Preview-quality rendering.)\n");
  fprintf(stderr, "\t-q \t\t(Run quietly.)\n");
  fprintf(stderr, "\t-R xres yres\t(Render at given resolution.)\n");
  fprintf(stderr, "\t-r \t\t(Render image for right eye view.)\n");
  fprintf(stderr, "\t-S samples\t(Max density of samples^2 samples.)\n");
  fprintf(stderr, "\t-s \t\t(Don't cache shadowing information.)\n");
  fprintf(stderr, "\t-T r g b\t(Set contrast threshold (0. - 1.).)\n");
  fprintf(stderr, "\t-V filename \t(Write verbose output to filename.)\n");
  fprintf(stderr, "\t-v \t\t(Verbose output.)\n");
  fprintf(stderr, "\t-W x x y y \t(Render subwindow.)\n");
  fprintf(stderr, "\t-X l r b t \t(Crop window.)\n");
}
