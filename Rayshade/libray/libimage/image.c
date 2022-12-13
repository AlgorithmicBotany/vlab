/*
 * image.c
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
#include <stdio.h>
#include <string.h>
#include "libcommon/common.h"
#include "image.h"
#include "lodepng.h"
#ifdef URT
#include "rle.h"
#endif

Image *image_list = NULL; /* Linked list of images */

Image *ImageCreate(filename)const char *filename;
{
  Image *new;

  new = (Image *)Malloc(sizeof(Image));
  new->filename = strsave(filename);
  new->width = 0;
  new->height = 0;
  new->chan = 0;
  new->data = NULL;
  new->next = image_list;
  image_list = new;
  return new;
}

Image *ImageFind(name) const char *name;
{
  Image *im;

  for (im = image_list; im; im = im->next) {
    if (strcmp(im->filename, name) == 0)
      return im;
  }

  return (Image *)NULL;
}

#ifdef URT
Image *ImageRead(filename) char *filename;
{
  FILE *fp;
  int i, y, chan;
  rle_hdr in_hdr;
  Image *image;
  rle_pixel **inrows;

  /*
   * See if image has been read before.
   */
  image = ImageFind(filename);
  if (image)
    return image;

  fp = fopen(filename, "r");
  if (fp == (FILE *)NULL) {
    RLerror(RL_ABORT, "Cannot open RLE file %s.\n", filename);
    return (Image *)NULL;
  }

  in_hdr.rle_file = fp;

  /* Try to open the RLE file */
  if (rle_get_setup(&in_hdr) < 0) {
    RLerror(RL_ABORT, "Error reading header of %s\n", filename);
    return (Image *)NULL;
  }

  /*
   * Read new image
   */
  image = ImageCreate(filename);

  in_hdr.xmax -= in_hdr.xmin;
  in_hdr.xmin = 0;
  image->width = in_hdr.xmax + 1;
  image->height = in_hdr.ymax - in_hdr.ymin + 1;
  image->chan = in_hdr.ncolors;
  image->has_alpha = in_hdr.alpha ? 1 : 0;
  image->totalchan = image->chan + image->has_alpha;
  image->chansize = image->width * image->height;

  image->data = (unsigned char *)Malloc(image->chansize * image->totalchan *
                                        sizeof(unsigned char));

  /*
   * Allocate array of pointers to pass to rle_getrow.
   */
  inrows = (rle_pixel **)Malloc(image->totalchan * sizeof(rle_pixel *));
  /*
   * Set inrows to point to appropriate initial location in image.
   */
  inrows[0] = (rle_pixel *)image->data;
  for (i = 1; i < image->totalchan; i++)
    inrows[i] = inrows[i - 1] + image->chansize;
  if (image->has_alpha)
    /* Alpha channel lives in channel -1 */
    inrows++;

  /* Read the image */
  for (y = 0; y < image->height; y++) {
    rle_getrow(&in_hdr, inrows);
    /*
     * Update inrows to point to next scanline for
     * each channel.
     */
    for (i = 0; i < image->chan; i++)
      inrows[i] += image->width;
    if (image->has_alpha)
      inrows[-1] += image->width;
  }

  (void)fclose(fp);
  return image;
}

#elif defined(PNG)

Image* ImageRead(const char* filename){
  unsigned error;
  unsigned char* loaded_image;
  unsigned char *imgDataChannels[4]; /* Can't have more than 4 channels */

  unsigned width, height;
  Image *image;
  size_t jump = 1;
  unsigned int x, y,i,idx,chan;
  int mipmapImageAllocSize;

  
  /*
   * See if image has been read before.
   */
  image = ImageFind(filename);
  if (image)
    return image;
  error = lodepng_decode32_file(&loaded_image , &width, &height, filename);
  if(error)
  {
    printf("error %u: %s\n", error, lodepng_error_text(error));
    return NULL;
  }
  image = NULL;
  
  image = ImageCreate(filename);
  image->chan = 3;
  image->has_alpha = 1;
  image->width = width;
  image->height = height;
  image->totalchan = image->chan + image->has_alpha;
  image->chansize = image->width * image->height;
  image->data = (unsigned char *)
    Malloc(image->chansize * image->totalchan * sizeof(unsigned char));

  //printf("%d - %d - %d\n", image->chan,  image->chansize , image->totalchan );
  if(image->has_alpha)
    imgDataChannels[image->chan] = &(image->data[0]);
  for(i = 0; i < image->chan ; i++)
    imgDataChannels[i] = &(image->data[(i + image->has_alpha) *
				       image->chansize *
				       sizeof(unsigned char)]);


   /*plot the pixels of the PNG file*/
  //printf("Plot png\n");
  for(y = 0 , idx = 0 ; y < image->height ; y++)
    for(x = 0 , i = 0 ; x < image->width ; x++ , idx++)
      for(chan = 0 ; chan < image->totalchan ; chan++ , i++){
	//printf("%d - %d - %d - %d - %d\n",y,x,chan, idx,4 * (height - y - jump) * width + 4 * (x) + chan);
	imgDataChannels[chan][idx] = loaded_image[4 * (height - y - jump) * width + 4 * (x) + chan];
      }


  free(loaded_image);
  /*
  
  // now copy loaded_image into image->data
  image->data = loaded_image;
  image->chan = 3;
  image->has_alpha = 1;

  image->totalchan = image->chan + image->has_alpha;
  image->chansize = image->width * image->height;
  */

  return image;
}


