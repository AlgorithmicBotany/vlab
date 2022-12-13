/*
 * imagetext.h
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
#ifndef IMAGETEXT_H
#define IMAGETEXT_H

#define TextImageCreate(s)                                                     \
  TextCreate((TextRef)ImageTextCreate(s), ImageTextApply)

typedef struct {
  Image *image;       /* image to use */
  Surface *surf;      /* Alternative surface */
  int component,      /* component to texture */
      smooth;         /* interpolate pixel values? */
  Float lo, hi;       /* high and low values in image */
  Float tileu, tilev; /* tiling control */
  Mapping *mapping;
} ImageText;

extern ImageText *ImageTextCreate();
extern void ImageTextSetComponent();
void ImageTextApply(ImageText *text, Geom *prim, Ray *ray, Vector *pos,
                    Vector *norm, Vector *gnorm, Surface *surf);
#endif /* IMAGETEXT_H */
