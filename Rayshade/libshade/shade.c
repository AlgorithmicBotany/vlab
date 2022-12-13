/*
 * shade.c
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
#include "libtext/texture.h"
#include "libsurf/surface.h"
#include "liblight/light.h"
#include "libsurf/atmosphere.h"
#include "options.h"
#include "stats.h"

Medium TopMedium;
Atmosphere *AtmosEffects;

static void shade( Vector *pos, Ray *ray, Vector  *nrm, int enter, Surface *surf, Color *back,Color *color,Color *contrib);
static void LightRay(Light *lp,Vector *pos, Vector *norm, Vector *reflect, Surface *surf, int depth, int samp,Float time, Color *color);
static void Lighting(), ReflectRay();
static int TransmitRay();

/*
 * Calculate color of ray.
 */
void ShadeRay(hitlist, ray, dist, back, color, contrib)
    HitList *hitlist; /* Information about point of intersection. */
Ray *ray;             /* Direction and origin of ray. */
Float dist;           /* Distance from origin of intersection. */
Color *back,          /* "Background" color */
    *color,           /* Color to assign current ray. */
    *contrib;         /* Contribution of this ray to final color */
{
  Vector norm, gnorm, pos; /* surface normal, point of intersection */
  Surface surf, *stmp;     /* surface properties */
  int enter, smooth;       /* entering ?, gnorm != snorm ?*/

  if (hitlist->nodes == 0) {
    /*
     * No valid intersection.  Set distance for atmospheric
     * effects and set color of ray to background.
     */
    *color = *back;
    VecAddScaled(ray->pos, FAR_AWAY, ray->dir, &pos);
    if (!ray->media && AtmosEffects)
      Atmospherics(AtmosEffects, ray, FAR_AWAY, &pos, color);
    return;
  }

  /*
   * Compute normal, surface properties, etc.
   */
  stmp = GetShadingSurf(hitlist);
  surf = *stmp;
  enter = ComputeSurfProps(hitlist, ray, &pos, &norm, &gnorm, &surf, &smooth);
  Stats.HitRays++;

  /*
   * Calculate ray color.
   */
  shade(&pos, ray, &norm, enter, &surf, back, color, contrib);
  if (!ray->media && AtmosEffects)
    Atmospherics(AtmosEffects, ray, dist, &pos, color);
}

/*
 * Perform lighting calculations based on surface normal & other properties,
 * incident ray direction and position, and light source properties.
 * Spawn any necessary reflected and transmitted rays.
 */
static void shade(pos, ray, nrm, enter, surf, back, color,
                  contrib) Vector *pos;
Vector  *nrm;     /* hit pos, shade normal, geo normal */
int enter;           /* TRUE if entering surface */
Ray *ray;            /* indicent ray */
Surface *surf;       /* properties of hit surface */
Color *back, *color; /* background color, computed color */
Color *contrib;      /* contribution to final pixel value */
{
  Float k; /* -ray . normal */
  Color newcontrib;
  Vector refl;          /* reflected direction */
  Color reflectivity,   /* effective surface reflectivity */
      intens;           /* reflected/transmitted intensity */
  Light *lp;            /* current light source */
  extern Light *Lights; /* list of defined sources */

  /*
   * Ambient color is always included.
   */
  ColorMultiply(surf->amb, Options.ambient, color);

  /*
   * Calculate direction of reflected ray.
   */
  k = -dotp(&ray->dir, nrm);
  VecAddScaled(ray->dir, 2. * k, *nrm, &refl);

  /*
   * Calculate intensity contributed by each light source.
   */
  for (lp = Lights; lp; lp = lp->next)
    LightRay(lp, pos, nrm, &refl, surf, ray->depth, ray->sample,
             ray->time, color);

  if (ray->depth >= Options.maxdepth)
    /*
     * Don't spawn any transmitted/reflected rays.
     */
    return;
  /*
   * Specular transmission (refraction).
   */
  ColorScale(surf->reflect, surf->spec, &reflectivity);

  if (surf->transp > EPSILON) {
    ColorScale(surf->transp, surf->body, &intens);
    ColorMultiply(intens, *contrib, &newcontrib);
    if (newcontrib.r > Options.cutoff.r || newcontrib.g > Options.cutoff.g ||
        newcontrib.b > Options.cutoff.b)
      /*
       * Transmit ray.  If TIR occurs, add transmitted
       * component to reflected component.  Kinda strange, but...
       */
      if (TransmitRay(ray, pos, nrm, k, surf->index, surf->statten, enter, back,
                      &newcontrib, &intens, color))
        ColorAdd(reflectivity, intens, &reflectivity);
  }

  if (reflectivity.r > EPSILON || reflectivity.g > EPSILON ||
      reflectivity.b > EPSILON) {
    ColorMultiply(reflectivity, *contrib, &newcontrib);
    if (newcontrib.r > Options.cutoff.r || newcontrib.g > Options.cutoff.g ||
        newcontrib.b > Options.cutoff.b)
      ReflectRay(ray, pos, &refl, back, &reflectivity, &newcontrib, color);
  }
}

