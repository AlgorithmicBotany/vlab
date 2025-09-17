/*
 * render.c
 *
 * Copyright (C) 1989, 1991 Craig E. Kolb, Rod G. Bogart
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
#include "libcommon/sampling.h"
#include "libsurf/atmosphere.h"
#include "viewing.h"
#include "options.h"
#include "stats.h"
#include "picture.h"
#include "render.h"
#include "graphics.h"

Ray TopRay; /* Top-level ray. */
int Rectmode = FALSE, Rectx0, Recty0, Rectx1, Recty1;
int SuperSampleMode = 0, SuperSampling = 0;

SSquare *SSquares;

void Render(argc, argv) int argc;
char **argv;
{
  /*
   * Do an adaptive trace, displaying results in a
   * window as we go.
   */
  int y;

  /*
   * The top-level ray TopRay always has as its origin the
   * eye position and as its medium NULL, indicating that it
   * is passing through a medium with index of refraction
   * equal to DefIndex.
   */
  TopRay.pos = Camera.pos;
  TopRay.media = (Medium *)0;
  TopRay.depth = 0;
  /*
   * Doesn't handle motion blur yet.
   */
  TopRay.time = Options.framestart;

  GraphicsInit(Screen.xsize, Screen.ysize, "raypaint", &argc, argv);
  /*
   * Allocate array of samples.
   */
  Image = (RGB **)Malloc(Screen.ysize * sizeof(RGB *));
  SampleMap = (char **)Malloc(Screen.ysize * sizeof(char *));
  for (y = 0; y < Screen.ysize; y++) {
    Image[y] = (RGB *)Calloc(Screen.xsize, sizeof(RGB));
    SampleMap[y] = (char *)Calloc(Screen.xsize, sizeof(char));
  }
  switch (Sampling.sidesamples) {
  case 1:
    SampleNumbers = OneSample;
    break;
  case 2:
    SampleNumbers = TwoSamples;
    break;
  case 3:
    SampleNumbers = ThreeSamples;
    break;
  case 4:
    SampleNumbers = FourSamples;
    break;
  case 5:
    SampleNumbers = FiveSamples;
    break;
  case 6:
    SampleNumbers = SixSamples;
    break;
  case 7:
    SampleNumbers = SevenSamples;
    break;
  case 8:
    SampleNumbers = EightSamples;
    break;
  default:
    RLerror(RL_PANIC, "Sorry, %d rays/pixel not supported.\n",
            Sampling.totsamples);
  }
  /*
   * Take initial four samples
   */
  SSquareSample(0, 0, FALSE);
  SSquareSample(Screen.xsize - 1, 0, FALSE);
  SSquareSample(Screen.xsize - 1, Screen.ysize - 1, FALSE);
  SSquareSample(0, Screen.ysize - 1, FALSE);
  SSquares = SSquareInstall(0, 0, Screen.xsize - 1, Screen.ysize - 1, 0,
                            (SSquare *)NULL);

  for (y = 1; y <= 1; y++) {
    /*
     * Create and draw every region at depth y.
     */
    SSquareDivideToDepth(SSquares, y);
  }

  GraphicsMainLoop(DivideFunc);
}

void SSHandleMouseDrag(void) {
  int x1, x2, y1, y2;

  GraphicsGetMouseDrag(&x1, &y1, &x2, &y2);

  if (x1 < x2) {
    Rectx0 = (x1 < 0) ? 0 : x1;
    Rectx1 = (x2 >= Screen.xsize) ? Screen.xsize - 1 : x2;
  } else {
    Rectx0 = (x2 < 0) ? 0 : x2;
    Rectx1 = (x1 >= Screen.xsize) ? Screen.xsize - 1 : x1;
  }
  if (y1 < y2) {
    Recty0 = (y1 < 0) ? 0 : y1;
    Recty1 = (y2 >= Screen.ysize) ? Screen.ysize - 1 : y2;
  } else {
    Recty0 = (y2 < 0) ? 0 : y2;
    Recty1 = (y1 >= Screen.ysize) ? Screen.ysize - 1 : y1;
  }

  Rectmode = TRUE;
  RecomputePriority(SSquares);

  return;
}

