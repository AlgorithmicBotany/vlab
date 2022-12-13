/*
 * imagetext.c
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
#include "texture.h"
#include "libimage/image.h"
#include "imagetext.h"

#define INTERP(v) (text->lo + (v) * (text->hi - text->lo))

/*
 * Create Image texture.
 * Image texture has so many options that there's usually no
 * nice way to parse without resorting to additional keywords.
 * Thus the ImageTextCreate routine only reads the image to be used;
 * all but ->component must be set by hand.  The routine
 * ImageTextSetComponent is provided to set the component and
 * ensure that the image being used is appropriate.
 */
ImageText *ImageTextCreate(imagefile) char *imagefile;
{
  ImageText *text;

  text = (ImageText *)Calloc(1, sizeof(ImageText));
  text->component = COLOR; /* texture all colors by default*/
  /*
   * Only apply one copy of the texture by default
   */
  text->tileu = text->tilev = 0;
  text->lo = 0.;
  text->hi = 1.;
  text->smooth = FALSE;
  text->mapping = UVMappingCreate();
  text->image = ImageRead(imagefile);
  return text;
}

/*
 * Set image texture to apply to given component,
 * ensuring that the image that is being used is compatible.
 */
void ImageTextSetComponent(text, component) ImageText *text;
int component;
{
  switch (component) {
  case COLOR: /* usual case, texture on amb, diff, spec, body */
  case AMBIENT:
  case DIFFUSE:
  case SPECULAR:
  case BODY:
    /* all of the above can use 1 or 3 channel images */
    if (text->image->chan != 1 && text->image->chan != 3) {
      RLerror(RL_ABORT, "Image %s must have 1 or 3 channels\n",
              text->image->filename);
    }
    break;
  case REFLECT:
  case TRANSP:
  case SPECPOW:
  case BUMP:
  case INDEX:
    /* the above need 1 channel images */
    if (text->image->chan != 1) {
      RLerror(RL_ABORT, "Image %s must have 1 channel\n",
              text->image->filename);
    }
    break;
  }
  text->component = component;
}

void ImageTextApply(ImageText *text, Geom *prim,
                    __attribute__((unused)) Ray *ray, Vector *pos, Vector *norm,
                    Vector *gnorm, Surface *surf) {
  Float fx, fy;
  Float outval[4], outval_u[4], outval_v[4];
  Float u, v;
  Surface tmpsurf;
  int ix, iy;
  int rchan, gchan, bchan;
  Vector dpdu, dpdv;

  /*
   * First, find the floating point location in image coords.
   * Then set ix, iy to the integer location in image coords and
   * use fx, fy to represent the subpixel position.
   */
  if (text->component == BUMP)
    TextToUV(text->mapping, prim, pos, gnorm, &u, &v, &dpdu, &dpdv);
  else
    TextToUV(text->mapping, prim, pos, gnorm, &u, &v, (Vector *)NULL,
             (Vector *)NULL);
  /*
   * Handle tiling at this point.
   */
  if (TileValue(text->tileu, text->tilev, u, v))
    return;
  u -= floor(u);
  v -= floor(v);
  fx = u * (Float)text->image->width;
  fy = v * (Float)text->image->height;
  ix = fx;
  iy = fy;
  fx = fx - (Float)ix;
  fy = fy - (Float)iy;

  if (text->image->has_alpha) {
    /* Alpha channel is 0 */
    rchan = 1;
    gchan = 2;
    bchan = 3;
  } else {
    rchan = 0;
    gchan = 1;
    bchan = 2;
  }

  if (text->image->chan == 1) {
    gchan = rchan;
    bchan = rchan;
  }

  ImageIndex(text->image, ix, iy, fx, fy, text->smooth, outval);

  if (text->component != COLOR || text->surf == (Surface *)NULL) {
    tmpsurf = *surf;
  } else {
    tmpsurf = *text->surf;
  }

  if (text->image->has_alpha && (outval[0] < 0.001)) {
    tmpsurf.amb.r = 0.0;
    tmpsurf.amb.g = 0.0;
    tmpsurf.amb.b = 0.0;
    tmpsurf.diff.r = 0.0;
    tmpsurf.diff.g = 0.0;
    tmpsurf.diff.b = 0.0;
    tmpsurf.spec.r = 0.0;
    tmpsurf.spec.g = 0.0;
    tmpsurf.spec.b = 0.0;
    tmpsurf.transp = 1.0;
    *surf = tmpsurf;
    return;
  }

  switch (text->component) {
  case COLOR: /* amb, diff, spec */
    tmpsurf.spec.r *= outval[rchan];
    tmpsurf.spec.g *= outval[gchan];
    tmpsurf.spec.b *= outval[bchan];
    tmpsurf.diff.r *= outval[rchan];
    tmpsurf.diff.g *= outval[gchan];
    tmpsurf.diff.b *= outval[bchan];
    tmpsurf.amb.r *= outval[rchan];
    tmpsurf.amb.g *= outval[gchan];
    tmpsurf.amb.b *= outval[bchan];
    break;
  case AMBIENT: /* ambient */
    tmpsurf.amb.r *= INTERP(outval[rchan]);
    tmpsurf.amb.g *= INTERP(outval[gchan]);
    tmpsurf.amb.b *= INTERP(outval[bchan]);
    break;
  case DIFFUSE: /* diffuse */
    tmpsurf.diff.r *= INTERP(outval[rchan]);
    tmpsurf.diff.g *= INTERP(outval[gchan]);
    tmpsurf.diff.b *= INTERP(outval[bchan]);
    break;
  case SPECULAR: /* specular */
    tmpsurf.spec.r *= INTERP(outval[rchan]);
    tmpsurf.spec.g *= INTERP(outval[gchan]);
    tmpsurf.spec.b *= INTERP(outval[bchan]);
    break;
  case BODY: /* transmitted */
    tmpsurf.body.r *= INTERP(outval[rchan]);
    tmpsurf.body.g *= INTERP(outval[gchan]);
    tmpsurf.body.b *= INTERP(outval[bchan]);
    break;
  case REFLECT: /* specular reflectivity */
    tmpsurf.reflect *= INTERP(outval[rchan]);
    break;
  case TRANSP: /* specular transmittance */
    tmpsurf.transp *= INTERP(outval[rchan]);
    break;
  case SPECPOW: /* specpow */
    tmpsurf.srexp *= INTERP(outval[rchan]);
    break;
  case INDEX: /* index of refraction */
    tmpsurf.index *= INTERP(outval[rchan]);
    break;
  case BUMP: /* bump map */
    ImageIndex(text->image, (ix == text->image->width - 1) ? 0 : ix + 1, iy, fx,
               fy, text->smooth, outval_u);
    ImageIndex(text->image, ix, (iy == text->image->height - 1) ? 0 : iy + 1,
               fx, fy, text->smooth, outval_v);
    MakeBump(norm, &dpdu, &dpdv, INTERP(outval_u[rchan] - outval[rchan]),
             INTERP(outval_v[rchan] - outval[rchan]));
    return;
  }

  if (text->image->has_alpha && (outval[0] < 0.999)) {
    /*
     * image partial coverage means blend surf and text->surf
     */
    SurfaceBlend(surf, &tmpsurf, 1. - outval[0], outval[0]);
  } else {
    /*
     * image full coverage means use text->surf
     */
    *surf = tmpsurf;
  }
}
