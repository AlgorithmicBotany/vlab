/* ******************************************************************** *
   Copyright (C) 1990-2022 University of Calgary
  
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
  
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * ******************************************************************** */



#include <stdlib.h>

#include "scale.h"
#include "xmemory.h"

/*
 *
 * ripped out of xv3.10
 *
 * xvsmooth.c - smoothing/color dither routines for XV
 *
 *  Contains:
 *
 *            byte *Smooth24(pic824, swide, shigh, dwide, dhigh);
 *
 */

static int smoothX(byte *pic24, byte *pic824, int swide, int shigh, int dwide,
                   int dhigh);
static int smoothY(byte *pic24, byte *pic824, int swide, int shigh, int dwide,
                   int dhigh);
static int smoothXY(byte *pic24, byte *pic824, int swide, int shigh, int dwide,
                    int dhigh);

/***************************************************/
static void xvbzero(char *s, size_t len) {
  for (; len > 0; len--)
    *s++ = 0;
}

/***************************************************/
byte *scale(byte *pic824, int swide, int shigh, int dwide, int dhigh) {
  /* does a SMOOTH resize from pic824 (which swide*shigh, 24-bit image
     dwide * dhigh 24-bit image

     returns a dwide*dhigh 24bit image, or NULL on failure (malloc) */

  byte *pic24, *pp;
  int *cxtab, *pxtab;
  int y1Off, cyOff;
  int ex, ey, cx, cy, px, py, apx, apy, x1, y1;
  int pA, pB, pC, pD;
  int retval, bperpix;

  pp = pic24 = (byte *)xmalloc((size_t)(dwide * dhigh * 3));
  if (!pp)
    return NULL;

  bperpix = 3;

  /* decide which smoothing routine to use based on type of expansion */
  if (dwide < swide && dhigh < shigh)
    retval = smoothXY(pic24, pic824, swide, shigh, dwide, dhigh);

  else if (dwide < swide && dhigh >= shigh)
    retval = smoothX(pic24, pic824, swide, shigh, dwide, dhigh);

  else if (dwide >= swide && dhigh < shigh)
    retval = smoothY(pic24, pic824, swide, shigh, dwide, dhigh);

  else {
    /* dwide >= swide && dhigh >= shigh */

    /* cx,cy = original pixel in pic824.  px,py = relative position
       of pixel ex,ey inside of cx,cy as percentages +-50%, +-50%.
       0,0 = middle of pixel */

    /* we can save a lot of time by precomputing cxtab[] and pxtab[], both
       dwide arrays of ints that contain values for the equations:
         cx = (ex * swide) / dwide;
         px = ((ex * swide * 100) / dwide) - (cx * 100) - 50; */

    cxtab = (int *)xmalloc(dwide * sizeof(int));
    if (!cxtab) {
      xfree(pic24);
      return NULL;
    }

    pxtab = (int *)xmalloc(dwide * sizeof(int));
    if (!pxtab) {
      xfree(pic24);
      xfree(cxtab);
      return NULL;
    }

    for (ex = 0; ex < dwide; ex++) {
      cxtab[ex] = (ex * swide) / dwide;
      pxtab[ex] = (((ex * swide) * 100) / dwide) - (cxtab[ex] * 100) - 50;
    }

    for (ey = 0; ey < dhigh; ey++) {
      byte *pptr, rA, gA, bA, rB, gB, bB, rC, gC, bC, rD, gD, bD;

      cy = (ey * shigh) / dhigh;
      py = (((ey * shigh) * 100) / dhigh) - (cy * 100) - 50;
      if (py < 0) {
        y1 = cy - 1;
        if (y1 < 0)
          y1 = 0;
      } else {
        y1 = cy + 1;
        if (y1 > shigh - 1)
          y1 = shigh - 1;
      }

      cyOff = cy * swide * bperpix; /* current line */
      y1Off = y1 * swide * bperpix; /* up or down one line, depending */

      for (ex = 0; ex < dwide; ex++) {
        rA = rB = rC = rD = gA = gB = gC = gD = bA = bB = bC = bD = 0;

        cx = cxtab[ex];
        px = pxtab[ex];

        if (px < 0) {
          x1 = cx - 1;
          if (x1 < 0)
            x1 = 0;
        } else {
          x1 = cx + 1;
          if (x1 > swide - 1)
            x1 = swide - 1;
        }

        pptr = pic824 + y1Off + x1 * bperpix; /* corner pixel */
        rA = *pptr++;
        gA = *pptr++;
        bA = *pptr++;

        pptr = pic824 + y1Off + cx * bperpix; /* up/down center pixel */
        rB = *pptr++;
        gB = *pptr++;
        bB = *pptr++;

        pptr = pic824 + cyOff + x1 * bperpix; /* left/right center pixel */
        rC = *pptr++;
        gC = *pptr++;
        bC = *pptr++;

        pptr = pic824 + cyOff + cx * bperpix; /* center pixel */
        rD = *pptr++;
        gD = *pptr++;
        bD = *pptr++;

        /* compute weighting factors */
        apx = abs(px);
        apy = abs(py);
        pA = (apx * apy) / 100;
        pB = (apy * (100 - apx)) / 100;
        pC = (apx * (100 - apy)) / 100;
        pD = 100 - (pA + pB + pC);

        *pp++ = ((int)(pA * rA)) / 100 + ((int)(pB * rB)) / 100 +
                ((int)(pC * rC)) / 100 + ((int)(pD * rD)) / 100;

        *pp++ = ((int)(pA * gA)) / 100 + ((int)(pB * gB)) / 100 +
                ((int)(pC * gC)) / 100 + ((int)(pD * gD)) / 100;

        *pp++ = ((int)(pA * bA)) / 100 + ((int)(pB * bB)) / 100 +
                ((int)(pC * bC)) / 100 + ((int)(pD * bD)) / 100;
      }
    }

    xfree(cxtab);
    xfree(pxtab);
    retval = 0; /* okay */
  }

  if (retval) { /* one of the Smooth**() methods failed */
    xfree(pic24);
    pic24 = (byte *)NULL;
  }

  return pic24;
}

