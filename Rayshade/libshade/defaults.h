/*
 * defaults.h
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
#ifndef DEFAULTS_H
#define DEFAULTS_H

#define DEFSAMPLES 3 /* sqrt of number of samples */

#define XRESOLUTION 512 /* Default screen size (pixels) */
#define YRESOLUTION 512
#define DEFCUTOFF 0.002 /* Default tree cutoff value */
#define MAXDEPTH 5      /* Maximum ray tree depth. */
#define HFOV 45         /* Field-of-view */
#define EYEX 0          /* Eye position */
#define EYEY -10
#define EYEZ 0
#define LOOKX 0 /* Look point */
#define LOOKY 0
#define LOOKZ 0
#define UPX 0 /* Up vector */
#define UPY 0
#define UPZ 1
#define GAMMA 1.        /* Default gamma */
#define GAUSSIAN FALSE  /* Use gaussian pixel filter? */
#define FILTERWIDTH 1.8 /* Default gaussian filter width */

#if defined(LINDA) && !defined(WORKERS)
#define WORKERS 4 /* Default # of workers. */
#endif

#define REPORTFREQ 10 /* Frequency of status report */

#define DEFREDCONT 0.2 /* Default contrast threshold values. */
#define DEFGREENCONT 0.15
#define DEFBLUECONT 0.3

#endif /* DEFAULTS_H */