void DivideFunc(void) {
  SSquare *cursq;
  Pixel *pixelbuf;
  int x, y;

  cursq = SSquareSelect(SSquares);
  if (cursq != (SSquare *)NULL) {
    SSquareDivide(cursq);
    if (GraphicsDragEvent())
      SSHandleMouseDrag();
  } else {
    fprintf(stderr, "Writing image as PPM...\n");
    /*
     * Finished the image; write to image file.
     */
    pixelbuf = (Pixel *)Malloc(Screen.xsize * sizeof(Pixel));

    /**** WARNING: PictureStart should be sent argv, but it's
     **** only used if we're outputing an RLE. In order to make
     **** this code a little cleaner, I'm sending NULL instead,
     **** but it'll have to be changed if we ever output RLE.
     */
    PictureStart(NULL);
    for (y = 0; y < Screen.ysize; y++) {
      /*
       * This is really disgusting.
       */
      for (x = 0; x < Screen.xsize; x++) {
        pixelbuf[x].r = Image[y][x][0] / 255.;
        pixelbuf[x].g = Image[y][x][1] / 255.;
        pixelbuf[x].b = Image[y][x][2] / 255.;
        pixelbuf[x].alpha = SampleMap[y][x];
      }
      PictureWriteLine(pixelbuf);
    }
    PictureEnd();
    free((voidstar)pixelbuf);
    exit(0);
  }
}

Float SampleTime(sampnum) int sampnum;
{
  Float window, jitter = 0.0, res;

  if (Options.shutterspeed <= 0.)
    return Options.framestart;
  if (Options.jitter)
    jitter = nrand();
  window = Options.shutterspeed / Sampling.totsamples;
  res = Options.framestart + window * (sampnum + jitter);
  TimeSet(res);
  return res;
}

int RecomputePriority(sq) SSquare *sq;
{
  Float maxp;

  if (!OVERLAPS_RECT(sq)) {
    sq->closed = SSCLOSED;
    return FALSE;
  }

  if (sq->leaf) {
    if (SQ_AREA(sq) >= MINAREA)
      sq->closed = NOT_CLOSED;
    return TRUE;
  }
  maxp = 0.;
  if (RecomputePriority(sq->child[0]))
    maxp = max(maxp, sq->child[0]->prio);
  if (RecomputePriority(sq->child[1]))
    maxp = max(maxp, sq->child[1]->prio);
  if (RecomputePriority(sq->child[2]))
    maxp = max(maxp, sq->child[2]->prio);
  if (RecomputePriority(sq->child[3]))
    maxp = max(maxp, sq->child[3]->prio);
  sq->prio = maxp;
#if 0
	if ((sq->child[0]->closed == CLOSED_SUPER) &&
	    (sq->child[1]->closed == CLOSED_SUPER) &&
	    (sq->child[2]->closed == CLOSED_SUPER) &&
	    (sq->child[3]->closed == CLOSED_SUPER))
		sq->closed = CLOSED_SUPER;
	else if (sq->child[0]->closed && sq->child[1]->closed &&
		 sq->child[2]->closed && sq->child[3]->closed)
		sq->closed = CLOSED_PARTIAL;
	else
		sq->closed = NOT_CLOSED;
#else
  if ((sq->child[0]->closed >= SSCLOSED) &&
      (sq->child[1]->closed >= SSCLOSED) &&
      (sq->child[2]->closed >= SSCLOSED) && (sq->child[3]->closed >= SSCLOSED))
    sq->closed = SSCLOSED;
  else
    sq->closed = NOT_CLOSED;
#endif
  return TRUE;
}

