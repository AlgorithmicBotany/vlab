/*
 * parts/time.h
 *
 * Internal header file, declaring types and structures which nominally
 * originate from time.h, but which POSIX requires to be made visible on
 * inclusion of certain other headers, without including time.h itself.
 *
 * $Id: time.h,v 43b5fbe8586e 2014/12/23 20:59:37 keithmarshall $
 *
 * Written by Keith Marshall  <keithmarshall@users.sourceforge.net>
 * Copyright (C) 2014, MinGW.org Project.
 *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, this permission notice, and the following
 * disclaimer shall be included in all copies or substantial portions of
 * the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OF OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */
#if _FAKE_TIME_H_SOURCED
/*
 * Since we expect this part-header to be sourced exclusively by other
 * system headers, (nominally time.h), we don't apply any conventional
 * multiple inclusion guard; rather, we rely on the guard within time.h
 * itself, but allow other headers to fake it for one-shot inclusion
 * only...
 */
# undef _FAKE_TIME_H_SOURCED

#elif ! defined _TIME_H
/*
 * ...otherwise, we fail if the time.h guard is not in place.
 */
# error "Never include <parts/time.h> directly; use <time.h> instead"
#endif

#if defined __need_struct_timespec && ! __struct_timespec_defined
/*
 * Structure timespec is mandated by POSIX, for specification of
 * intervals with the greatest precision supported by the OS kernel.
 * Although this allows for specification to nanosecond precision, do
 * not be deluded into any false expectation that such short intervals
 * can be realized on Windows; on Win9x derivatives, the metronome used
 * by the process scheduler has a period of ~55 milliseconds, while for
 * WinNT derivatives, the corresponding period is ~15 milliseconds; thus,
 * the shortest intervals which can be realistically timed will range
 * from 0..55 milliseconds on Win9x hosts, and from 0..15 ms on WinNT,
 * with period values normally distributed around means of ~27.5 ms
 * and ~7.5 ms, for the two system types respectively.
 */
struct timespec
{
  /* Period is sum of tv_sec + tv_nsec; use fundamental integer types
   * to avoid 32-bit vs. 64-bit time_t ambiguity.
   */
  long long	tv_sec; 	/* seconds */
  long  	tv_nsec;	/* nanoseconds */
};
# define __struct_timespec_defined  1
#endif

/* $RCSfile: time.h,v $: end of file */
