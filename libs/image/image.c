#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#include <malloc.h>
#include <memory.h> /* for memset */
#endif

#include "image.h"

#ifndef IMAGIC
#define IMAGIC 0x01da
#endif
#define IMAGIC_SWAP 0xda01

#define SWAP_SHORT_BYTES(x) ((((x)&0xff) << 8) | (((x)&0xff00) >> 8))
#define SWAP_LONG_BYTES(x)                                                     \
  (((((x)&0xff) << 24) | (((x)&0xff00) << 8)) |                                \
   ((((x)&0xff0000) >> 8) | (((x)&0xff000000) >> 24)))


static IMAGE *OpenForWriting(const char *file, const char *mode,
                             unsigned int type, unsigned int dim,
                             unsigned int xsize, unsigned int ysize,
                             unsigned int zsize);
IMAGE *iopen(const char *file, const char *mode, unsigned int type,
             unsigned int dim, unsigned int xsize, unsigned int ysize,
             unsigned int zsize) {
  IMAGE *pRes;
  FILE *fp;
  // get size of file

  if (mode[0] != 'r')
    /*return NULL; */
    return OpenForWriting(file, mode, type, dim, xsize, ysize, zsize);
  pRes = (IMAGE *)malloc(sizeof(IMAGE));
  if (NULL == pRes)
    return NULL;
  strcpy(pRes->name, file);
  memset(pRes, 0, sizeof(IMAGE));

  fp = fopen(file, "rb");
  if (NULL == fp) {
    free(pRes);
    return NULL;
  }

  pRes->file = fp;
  fread(pRes, 12, 1, fp);

  pRes->tmpbuf2 = NULL;
  /* Determine if valid image file and check the ENDIAN */
  if (pRes->imagic == IMAGIC_SWAP) {
    pRes->type = (uint16_t)(SWAP_SHORT_BYTES(pRes->type));
    pRes->dim = (uint16_t)(SWAP_SHORT_BYTES(pRes->dim));
    pRes->xsize = (uint16_t)(SWAP_SHORT_BYTES(pRes->xsize));
    pRes->ysize = (uint16_t)(SWAP_SHORT_BYTES(pRes->ysize));
    pRes->zsize = (uint16_t)(SWAP_SHORT_BYTES(pRes->zsize));
  } else if (pRes->imagic != IMAGIC) {
    /* Not a valid rgb file */
    free(pRes);
    fclose(fp);
    return NULL;
  }

  /* In case of RLE encoding */
  if (0x0100 == (0xFF00 & pRes->type)) {
    int x = pRes->ysize * pRes->zsize * sizeof(int32_t);
    pRes->rowstart = (uint32_t *)malloc(x);
    if (NULL == pRes->rowstart) {
      fclose(fp);
      free(pRes);
      return NULL;
    }
    pRes->rowsize = (int32_t *)malloc(x);
    if (NULL == pRes->rowsize) {
      free(pRes->rowstart);
      fclose(fp);
      free(pRes);
      return NULL;
    }
    pRes->tmpbuf2 = (unsigned char *)malloc(pRes->xsize * 256);
    if (NULL == pRes->tmpbuf2) {
      free(pRes->rowsize);
      free(pRes->rowstart);
      fclose(fp);
      free(pRes);
      return NULL;
    }
    pRes->tmpbuf = NULL;
    pRes->rleend = 512 + (2 * x);
    fseek(fp, 512, SEEK_SET);
    fread(pRes->rowstart, x, 1, fp);
    fread(pRes->rowsize, x, 1, fp);
    if (IMAGIC_SWAP == pRes->imagic) {

      uint32_t tmp;
      x /= sizeof(int32_t);
      x--;
      for (; x >= 0; x--) {
        tmp = pRes->rowstart[x];
        pRes->rowstart[x] = SWAP_LONG_BYTES(tmp);
        tmp = pRes->rowsize[x];
        pRes->rowsize[x] = SWAP_LONG_BYTES(tmp);
      }
    }
  } else {
    pRes->rowstart = NULL;
    pRes->rowsize = NULL;
    pRes->tmpbuf = NULL;
    pRes->tmpbuf2 = (unsigned char *)malloc(pRes->xsize);
    if (NULL == pRes->tmpbuf2) {
      fclose(fp);
      free(pRes);
      return NULL;
    }
  }

  return pRes;
}