/***************************************************/
static int smoothX(byte *pic24, byte *pic824, int swide, int shigh, int dwide,
                   int dhigh) {
  byte *cptr, *cptr1;
  int i, j;
  int *lbufR, *lbufG, *lbufB;
  int pixR, pixG, pixB, bperpix;
  int pcnt0, pcnt1, lastpix, pixcnt, thisline, ypcnt;
  int *pixarr, *paptr;

  /* returns '0' if okay, '1' if failed (malloc) */

  /* for case where pic8 is shrunk horizontally and stretched vertically
     maps pic8 into an dwide * dhigh 24-bit picture.  Only works correctly
     when swide>=dwide and shigh<=dhigh */

  /* malloc some arrays */
  lbufR = (int *)calloc((size_t)swide, sizeof(int));
  lbufG = (int *)calloc((size_t)swide, sizeof(int));
  lbufB = (int *)calloc((size_t)swide, sizeof(int));
  pixarr = (int *)calloc((size_t)swide + 1, sizeof(int));

  if (!lbufR || !lbufG || !lbufB || !pixarr) {
    if (lbufR)
      free(lbufR);
    if (lbufG)
      free(lbufG);
    if (lbufB)
      free(lbufB);
    if (pixarr)
      free(pixarr);
    return 1;
  }

  bperpix = 3;

  for (j = 0; j <= swide; j++)
    pixarr[j] = (j * dwide + (15 * swide) / 16) / swide;

  cptr = pic824;
  cptr1 = cptr + swide * bperpix;

  for (i = 0; i < dhigh; i++) {

    ypcnt = (((i * shigh) << 6) / dhigh) - 32;
    if (ypcnt < 0)
      ypcnt = 0;

    pcnt1 = ypcnt & 0x3f; /* 64ths of NEXT line to use */
    pcnt0 = 64 - pcnt1;   /* 64ths of THIS line to use */

    thisline = ypcnt >> 6;

    cptr = pic824 + thisline * swide * bperpix;
    if (thisline + 1 < shigh)
      cptr1 = cptr + swide * bperpix;
    else
      cptr1 = cptr;

    for (j = 0; j < swide; j++) {
      lbufR[j] = ((int)((*cptr++ * pcnt0) + (*cptr1++ * pcnt1))) >> 6;
      lbufG[j] = ((int)((*cptr++ * pcnt0) + (*cptr1++ * pcnt1))) >> 6;
      lbufB[j] = ((int)((*cptr++ * pcnt0) + (*cptr1++ * pcnt1))) >> 6;
    }

    pixR = pixG = pixB = pixcnt = lastpix = 0;

    for (j = 0, paptr = pixarr; j <= swide; j++, paptr++) {
      if (*paptr != lastpix) { /* write a pixel to pic24 */
        if (!pixcnt)
          pixcnt = 1; /* this NEVER happens:  quiets compilers */
        *pic24++ = pixR / pixcnt;
        *pic24++ = pixG / pixcnt;
        *pic24++ = pixB / pixcnt;
        lastpix = *paptr;
        pixR = pixG = pixB = pixcnt = 0;
      }

      if (j < swide) {
        pixR += lbufR[j];
        pixG += lbufG[j];
        pixB += lbufB[j];
        pixcnt++;
      }
    }
  }

  free(lbufR);
  free(lbufG);
  free(lbufB);
  free(pixarr);
  return 0;
}