void SSquareSample(x, y, supersample) int x, y, supersample;
{
  Float upos, vpos, u, v;
  int xx, yy, xp, sampnum, usamp, vsamp;
  Pixel ctmp;
  Pixel p;
  extern unsigned char correct();

  if (SampleMap[y][x] >= 128 + supersample)
    /* already a sample there */
    return ;
  SampleMap[y][x] = 128 + supersample;
  if (supersample) {
    p.r = p.g = p.b = p.alpha = 0;
    sampnum = 0;
    xp = x + Screen.minx;
    vpos = Screen.miny + y - 0.5 * Sampling.filterwidth;
    for (yy = 0; yy < Sampling.sidesamples;
         yy++, vpos += Sampling.filterdelta) {
      upos = xp - 0.5 * Sampling.filterwidth;
      for (xx = 0; xx < Sampling.sidesamples;
           xx++, upos += Sampling.filterdelta) {
        if (Options.jitter) {
          u = upos + nrand() * Sampling.filterdelta;
          v = vpos + nrand() * Sampling.filterdelta;
        } else {
          u = upos;
          v = vpos;
        }
        TopRay.time = SampleTime(SampleNumbers[sampnum]);
        SampleScreen(u, v, &TopRay, &ctmp, SampleNumbers[sampnum]);
        p.r += ctmp.r * Sampling.filter[xx][yy];
        p.g += ctmp.g * Sampling.filter[xx][yy];
        p.b += ctmp.b * Sampling.filter[xx][yy];
        if (++sampnum == Sampling.totsamples)
          sampnum = 0;
      }
    }
  } else {
    sampnum = nrand() * Sampling.totsamples;
    usamp = sampnum % Sampling.sidesamples;
    vsamp = sampnum / Sampling.sidesamples;

    vpos = Screen.miny + y - 0.5 * Sampling.filterwidth +
           vsamp * Sampling.filterdelta;
    upos = x + Screen.minx - 0.5 * Sampling.filterwidth +
           usamp * Sampling.filterdelta;
    if (Options.jitter) {
      vpos += nrand() * Sampling.filterdelta;
      upos += nrand() * Sampling.filterdelta;
    }
    TopRay.time = SampleTime(SampleNumbers[sampnum]);
    SampleScreen(upos, vpos, &TopRay, &p, SampleNumbers[sampnum]);
  }
  Image[y][x][0] = CORRECT(p.r);
  Image[y][x][1] = CORRECT(p.g);
  Image[y][x][2] = CORRECT(p.b);
}

SSquare *SSquareCreate(xp, yp, xs, ys, d, parent) int xp, yp, xs, ys, d;
SSquare *parent;
{
  SSquare *new;
  Float i1, i2, i3, i4;

  new = (SSquare *)Calloc(1, sizeof(SSquare));
  new->xpos = xp;
  new->ypos = yp;
  new->xsize = xs;
  new->ysize = ys;
  new->depth = d;
  new->parent = parent;
  i1 = INTENSITY(Image[new->ypos][new->xpos]);
  i2 = INTENSITY(Image[new->ypos + new->ysize][new->xpos]);
  i3 = INTENSITY(Image[new->ypos + new->ysize][new->xpos + new->xsize]);
  i4 = INTENSITY(Image[new->ypos][new->xpos + new->xsize]);
  new->mean = 0.25 * (i1 + i2 + i3 + i4);
  if (SQ_AREA(new) < MINAREA) {
    new->prio = 0;
    new->closed = SSCLOSED;
  } else {
    new->var = SSquareComputeLeafVar(new, i1, i2, i3, i4);
    new->prio = PRIORITY(new);
    new->closed = NOT_CLOSED;
  }
  new->leaf = TRUE;
  return new;
}

Float SSquareComputeLeafVar(sq, i1, i2, i3, i4) SSquare *sq;
Float i1, i2, i3, i4;
{
  Float res, diff;

  diff = i1 - sq->mean;
  res = diff * diff;
  diff = i2 - sq->mean;
  res += diff * diff;
  diff = i3 - sq->mean;
  res += diff * diff;
  diff = i4 - sq->mean;
  return res + diff * diff;
}

void SSquareDivideToDepth(sq, d) SSquare *sq;
int d;
{
  if (sq->depth == d)
    return;
  if (sq->leaf)
    SSquareDivide(sq);
  SSquareDivideToDepth(sq->child[0], d);
  SSquareDivideToDepth(sq->child[1], d);
  SSquareDivideToDepth(sq->child[2], d);
  SSquareDivideToDepth(sq->child[3], d);
}

