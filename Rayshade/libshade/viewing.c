/*
 * viewing.c
 *
 * Copyright (C) 1989, 1991, Craig E. Kolb, Rod G. Bogart
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
#include "viewing.h"
#include "libcommon/sampling.h"
#include "options.h"
#include "defaults.h"
#include "picture.h"
#include "stats.h"

RSCamera Camera;
RSScreen Screen;

void SampleScreen(), SampleScreenFiltered();

void RSViewing() {
  Float magnitude;

  VecSub(Camera.lookp, Camera.pos, &Camera.dir);
  Screen.firstray = Camera.dir;

  Camera.lookdist = VecNormalize(&Camera.dir);
  if (VecNormCross(&Camera.dir, &Camera.up, &Screen.scrni) == 0.)
    RLerror(RL_PANIC, "The view and up directions are identical?\n");
  (void)VecNormCross(&Screen.scrni, &Camera.dir, &Screen.scrnj);

  /*
   * Add stereo separation if desired.
   */
  if (Options.stereo) {
    if (Options.stereo == LEFT)
      magnitude = -.5 * Options.eyesep;
    else
      magnitude = .5 * Options.eyesep;
    Camera.pos.x += magnitude * Screen.scrni.x;
    Camera.pos.y += magnitude * Screen.scrni.y;
    Camera.pos.z += magnitude * Screen.scrni.z;
    VecSub(Camera.lookp, Camera.pos, &Screen.firstray);
    Camera.dir = Screen.firstray;
    Camera.lookdist = VecNormalize(&Camera.dir);
    (void)VecNormCross(&Camera.dir, &Camera.up, &Screen.scrni);
    (void)VecNormCross(&Screen.scrni, &Camera.dir, &Screen.scrnj);
  }

  magnitude =
      2. * Camera.lookdist * tan(deg2rad(0.5 * Camera.hfov)) / Screen.xres;

  VecScale(magnitude, Screen.scrni, &Screen.scrnx);
  magnitude =
      2. * Camera.lookdist * tan(deg2rad(0.5 * Camera.vfov)) / Screen.yres;
#ifndef URT
  /*
   * If using "generic" file format, render top-to-bottom (yick).
   */
  magnitude *= -1;
#endif
  VecScale(magnitude, Screen.scrnj, &Screen.scrny);

  Screen.firstray.x -=
      0.5 * Screen.yres * Screen.scrny.x + 0.5 * Screen.xres * Screen.scrnx.x;
  Screen.firstray.y -=
      0.5 * Screen.yres * Screen.scrny.y + 0.5 * Screen.xres * Screen.scrnx.y;
  Screen.firstray.z -=
      0.5 * Screen.yres * Screen.scrny.z + 0.5 * Screen.xres * Screen.scrnx.z;

  if (Camera.focaldist == UNSET)
    Camera.focaldist = Camera.lookdist;
}

/*
 * Adjust the initial ray to account for an aperture and a focal
 * distance.  The ray argument is assumed to be an initial ray, and
 * always reset to the eye point.  It is assumed to be unit length.
 */
void focus_blur_ray(ray) Ray *ray;
{
  Vector circle_point, aperture_inc;
  extern void UnitCirclePoint();
  /*
   * Find a point on a unit circle and scale by aperture size.
   * This simulates rays passing thru different parts of the aperture.
   * Treat the point as a vector and rotate it so the circle lies
   * in the plane of the screen.  Add the aperture increment to the
   * starting position of the ray.  Stretch the ray to be focaldist
   * in length.  Subtract the aperture increment from the end of the
   * long ray.  This insures that the ray heads toward a point at
   * the specified focus distance, so that point will be in focus.
   * Normalize the ray, and that's it.  Really.
   */
  UnitCirclePoint(&circle_point, ray->sample);
  VecComb(Camera.aperture * circle_point.x, Screen.scrni,
          Camera.aperture * circle_point.y, Screen.scrnj, &aperture_inc);
  VecAdd(aperture_inc, Camera.pos, &(ray->pos));
  VecScale(Camera.focaldist, ray->dir, &(ray->dir));
  VecSub(ray->dir, aperture_inc, &(ray->dir));
  (void)VecNormalize(&ray->dir);
}