/***************************************************/
static int smoothY(byte *pic24, byte *pic824, int swide, int shigh, int dwide,
                   int dhigh) {
  byte *clptr, *cptr, *cptr1;
  int i, j, bperpix;
  int *lbufR, *lbufG, *lbufB, *pct0, *pct1, *cxarr, *cxptr;
  int lastline, thisline, linecnt;
  int retval;

  /* returns '0' if okay, '1' if failed (malloc) */

  /* for case where pic8 is shrunk vertically and stretched horizontally
     maps pic8 into a dwide * dhigh 24-bit picture.  Only works correctly
     when swide<=dwide and shigh>=dhigh */

  retval = 0; /* no probs, yet... */

  bperpix = 3;

  lbufR = lbufG = lbufB = pct0 = pct1 = cxarr = NULL;
  lbufR = (int *)calloc((size_t)dwide, sizeof(int));
  lbufG = (int *)calloc((size_t)dwide, sizeof(int));
  lbufB = (int *)calloc((size_t)dwide, sizeof(int));
  pct0 = (int *)calloc((size_t)dwide, sizeof(int));
  pct1 = (int *)calloc((size_t)dwide, sizeof(int));
  cxarr = (int *)calloc((size_t)dwide, sizeof(int));

  if (!lbufR || !lbufG || !lbufB || !pct0 || !pct1 || !cxarr) {
    retval = 1;
    goto smyexit;
  }

  for (i = 0; i < dwide; i++) { /* precompute some handy tables */
    int cx64;
    cx64 = (((i * swide) << 6) / dwide) - 32;
    if (cx64 < 0)
      cx64 = 0;
    pct1[i] = cx64 & 0x3f;
    pct0[i] = 64 - pct1[i];
    cxarr[i] = cx64 >> 6;
  }

  lastline = linecnt = 0;

  for (i = 0, clptr = pic824; i <= shigh; i++, clptr += swide * bperpix) {

    thisline = (i * dhigh + (15 * shigh) / 16) / shigh;

    if (thisline != lastline) { /* copy a line to pic24 */
      for (j = 0; j < dwide; j++) {
        *pic24++ = lbufR[j] / linecnt;
        *pic24++ = lbufG[j] / linecnt;
        *pic24++ = lbufB[j] / linecnt;
      }

      xvbzero((char *)lbufR, dwide * sizeof(int)); /* clear out line bufs */
      xvbzero((char *)lbufG, dwide * sizeof(int));
      xvbzero((char *)lbufB, dwide * sizeof(int));
      linecnt = 0;
      lastline = thisline;
    }

    for (j = 0, cxptr = cxarr; j < dwide; j++, cxptr++) {
      cptr = clptr + *cxptr * bperpix;
      if (*cxptr < swide - 1)
        cptr1 = cptr + 1 * bperpix;
      else
        cptr1 = cptr;

      lbufR[j] += ((int)((*cptr++ * pct0[j]) + (*cptr1++ * pct1[j]))) >> 6;
      lbufG[j] += ((int)((*cptr++ * pct0[j]) + (*cptr1++ * pct1[j]))) >> 6;
      lbufB[j] += ((int)((*cptr++ * pct0[j]) + (*cptr1++ * pct1[j]))) >> 6;
    }

    linecnt++;
  }

smyexit:
  if (lbufR)
    free(lbufR);
  if (lbufG)
    free(lbufG);
  if (lbufB)
    free(lbufB);
  if (pct0)
    free(pct0);
  if (pct1)
    free(pct1);
  if (cxarr)
    free(cxarr);

  return retval;
}