IMAGE *OpenForWriting(const char *file, char *mode, unsigned int type,
                      unsigned int dim, unsigned int xsize, unsigned int ysize,
                      unsigned int zsize) {
  static char dummy[404];
  IMAGE *pRes = NULL;
  FILE *fp = NULL;

  {
    int i;
    for (i = 0; i < 404; i++)
      dummy[i] = 0;
  }

  pRes = (IMAGE *)malloc(sizeof(IMAGE));
  if (NULL == pRes)
    goto fail;
  memset(pRes, 0, sizeof(IMAGE));
  fp = fopen(file, "wb");
  if (NULL == fp)
    goto fail;

  pRes->imagic = IMAGIC;
  pRes->type = VERBATIM(2);
  pRes->dim = (uint16_t)dim;
  pRes->xsize = (uint16_t)xsize;
  pRes->ysize = (uint16_t)ysize;
  pRes->zsize = (uint16_t)zsize;

  pRes->tmpbuf2 = (unsigned char *)malloc(xsize);

  if (NULL == pRes->tmpbuf2)
    goto fail;

  fwrite(pRes, 512 - 404, 1, fp);
  fwrite(dummy, 404, 1, fp);

  pRes->file = fp;

  return pRes;
fail:
  if (NULL != fp)
    fclose(fp);
  if (NULL != pRes) {
    if (NULL != pRes->tmpbuf2)
      free(pRes->tmpbuf2);
    free(pRes);
  }
  return NULL;
}

int iclose(IMAGE *pImg) {
  if (NULL != pImg) {
    FILE *fp = (FILE *)(pImg->file);
    if (NULL != pImg->tmpbuf2)
      free(pImg->tmpbuf2);
    if (NULL != pImg->tmpbuf)
      free(pImg->tmpbuf);
    if (NULL != pImg->rowstart)
      free(pImg->rowstart);
    if (NULL != pImg->rowsize)
      free(pImg->rowsize);
    if (NULL != fp)
      fclose(fp);
    free(pImg);
  }
  return 0;
}

int getrow(IMAGE *pIMG, uint16_t *bf, unsigned int row, unsigned int z) {
  FILE *fp = (FILE *)(pIMG->file);
  int res;
  const uint16_t *pMax = bf + pIMG->xsize;
  if (0x0100 == (0xFF00 & pIMG->type)) {
    unsigned char *iPtr;
    uint16_t *oPtr;
    unsigned char pixel;
    int count;
    res = fseek(fp, pIMG->rowstart[row + z * pIMG->ysize], SEEK_SET);
    if (0 != res)
      return 1;
    res = fread(pIMG->tmpbuf2,
                (unsigned int)(pIMG->rowsize[row + z * pIMG->ysize]), 1, fp);
    if (1 != res)
      return 1;
    iPtr = (unsigned char *)(pIMG->tmpbuf2);
    oPtr = bf;
    for (;;) {
      pixel = *iPtr++;
      count = (int)(pixel & 0x7F);
      if (!count) {
        return 0;
      }
      if (pixel & 0x80) {
        while (count--) {
          if (oPtr == pMax)
            return 1;
          *oPtr++ = *iPtr++;
        }
      } else {
        pixel = *iPtr++;
        while (count--) {
          if (oPtr == pMax)
            return 1;
          *oPtr++ = pixel;
        }
      }
    }
  } else {
    int i;
    fseek(fp, 512 + (row * pIMG->xsize) + (z * pIMG->xsize * pIMG->ysize),
          SEEK_SET);
    fread(pIMG->tmpbuf2, pIMG->xsize, 1, fp);
    for (i = 0; i < pIMG->xsize; i++)
      bf[i] = pIMG->tmpbuf2[i];
  }
  return 0;
}

int putrow(IMAGE *pIMG, uint16_t *bf, unsigned int row, unsigned int z) {
  FILE *fp;
  int i;
  for (i = 0; i < pIMG->xsize; i++)
    pIMG->tmpbuf2[i] = bf[i];

  fp = (FILE *)(pIMG->file);
  i = fseek(fp, 512 + (row * pIMG->xsize) + (z * pIMG->xsize * pIMG->ysize),
            SEEK_SET);
  assert(i == 0);
  fwrite(pIMG->tmpbuf2, pIMG->xsize, 1, fp);
  return 0;
}

void isetname(IMAGE *pImg,char *name) {}
