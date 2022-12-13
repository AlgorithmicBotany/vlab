
#ifdef WIN32
#include <image.h>
#else
#include "image.h"
#endif

#include "targa.h"
#include "patches.h"

struct image_data_type {
  int size[2];    /* size of the image */
  float range[2]; /* size in coordinates */
  float pos[2];   /* position of lower left front corner */
  unsigned char *data;
  unsigned char *input_image_name;
};
typedef struct image_data_type image_data_type;

#define X 0
#define Y 1

static image_data_type sky = {0};

/****************************************************************************/
void FreeImageData(void) {
  if (sky.data != NULL) {
    free(sky.data);
    sky.data = NULL;
  }

  if (sky.input_image_name != NULL) {
    free(sky.input_image_name);
    sky.input_image_name = NULL;
  }
}

/****************************************************************************/
int GetImageData(float x, float y, float *val) {
  unsigned char *ptr;
  int xn, yn;

  if (sky.data == NULL || x < sky.pos[X] || x >= sky.pos[X] + sky.range[X] ||
      y < sky.pos[Y] || y >= sky.pos[Y] + sky.range[Y]) {
    val[0] = val[1] = 0;
    return 0;
  }

  xn = floor((x - sky.pos[X]) / sky.range[X]);
  yn = floor((y - sky.pos[Y]) / sky.range[Y]);

  ptr = &sky.data[yn * sky.size[X] * 2 + xn * 2];

  val[0] = (float)ptr[0] / 255.0;
  val[1] = (float)ptr[1] / 255.0;

  return 1;
}

/****************************************************************************/
int LoadImageData(char *input_image_name) {
  int x, y;
  float xratio, yratio;
  IMAGE *image;
  unsigned char *ptr;
  unsigned short *buf;
  targa_params_type TGAspec;
  int c;

  if ((c = strlen(sky.input_image_name)) > 3)
    if (strcmp(sky.input_image_name + c - 3, "tga") == 0) {
      FILE *fp;
      unsigned char *rowbuf, tmp, *ptrc;

      /* load in targa image */
      if (!loadTGAhead(sky.input_image_name, &TGAspec)) {
        fprintf(stderr, "MonteCarlo: cannot open input targa image %s.\n",
                sky.input_image_name);
        return 0;
      }
      sky.size[X] = TGAspec.Xres;
      sky.size[Y] = TGAspec.Yres;

      if ((sky.data = (unsigned char *)malloc(sky.size[X] * sky.size[Y] * 2)) ==
          NULL) {
        fprintf(stderr, "MonteCarlo: cannot allocate memory for image %s!\n",
                sky.input_image_name);
        return 0;
      }

      if ((rowbuf = (unsigned char *)malloc(sky.size[X] * 3 *
                                            sizeof(unsigned char))) == NULL) {
        fprintf(stderr, "MonteCarlo: cannot allocate memory for image %s!\n",
                sky.input_image_name);
        return 0;
      }

      /* read the image in - starting with the last row */
      /* image is stored in rows of R, G, and B. We need R,G,B for each
         pixel */
      for (y = sky.size[Y] - 1; y >= 0; y--) {
        loadTGArow(&TGAspec, y, rowbuf);

        ptrc = sky.data + y * sky.size[X];
        for (x = 0; x < sky.size[X]; x++) {
          /* get green and red */
          *(ptrc++) = rowbuf[x * 3 + 1];
          *(ptrc++) = rowbuf[x * 3 + 2];
        }
      }

      loadTGAfinish(&TGAspec);
      free(rowbuf);

      fclose(fp);

      return 1;
    }

#ifndef SUN
  /* otherwice open rgb image */
  if ((image = iopen(sky.input_image_name, "r", 0, 0, 0, 0, 0)) == NULL) {
    fprintf(stderr, "MonteCarlo - cannot open image %s!\n",
            sky.input_image_name);
    return 0;
  }

  sky.size[X] = image->xsize;
  sky.size[Y] = image->ysize;

  /* one line buffer */
  if ((buf = (unsigned short *)malloc(image->xsize * 2)) == NULL) {
    fprintf(
        stderr,
        "MonteCarlo - cannot allocate memory for one channel of image %s!\n",
        sky.input_image_name);
    return 0;
  }

  ptr = sky.data;

  for (y = 0; y < image->ysize; y++) {
    /* red */
    getrow(image, buf, y, 0);
    /* green */
    getrow(image, buf + image->xsize, y, 0);

    for (x = 0; x < sky.size[X]; x++) {
      *(ptr++) = buf[x];
      *(ptr++) = buf[image->xsize + x];
    }
  }

  free(buf);
  iclose(image);
#else
  fprintf(stderr,
          "MonteCarlo: reading of rgb image not implemeted on suns!.\n");

  ptr = sky.data;

  for (y = 0; y < sky.size[Y]; y++)
    for (x = 0; x < sky.size[X]; x++) {
      *(ptr++) = 0;
      *(ptr++) = 0;
    }
#endif

  return 1;
}