/***************************************************/
static int smoothXY(byte *pic24, byte *pic824, int swide, int shigh, int dwide,
                    int dhigh) {
  byte *cptr;
  int i, j;
  int *lbufR, *lbufG, *lbufB;
  int pixR, pixG, pixB;
  int lastline, thisline, lastpix, linecnt, pixcnt;
  int *pixarr, *paptr;

  /* returns '0' if okay, '1' if failed (malloc) */

  /* shrinks pic8 into a dwide * dhigh 24-bit picture.  Only works correctly
     when swide>=dwide and shigh>=dhigh (ie, the picture is shrunk on both
     axes) */

  /* malloc some arrays */
  lbufR = (int *)calloc((size_t)swide, sizeof(int));
  lbufG = (int *)calloc((size_t)swide, sizeof(int));
  lbufB = (int *)calloc((size_t)swide, sizeof(int));
  pixarr = (int *)calloc((size_t)swide + 1, sizeof(int));
  if (!lbufR || !lbufG || !lbufB || !pixarr) {
    if (lbufR)
      free(lbufR);
    if (lbufG)
      free(lbufG);
    if (lbufB)
      free(lbufB);
    if (pixarr)
      free(pixarr);
    return 1;
  }

  for (j = 0; j <= swide; j++)
    pixarr[j] = (j * dwide + (15 * swide) / 16) / swide;

  lastline = linecnt = pixR = pixG = pixB = 0;
  cptr = pic824;

  for (i = 0; i <= shigh; i++) {

    thisline = (i * dhigh + (15 * shigh) / 16) / shigh;

    if ((thisline != lastline)) { /* copy a line to pic24 */
      pixR = pixG = pixB = pixcnt = lastpix = 0;

      for (j = 0, paptr = pixarr; j <= swide; j++, paptr++) {
        if (*paptr != lastpix) { /* write a pixel to pic24 */
          if (!pixcnt)
            pixcnt = 1; /* NEVER happens: quiets compilers */
          *pic24++ = (pixR / linecnt) / pixcnt;
          *pic24++ = (pixG / linecnt) / pixcnt;
          *pic24++ = (pixB / linecnt) / pixcnt;
          lastpix = *paptr;
          pixR = pixG = pixB = pixcnt = 0;
        }

        if (j < swide) {
          pixR += lbufR[j];
          pixG += lbufG[j];
          pixB += lbufB[j];
          pixcnt++;
        }
      }

      lastline = thisline;
      xvbzero((char *)lbufR, swide * sizeof(int)); /* clear out line bufs */
      xvbzero((char *)lbufG, swide * sizeof(int));
      xvbzero((char *)lbufB, swide * sizeof(int));
      linecnt = 0;
    }

    if (i < shigh) {
      for (j = 0; j < swide; j++) {
        lbufR[j] += *cptr++;
        lbufG[j] += *cptr++;
        lbufB[j] += *cptr++;
      }
      linecnt++;
    }
  }

  free(lbufR);
  free(lbufG);
  free(lbufB);
  free(pixarr);
  return 0;
}
