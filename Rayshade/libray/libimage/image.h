/*
 * image.h
 *
 * Copyright (C) 1989, 1991, Rod G. Bogart, Craig E. Kolb
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
#ifndef IMAGE_H
#define IMAGE_H

/*
 * Generic image object for texture map storage.
 */
typedef struct Image {
  int width, height,   /* Image size */
      chan, has_alpha, /* # of channels, has alpha info? */
      totalchan,       /* # channels + any alpha channel */
      chansize;        /* width * height */
  unsigned char *data; /* Image */
  char *filename;      /* Filename (identifier) */
  struct Image *next;  /* Next image in list. */
} Image;

Image *ImageCreate(), *ImageFind(), *ImageRead();
void ImageIndex();

#endif /* IMAGE_H */