/*
 * Lighting calculations
 */
static void LightRay(lp, pos, norm, reflect, surf, depth, samp, time,
                     color) Light *lp; /* Light source */
Vector *pos, *norm;                    /* hit pos, shade norm, geo norm */
Vector *reflect;                       /* reflection direction */
Surface *surf;                         /* surface characteristics */
int depth, samp;                       /* ray depth, sample # */
Float time;
Color *color; /* resulting color */
{
  Color lcolor;
  Ray newray;
  Float costheta, cosalpha, dist;

  newray.pos = *pos;
  newray.depth = depth;
  newray.sample = samp;
  newray.time = time;
  newray.media = (Medium *)NULL;

  LightDirection(lp, pos, &newray.dir, &dist);

  costheta = dotp(&newray.dir, norm);

  //[PASCAL/MIK] Problem with computation of normals when using a texture
  // The only way we found to correct the issue it's by commenting this part of
  // the code we don't know how to fix this bug !!!

  // If (smooth) {
  /* if (0){ */
  /*   cosalpha = dotp(&newray.dir, gnorm);  */
  /* 	/\* */
  /* 	 * If shading normal indicates self-shadowing */
  /* 	 * and geom normal indicates no self-shadowing, */
  /* 	 * trust the geom normal. */
  /* 	 *\/ */
  /* 	if (costheta <= 0. && cosalpha > 0.) */
  /* 		costheta = cosalpha; */
  /* 	/\* */
  /* 	 * If geom normal indicates self-shadowing and */
  /* 	 * geom normal doesn't, then have to do something */
  /* 	 * clever ala Snyder & Barr. */
  /* 	 *\/ */
  /* } */

  if (costheta <= 0.) {
    /*
     * Light source is on opposite side of surface,
     * hence light must be transmitted through...
     */
    if (surf->translucency < EPSILON)
      return;
    if (!LightIntens(lp, &newray, dist, (int)surf->noshadow, &lcolor))
      return;
    cosalpha = -dotp(reflect, &newray.dir);
    Lighting(-costheta, cosalpha, &lcolor, &surf->translu, &surf->body,
             surf->stexp, color);
    ColorScale(surf->translucency, *color, color);
  } else {
    if (!LightIntens(lp, &newray, dist, (int)surf->noshadow, &lcolor))
      return; /* prim is in shadow w.r.t light source */

    cosalpha = dotp(reflect, &newray.dir);
    Lighting(costheta, cosalpha, &lcolor, &surf->diff, &surf->spec, surf->srexp,
             color);
  }
}

/*
 * Compute shading function (diffuse reflection and specular highlight)
 *
 * This function *adds* the computed color to "color".
 */
static void Lighting(costheta, cosalpha, lcolor, diff, spec, coef,
                     color) Float costheta,
    cosalpha, coef;