void ViewingSetup() {
#define SWAP(a, b) (tmp = (a), (a) = (b), (b) = tmp)

  Float tmp;
  int xwidth, ywidth;

  if (Options.stereo && Options.eyesep == UNSET)
    RLerror(RL_PANIC, "No eye separation specified for stereo rendering.\n");
  /*
   * Because we want the user to be able to override the input file
   * through the command line, we have to initialize some variables to
   * bogus values so that when the file is being parsed, it is
   * possible to tell if a given variable has been set on the
   * command line.
   *
   * If such variables are not set to legal values on the command
   * line or in the input file, we must do it here.
   */
  if (Screen.xres == UNSET)
    Screen.xres = XRESOLUTION;
  if (Screen.yres == UNSET)
    Screen.yres = YRESOLUTION;

  /*
   * The window to be rendered is defined by applying
   * the crop window to the sub window.  The subwindow
   * is defined using pixel numbers, and must be within
   * [0, xres -1],[0, yres -1].  The default is the entire
   * screen.  The crop window is specified using normalized
   * coordinates.
   */

  if (!Options.window_set) {
    /* If no window set, set equal to entire screen. */
    Options.window[LOW][X] = Options.window[LOW][Y] = 0;
    Options.window[HIGH][X] = Screen.xres - 1;
    Options.window[HIGH][Y] = Screen.yres - 1;
  }

  /* Truncate crop window to legal limits. */
  if (Options.crop[LOW][X] > Options.crop[HIGH][X])
    SWAP(Options.crop[LOW][X], Options.crop[HIGH][X]);
  if (Options.crop[LOW][Y] > Options.crop[HIGH][Y])
    SWAP(Options.crop[LOW][Y], Options.crop[HIGH][Y]);
  if (Options.crop[LOW][X] < 0.)
    Options.crop[LOW][X] = 0.;
  if (Options.crop[LOW][Y] < 0.)
    Options.crop[LOW][Y] = 0.;
  if (Options.crop[HIGH][X] > 1.)
    Options.crop[HIGH][X] = 1.;
  if (Options.crop[HIGH][Y] > 1.)
    Options.crop[HIGH][Y] = 1.;

  xwidth = Options.window[HIGH][X] - Options.window[LOW][X];
  ywidth = Options.window[HIGH][Y] - Options.window[LOW][Y];

  /* Compute x and y extents of window to be renered. */
  Screen.minx = (int)(Options.window[LOW][X] + Options.crop[LOW][X] * xwidth);
  Screen.maxx = (int)(Options.window[LOW][X] + Options.crop[HIGH][X] * xwidth);
  Screen.miny = (int)(Options.window[LOW][Y] + Options.crop[LOW][Y] * ywidth);
  Screen.maxy = (int)(Options.window[LOW][Y] + Options.crop[HIGH][Y] * ywidth);

#ifdef URT
  /*
   * If using the URT, we should use the RLE file header to
   * determine cropped window size.  Screen size
   * (Screen.xres, Screen.yres) is determined from command
   * line or input file, as usual.
   *
   * If the cropped window computed in PictureSetWindow()
   * is not equal the cropped window computed above,
   * a warning message is issued.
   */
  if (Options.appending) {
    /*
     * Read image header to determine window size.
     */
    PictureSetWindow();
  }
#endif

  Screen.xsize = Screen.maxx - Screen.minx + 1;
  Screen.ysize = Screen.maxy - Screen.miny + 1;

  /*
   * Sanity check.
   */
  if (Screen.minx < 0 || Screen.miny < 0 || Screen.maxx >= Screen.xres ||
      Screen.maxy >= Screen.yres)
    RLerror(RL_PANIC, "Invalid window specification.\n");

  /*
   * If not defined in the input file, calculate VFOV
   * by hand.  This assumes that pixels are square, which is
   * probably a bad idea.  ("aspect" option?)
   */
  if (Camera.vfov == UNSET)
    Camera.vfov = Camera.hfov * Screen.yres / Screen.xres;
}

void SampleScreenFiltered(x, y, u, v, ray, color, sample) Float x, y;
Ray *ray;
Pixel *color;
int sample, u, v;
{
  SampleScreen(x, y, ray, color, sample);
  color->r *= Sampling.filter[u][v];
  color->g *= Sampling.filter[u][v];
  color->b *= Sampling.filter[u][v];
  color->alpha *= Sampling.filter[u][v];
}

void SampleScreen(x, y, ray, color, sample) Float x,
    y;        /* Screen position to sample */
Ray *ray;     /* ray, with origin and medium properly set */
Pixel *color; /* resulting color */
int sample;   /* sample number */
{
  Float dist;
  HitList hitlist;
  Color ctmp, fullintens;
  extern void focus_blur_ray(), ShadeRay();

  /*
   * Calculate ray direction.
   */
  Stats.EyeRays++;
  ray->dir.x = Screen.firstray.x + x * Screen.scrnx.x + y * Screen.scrny.x;
  ray->dir.y = Screen.firstray.y + x * Screen.scrnx.y + y * Screen.scrny.y;
  ray->dir.z = Screen.firstray.z + x * Screen.scrnx.z + y * Screen.scrny.z;

  (void)VecNormalize(&ray->dir);

  ray->sample = sample;

  if (Camera.aperture > 0.0) {
    /*
     * If the aperture is open, adjust the initial ray
     * to account for depth of field.
     */
    focus_blur_ray(ray);
  }

  /*
   * Do the actual ray trace.
   */
  fullintens.r = fullintens.g = fullintens.b = 1.;
  dist = FAR_AWAY;
  hitlist.nodes = 0;
  (void)TraceRay(ray, &hitlist, EPSILON, &dist);
  ShadeRay(&hitlist, ray, dist, &Screen.background, &ctmp, &fullintens);
  color->r = ctmp.r;
  color->g = ctmp.g;
  color->b = ctmp.b;
  if (hitlist.nodes != 0) {
    color->alpha = 1.;
  } else {
    color->alpha = 0.;
  }
}