/* Image *ImageRead(filename) char *filename; */
/* { */
/*   FILE *fp; */
/*   int i, idx, x, y, chan; */
/*   unsigned char header[8]; */
/*   int numRead = 8; */

/*   png_structp read_ptr; */
/*   png_infop read_info_ptr; */
/*   png_uint_32 tWidth, tHeight; */
/*   int bitDepth, colorType; */

/*   Image *image; */
/*   unsigned char *imgDataChannels[4]; /\* Can't have more than 4 channels *\/ */
/*   png_bytepp pngRows; */

/*   /\* */
/*    * See if image has been read before. */
/*    *\/ */
/*   image = ImageFind(filename); */
/*   if (image) */
/*     return image; */

/*   fp = fopen(filename, "r"); */
/*   if (fp == (FILE *)NULL) { */
/*     RLerror(RL_ABORT, "Cannot open file %s.\n", filename); */
/*     return (Image *)NULL; */
/*   } */

/*   numRead = fread(header, 1, numRead, fp); */
/*   if (png_sig_cmp(header, 0, numRead)) */
/*     RLerror(RL_ABORT, "File %s is not a PNG.\n", filename); */

/*   read_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL); */
/*   if (!read_ptr) */
/*     RLerror(RL_ABORT, "Could not allocate libpng structure.\n"); */

/*   read_info_ptr = png_create_info_struct(read_ptr); */
/*   if (!read_info_ptr) { */
/*     png_destroy_read_struct(&read_ptr, NULL, NULL); */
/*     RLerror(RL_ABORT, "Could not allocate libpng structure.\n"); */
/*   } */

/*   if (setjmp(png_jmpbuf(read_ptr))) { */
/*     png_destroy_read_struct(&read_ptr, &read_info_ptr, NULL); */
/*     fclose(fp); */
/*     RLerror(RL_ABORT, "LibPNG error.\n"); */
/*   } */

/*   png_init_io(read_ptr, fp); */
/*   png_set_sig_bytes(read_ptr, numRead); */
/*   png_read_png(read_ptr, read_info_ptr, */
/*                PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_STRIP_16, NULL); */

/*   image = ImageCreate(filename); */
/*   png_get_IHDR(read_ptr, read_info_ptr, &tWidth, &tHeight, &bitDepth, */
/*                &colorType, NULL, NULL, NULL); */
/*   image->width = tWidth; */
/*   image->height = tHeight; */
/*   switch (colorType) { */
/*   case PNG_COLOR_TYPE_GRAY: */
/*     image->chan = 1; */
/*     image->has_alpha = 0; */
/*     break; */

/*   case PNG_COLOR_TYPE_GRAY_ALPHA: */
/*     image->chan = 1; */
/*     image->has_alpha = 1; */
/*     break; */

/*   case PNG_COLOR_TYPE_RGB: */
/*     image->chan = 3; */
/*     image->has_alpha = 0; */
/*     break; */

/*   case PNG_COLOR_TYPE_RGB_ALPHA: */
/*     image->chan = 3; */
/*     image->has_alpha = 1; */
/*     break; */

/*   default: */
/*     RLerror(RL_ABORT, "Unknown PNG color type %d.\n", colorType); */
/*   } */
/*   image->totalchan = image->chan + image->has_alpha; */
/*   image->chansize = image->width * image->height; */
/*   image->data = (unsigned char *)Malloc(image->chansize * image->totalchan * */
/*                                         sizeof(unsigned char)); */

/*   if (image->has_alpha) */
/*     imgDataChannels[image->chan] = &(image->data[0]); */
/*   for (i = 0; i < image->chan; i++) */
/*     imgDataChannels[i] = */
/*         &(image->data[(i + image->has_alpha) * image->chansize * */
/*                       sizeof(unsigned char)]); */