void SSquareDivide(sq) SSquare *sq;
{
  int lowx, lowy, midx, midy, hix, hiy;
  int newxsize, newysize, ndepth, supersample;
  /*
   * Divide the square into fourths by tracing 12
   * new samples if necessary.
   */
  newxsize = sq->xsize / 2;
  newysize = sq->ysize / 2;
  lowx = sq->xpos;
  lowy = sq->ypos;
  midx = sq->xpos + newxsize;
  midy = sq->ypos + newysize;
  hix = sq->xpos + sq->xsize;
  hiy = sq->ypos + sq->ysize;
  ndepth = sq->depth + 1;
  /* create new samples */
  supersample = FALSE;
  SSquareSample(midx, lowy, supersample);
  SSquareSample(lowx, midy, supersample);
  SSquareSample(midx, midy, supersample);
  SSquareSample(hix, midy, supersample);
  SSquareSample(midx, hiy, supersample);
#ifdef SHARED_EDGES
  /* create and draw new squares */
  sq->child[0] = SSquareInstall(lowx, lowy, newxsize, newysize, ndepth, sq);
  sq->child[1] =
      SSquareInstall(midx, lowy, sq->xsize - newxsize, newysize, ndepth, sq);
  sq->child[2] =
      SSquareInstall(lowx, midy, newxsize, sq->ysize - newysize, ndepth, sq);
  sq->child[3] = SSquareInstall(midx, midy, sq->xsize - newxsize,
                                sq->ysize - newysize, ndepth, sq);
#else
  /*
   *  draw additional samples in order to subdivide such that
   * edges of regions do not overlap
   */
  SSquareSample(midx + 1, lowy, supersample);
  SSquareSample(midx + 1, midy, supersample);
  SSquareSample(lowx, midy + 1, supersample);
  SSquareSample(midx, midy + 1, supersample);
  SSquareSample(midx + 1, midy + 1, supersample);
  SSquareSample(hix, midy + 1, supersample);
  SSquareSample(midx + 1, hiy, supersample);

  /* create and draw new squares */
  sq->child[0] = SSquareInstall(lowx, lowy, newxsize, newysize, ndepth, sq);
  sq->child[1] = SSquareInstall(midx + 1, lowy, sq->xsize - newxsize - 1,
                                newysize, ndepth, sq);
  sq->child[2] = SSquareInstall(lowx, midy + 1, newxsize,
                                sq->ysize - newysize - 1, ndepth, sq);
  sq->child[3] = SSquareInstall(midx + 1, midy + 1, sq->xsize - newxsize - 1,
                                sq->ysize - newysize - 1, ndepth, sq);
#endif
  sq->leaf = FALSE;
  /*
   * Recompute parent's mean and variance.
   */
  if (OVERLAPS_RECT(sq))
    SSquareRecomputeStats(sq);
}

void SSquareRecomputeStats(sq) SSquare *sq;
{
  Float maxp;
  int in[4];

  in[0] = OVERLAPS_RECT(sq->child[0]);
  in[1] = OVERLAPS_RECT(sq->child[1]);
  in[2] = OVERLAPS_RECT(sq->child[2]);
  in[3] = OVERLAPS_RECT(sq->child[3]);

  if ((in[0] && (sq->child[0]->closed < SSCLOSED)) ||
      (in[1] && (sq->child[1]->closed < SSCLOSED)) ||
      (in[2] && (sq->child[2]->closed < SSCLOSED)) ||
      (in[3] && (sq->child[3]->closed < SSCLOSED))) {
    maxp = 0.;
    if (in[0])
      maxp = max(maxp, sq->child[0]->prio);
    if (in[1])
      maxp = max(maxp, sq->child[1]->prio);
    if (in[2])
      maxp = max(maxp, sq->child[2]->prio);
    if (in[3])
      maxp = max(maxp, sq->child[3]->prio);
    sq->closed = NOT_CLOSED;
    sq->prio = maxp;
  } else if ((sq->child[0]->closed == CLOSED_SUPER) &&
             (sq->child[1]->closed == CLOSED_SUPER) &&
             (sq->child[2]->closed == CLOSED_SUPER) &&
             (sq->child[3]->closed == CLOSED_SUPER)) {
    sq->closed = CLOSED_SUPER;
    sq->prio = 0;
#if 0
	} else if ((!in[0] || sq->child[0]->closed >= SSCLOSED) &&
		   (!in[1] || sq->child[1]->closed >= SSCLOSED) &&
		   (!in[2] || sq->child[2]->closed >= SSCLOSED) &&
		   (!in[3] || sq->child[3]->closed >= SSCLOSED)) {
		sq->closed = SSCLOSED;
		sq->prio = 0;
#endif
  } else {
    sq->closed = SSCLOSED;
    sq->prio = 0;
  }
  if (sq->parent)
    SSquareRecomputeStats(sq->parent);
}