Color *diff, *spec, *color, *lcolor;
{
  Float intens;

  /*
   * Diffuse reflection.
   * Falls off as the cosine of the angle between
   * the normal and the ray to the light (costheta).
   */
  color->r += diff->r * costheta * lcolor->r;
  color->g += diff->g * costheta * lcolor->g;
  color->b += diff->b * costheta * lcolor->b;
  /*
   * Specularly reflected highlights.
   * Fall off as the cosine of the angle
   * between the reflected ray and the ray to the light source.
   */
  if (coef < EPSILON || cosalpha <= 0.)
    return;
  /*
   * Specular highlight = cosine of the angle raised to the
   * appropriate power.
   */
  intens = pow(cosalpha, coef);
  color->r += spec->r * intens * lcolor->r;
  color->g += spec->g * intens * lcolor->g;
  color->b += spec->b * intens * lcolor->b;
}

/*
 * Spawn a transmitted ray.  Returns TRUE if total internal reflection
 * occurs, FALSE otherwise.
 */
static int TransmitRay(ray, pos, norm, k, index, statten, enter, back, contrib,
                       intens, color) Ray *ray;
Vector *pos, *norm;
Float k, index, statten;
int enter;
Color *back, *contrib, *intens, *color;
{
  int total_int_refl = FALSE;
  Ray NewRay;
  Float dist;
  Color newcol;
  HitList hittmp; /* Geom intersection record */

  NewRay.pos = *pos;         /* Origin == hit point */
  NewRay.media = ray->media; /* Media == old media */
  NewRay.sample = ray->sample;
  NewRay.time = ray->time;
  NewRay.depth = ray->depth + 1;

  if (enter) {
    /*
     * Entering surface.
     */
    if (Refract(&NewRay.dir,
                NewRay.media ? NewRay.media->index : TopMedium.index, index,
                &ray->dir, norm, k)) {
      total_int_refl = TRUE;
    } else {
      /*
       * Push information for new medium.
       */
      NewRay.media = MediumPush(index, statten, NewRay.media);
    }
  } else {
    /*
     * Exiting surface
     * Pop medium from stack.
     */
    if (NewRay.media != (Medium *)0)
      NewRay.media = NewRay.media->next;
    if (Refract(&NewRay.dir, index,
                NewRay.media ? NewRay.media->index : TopMedium.index, &ray->dir,
                norm, k)) {
      total_int_refl = TRUE;
    }
  }

  /*
   * At this point, NewRay.media is the medium into which
   * the new ray is entering.
   */

  if (!total_int_refl) {
    Stats.RefractRays++;
    hittmp.nodes = 0;
    dist = FAR_AWAY;
    TraceRay(&NewRay, &hittmp, EPSILON, &dist);
    ShadeRay(&hittmp, &NewRay, dist, back, &newcol, contrib);
    ColorMultiply(newcol, *intens, &newcol);
    /*
     * Attenuate transmitted color.  Note that
     * if the transmitted ray hit nothing, we still
     * perform this computation, as it's possible
     * that 'air' has a non-unit statten.
     */
    statten = NewRay.media ? NewRay.media->statten : TopMedium.statten;
    if (statten != 1.0) {
      statten = pow(statten, dist);
      ColorScale(statten, newcol, &newcol);
    }
    ColorAdd(*color, newcol, color);
    /* Free pushed medium */
    if (enter)
      free((voidstar)NewRay.media);
  }

  return total_int_refl;
}

static void ReflectRay(ray, pos, dir, back, intens, contrib, color) Ray *ray;
Vector *pos, *dir;
Color *back, *intens, *contrib, *color;
{
  Ray NewRay;
  HitList hittmp; /* Geom intersection record */
  Color newcol;
  Float dist;

  NewRay.pos = *pos;         /* Origin == hit point */
  NewRay.dir = *dir;         /* Direction == reflection */
  NewRay.media = ray->media; /* Medium == old medium */
  NewRay.sample = ray->sample;
  NewRay.time = ray->time;
  NewRay.depth = ray->depth + 1;
  Stats.ReflectRays++;
  hittmp.nodes = 0;
  dist = FAR_AWAY;
  (void)TraceRay(&NewRay, &hittmp, EPSILON, &dist);
  ShadeRay(&hittmp, &NewRay, dist, back, &newcol, contrib);
  ColorMultiply(newcol, *intens, &newcol);
  ColorAdd(*color, newcol, color);
}