/*   /\*** As recommended in the libpng manual, but doesn't work: */
/*    * */
/*   pngData = (png_bytep) Malloc(image->chansize * image->totalchan * */
/*                                sizeof(png_byte)); */
/*   pngRows = (png_bytep*) Malloc(image->height * sizeof(png_bytep)); */
/*   for(i = 0 ; i < image->height ; i++) */
/*     pngRows[i] = &(pngData[i * image->width * image->totalchan]); */
/*   png_set_rows(read_ptr,read_info_ptr,pngRows); */
/*   ***\/ */

/*   pngRows = png_get_rows(read_ptr, read_info_ptr); */

/*   for (y = 0, idx = 0; y < image->height; y++) */
/*     for (x = 0, i = 0; x < image->width; x++, idx++) */
/*       for (chan = 0; chan < image->totalchan; chan++, i++) */
/*         imgDataChannels[chan][idx] = pngRows[image->height - y - 1][i]; */

/*   png_destroy_read_struct(&read_ptr, &read_info_ptr, NULL); */
/*   fclose(fp); */

/*   return image; */
/* } */

#else /* !URT , !PNG */

Image *ImageRead(filename) char *filename;
{
  FILE *fp;
  char buf[80];
  Image *image;
  int y, x;
  unsigned char *rbuf, *gbuf, *bbuf;

  image = ImageFind(filename);
  if (image)
    return image;

  fp = fopen(filename, "r");
  if (fp == (FILE *)NULL) {
    RLerror(RL_ABORT, "Cannot open image file %s.\n", filename);
    return (Image *)NULL;
  }

  image = ImageCreate(filename);
  /*
   * Read image header.
   */
  if (fgets(buf, 100, fp) == (char *)NULL ||
      sscanf(buf, "%d %d\n", &image->width, &image->height) != 2) {
    RLerror(RL_ABORT, "Cannot read header of image file %s.\n", filename);
    fclose(fp);
    return (Image *)NULL;
  }
  /*
   * Generic image files always have 3 channels, no alpha.
   */
  image->chan = image->totalchan = 3;
  image->has_alpha = 0;
  image->chansize = image->width * image->height;

  image->data = (unsigned char *)Malloc(image->chansize * image->totalchan *
                                        sizeof(unsigned char));

  rbuf = image->data;
  gbuf = &image->data[image->chansize];
  bbuf = &image->data[image->chansize + image->chansize];
  for (y = 0; y < image->height; y++) {
    for (x = 0; x < image->width; x++) {
      *(rbuf++) = getc(fp);
      *(gbuf++) = getc(fp);
      *(bbuf++) = getc(fp);
      if (feof(fp)) {
        RLerror(RL_ABORT, "Error reading image %s\n", filename);
        fclose(fp);
        return (Image *)NULL;
      }
    }
  }

  (void)fclose(fp);
  return image;
}
#endif

void ImageIndex(img, ix, iy, fx, fy, smooth, outval) Image *img;
int ix, iy, smooth;
Float fx, fy;
Float outval[4];
{
  int xplus, yplus, chan, offset;
  Float x0y0, x1y0, x0y1, x1y1;
  unsigned char *data;

  if (smooth) {
    /*
     * bi-linear interp of four pixels.  Note this blends
     * the top with the bottom, and the left with the right.
     */
    if (ix == img->width - 1)
      xplus = 1 - img->width;
    else
      xplus = 1;
    if (iy == img->height - 1)
      yplus = (1 - img->height) * img->width;
    else
      yplus = img->width;
    data = img->data;
    /* compute offset into first channel */
    offset = ix + iy * img->width;
    for (chan = 0; chan < img->totalchan; chan++) {
      x0y0 = (Float)data[offset] / 255.0;
      x1y0 = (Float)data[offset + xplus] / 255.0;
      x0y1 = (Float)data[offset + yplus] / 255.0;
      x1y1 = (Float)data[offset + xplus + yplus] / 255.0;
      outval[chan] =
          (x0y0 * (1.0 - fx) * (1.0 - fy) + x1y0 * (fx) * (1.0 - fy) +
           x0y1 * (1.0 - fx) * (fy) + x1y1 * (fx) * (fy));
      /* Make offset point to next channel */
      offset += img->chansize;
    }
  } else {
    /*
     * Hard edged image pixels (rectangles)
     * Compute offset into first channel
     */
    offset = ix + iy * img->width;
    for (chan = 0; chan < img->totalchan; chan++) {
      outval[chan] = (Float)img->data[offset] / 255.0;
      /* Make offset point to next channel */
      offset += img->chansize;
    }
  }
}