SSquare *SSquareInstall(xp, yp, xs, ys, d, parent) int xp, yp, xs, ys, d;
SSquare *parent;
{
  SSquare *new;

  new = SSquareCreate(xp, yp, xs, ys, d, parent);
  SSquareDraw(new);
  return new;
}

SSquare *SSquareSelect(list) SSquare *list;
{
  int i;
  SSquare *res, *which;

  /*
   * If mousebutton is pressed,
   * find the square in which the mouse is located and
   * return that if not a leaf (pixel-sized square).
   */
  if (GraphicsLeftMouseEvent()) {
    SuperSampleMode = SuperSampling;
    if ((res = SSquareFetchAtMouse(list)) != (SSquare *)NULL)
      return res;
  } else if (GraphicsRightMouseEvent()) {
    SuperSampleMode = 1;
    if ((res = SSquareFetchAtMouse(list)) != (SSquare *)NULL) {
      return res;
    }
  }
  if (list->closed >= SSCLOSED) {
    if (Rectmode) {
      Rectmode = FALSE;
      RecomputePriority(SSquares);
      return SSquareSelect(list);
    }

    if (list->closed >= ALL_SUPERSAMPLED)
      return (SSquare *)NULL;
    else {
      fprintf(stderr, "Image sampled to one pixel. Supersampling...\n");
      SuperSampling = 1;
      SuperSampleMode = 1;
      return SSquareSelect(list);
    }
  }
  /*
   * Otherwise, find the square with the greatest
   * 'priority'.
   */
  res = list;
  while (res && !res->leaf) {
    which = (SSquare *)NULL;
    for (i = 0; i < 4; i++) {
      if ((res->child[i]->closed < SSCLOSED) && OVERLAPS_RECT(res->child[i])) {
        which = res->child[i];
        break;
      }
    }
    while (++i < 4) {
      if ((res->child[i]->closed < SSCLOSED) &&
          which->prio < res->child[i]->prio && OVERLAPS_RECT(res->child[i]))
        which = res->child[i];
    }
    res = which;
  }
  return res;
}

SSquare *SSquareFetchAtMouse(list) SSquare *list;
{
  SSquare *res;
  int x, y;

  /*
   * Get mouse position.
   */
  GraphicsGetMousePos(&x, &y);
  res = list;
  while (!res->leaf && (res->closed < SSCLOSED)) {
    /*
     * Find in which child the mouse is located.
     */
    if (x < res->child[1]->xpos) {
      if (y < res->child[2]->ypos)
        res = res->child[0];
      else
        res = res->child[2];
    } else if (y < res->child[3]->ypos)
      res = res->child[1];
    else
      res = res->child[3];
  }
  if (res->closed >= SSCLOSED)
    return (SSquare *)NULL;
  return res;
}

void SSquareDraw(sq) SSquare *sq;
{
  if (SQ_AREA(sq) >= MINAREA)
    GraphicsDrawRectangle(sq->xpos, sq->ypos, sq->xsize, sq->ysize,
                          Image[sq->ypos][sq->xpos],
                          Image[sq->ypos][sq->xpos + sq->xsize],
                          Image[sq->ypos + sq->ysize][sq->xpos + sq->xsize],
                          Image[sq->ypos + sq->ysize][sq->xpos]);
  else
    DrawPixels(sq->xpos, sq->ypos, sq->xsize, sq->ysize);
  if (!sq->leaf) {
    SSquareDraw(sq->child[0]);
    SSquareDraw(sq->child[1]);
    SSquareDraw(sq->child[2]);
    SSquareDraw(sq->child[3]);
  }
}

void DrawPixels(xp, yp, xs, ys) int xp, yp, xs, ys;
{
  int x, y, xi, yi;

  yi = yp;
  for (y = 0; y <= ys; y++, yi++) {
    xi = xp;
    for (x = 0; x <= xs; x++, xi++) {
      SSquareSample(xi, yi, SuperSampleMode);
      GraphicsDrawPixel(xi, yi, Image[yi][xi]);
    }
  }
}
