/*
   Provides functions for saving truecolor rle images.

   Author: Radomir Mech

   */

#ifdef WIN32
#include "warningset.h"
#endif

#include <stdlib.h>
#include <stdio.h>

#include "rle.h"
#include "log.h"

/* packet type */
#define PACKET_RAW 0
#define PACKET_RLE 1

#define MAXINPACKET 256

/* Opcodes */
#define SkipLinesOp 1
#define SetChannelOp 2
#define RawDataOp 5
#define RleDataOp 6
#define EofOp 7

/************************************************************/
static int savebyte(unsigned char c, FILE *fp) { return fwrite(&c, 1, 1, fp); }

/***************************************************************************/
int saveRLEhead(RLE_params_type *spec) {
  if (spec->fp == NULL) {
    Message("RLE error - file not opened!\n");
    return 0;
  }

  /* magic number */
  savebyte(0x52, spec->fp);
  savebyte(0xcc, spec->fp);

  /* 2 bytes (lo-hi) X origin */
  savebyte((unsigned char)(spec->Xorigin & 0xff), spec->fp);
  savebyte((unsigned char)(spec->Xorigin >> 8), spec->fp);

  /* 2 bytes (lo-hi) Y origin */
  savebyte((unsigned char)(spec->Yorigin & 0xff), spec->fp);
  savebyte((unsigned char)(spec->Yorigin >> 8), spec->fp);

  /* 2 bytes (lo-hi) X resolution */
  savebyte((unsigned char)(spec->Xres & 0xff), spec->fp);
  savebyte((unsigned char)(spec->Xres >> 8), spec->fp);

  /* 2 bytes (lo-hi) Y resolution */
  savebyte((unsigned char)(spec->Yres & 0xff), spec->fp);
  savebyte((unsigned char)(spec->Yres >> 8), spec->fp);

  /* flag byte
  bit 0 (1)- clearfirst - image is initialized by the background color
  bit 1 (2)- no background color supplied
  bit 2 (4)- alpha channel (-1) present
  bit 3 (8)- comments are present */
  savebyte(0x02, spec->fp);

  /* number of color channels */
  savebyte(3, spec->fp);

  /* number of bits per pixel - always 8 */
  savebyte(8, spec->fp);

  /* number of channels in colormap */
  savebyte(0, spec->fp); /* no colormap */

  /* colormap length (2^x) */
  savebyte(0, spec->fp); /* no colormap */

  /* no background color - would be red, green, and blue */

  /* filler byte to allign the header to odd number of bytes */
  savebyte(0, spec->fp);

  /* header complete */
  spec->current_row = 0;

  return 1;
}

/***************************************************************************/
/* recognizes only 3 channel pictures */
int loadRLEhead(const char *filename, RLE_params_type *spec) {
  unsigned char pom, pom2;

  if ((spec->fp = fopen(filename, "rb")) == NULL) {
    Message("RLE error - cannot open file %s!\n", filename);
    return 0;
  }

  /* magic number */
  fread(&pom, 1, 1, spec->fp);
  fread(&pom2, 1, 1, spec->fp);

  if ((pom != 0x52) || (pom2 != 0xcc)) {
    Message("File %s is not an rle image (magic number missing).\n", filename);
    return 0;
  }

  /* 2 bytes (lo-hi) X origin */
  fread(&pom, 1, 1, spec->fp);
  fread(&pom2, 1, 1, spec->fp);
  spec->Xorigin = (((unsigned int)pom2) << 8) + (unsigned int)pom;

  /* 2 bytes (lo-hi) Y origin */
  fread(&pom, 1, 1, spec->fp);
  fread(&pom2, 1, 1, spec->fp);
  spec->Yorigin = (((unsigned int)pom2) << 8) + (unsigned int)pom;

  /* 2 bytes (lo-hi) X resolution */
  fread(&pom, 1, 1, spec->fp);
  fread(&pom2, 1, 1, spec->fp);
  spec->Xres = (((unsigned int)pom2) << 8) + (unsigned int)pom;

  /* 2 bytes (lo-hi) Y resolution */
  fread(&pom, 1, 1, spec->fp);
  fread(&pom2, 1, 1, spec->fp);
  spec->Yres = (((unsigned int)pom2) << 8) + (unsigned int)pom;

  /* flag byte
  bit 0 (1)- clearfirst - image is initialized by the background color
  bit 1 (2)- no background color supplied
  bit 2 (4)- alpha channel (-1) present
  bit 3 (8)- comments are present */
  fread(&pom, 1, 1, spec->fp);
  if (pom != 0x02) {
    Message("RLE error - only images with flag 2 supported (is %x)\n",
            (int)pom);
    return 0;
  }

  /* number of color channels */
  fread(&pom, 1, 1, spec->fp);
  if (pom != 3) {
    Message("RLE error - only images with three channels supported (is %d)\n",
            (int)pom);
    return 0;
  }

  /* number of bits per pixel. Ignored, assumed 8 */
  fread(&pom, 1, 1, spec->fp);

  /* number of channels in colormap */
  fread(&pom, 1, 1, spec->fp);
  if (pom != 0) {
    Message("RLE error - only images without colormap supported\n");
    return 0;
  }

  /* colormap length (2^x) - ignored */
  fread(&pom, 1, 1, spec->fp);

  /* no background color - would be red, green, and blue */

  /* filler byte to allign the header to odd number of bytes */
  fread(&pom, 1, 1, spec->fp);

  /* header complete */
  spec->current_row = 0;

  return 1;
}

/************************************************************/
static int SavePacket(unsigned char type, unsigned char *row,
                      unsigned short count, FILE *fp) {
  if (count == 0)
    return -1;

  if (type == PACKET_RLE) {
    savebyte(RleDataOp, fp);
    savebyte((unsigned char)(count - 1), fp);
    savebyte((unsigned char)(row[0]), fp);
    savebyte(0, fp);
    return 4;
  }

  savebyte(RawDataOp, fp);
  savebyte((unsigned char)(count - 1), fp);

  fwrite(row, count, 1, fp);

  if ((count % 2) == 1) {
    /* filler byte to allign to 16 bits */
    savebyte(0, fp);
    count++;
  }

  return 2 + (int)count;
}

/************************************************************/
static int RleCodeRGBRow(unsigned char *row, int Xres, FILE *fp) {
  int x;
  unsigned char *pb_ptr;
  int pb_x;
  char the_same;
  unsigned short col1, col2;
  unsigned char packet_type;

  x = 0;

  /* store the beginning of a packet */
  pb_ptr = row;
  pb_x = 0;

  col1 = *(row++);
  x++;
  the_same = 0;
  packet_type = PACKET_RAW;

  while (x < Xres) {
    /* is the end of packet reached? */
    if (x - pb_x == MAXINPACKET) {
      SavePacket((unsigned char)packet_type, pb_ptr, (short)(x - pb_x), fp);
      pb_ptr = row;
      pb_x = x;
      the_same = 0;
      packet_type = PACKET_RAW;
      col1 = 256;
    }

    col2 = *(row++);
    x++;

    if (col2 == col1) {
      if (packet_type == PACKET_RAW) {
        if (the_same) {
          /* three in a row */
          SavePacket((unsigned char)PACKET_RAW, pb_ptr, (short)(x - 3 - pb_x),
                     fp);
          pb_ptr = row - 3;
          pb_x = x - 3;
          packet_type = PACKET_RLE;
        } else
          the_same = 1;
      }

      /* nothing for RLE packet */
    } else if (packet_type == PACKET_RAW)
      the_same = 0;
    else {
      /* RLE packet */
      SavePacket((unsigned char)PACKET_RLE, pb_ptr, (short)(x - 1 - pb_x), fp);
      pb_ptr = row - 1;
      pb_x = x - 1;
      packet_type = PACKET_RAW;
      the_same = 0;
    }

    col1 = col2;
  }

  /* the end of line reached */
  SavePacket((unsigned char)packet_type, pb_ptr, (short)(x - pb_x), fp);

  return Xres;
}

/************************************************************/
#ifndef min
#define min(x, y) ((x) < (y) ? (x) : (y))
#endif

static int RleDecodeRGBRow(unsigned char *row, RLE_params_type *spec) {
  int num, count, i;
  unsigned char opt_code, op, val;
  unsigned char *ptr = row;

  count = 0;

  for (;;) {
    fread(&opt_code, 1, 1, spec->fp);
    fread(&op, 1, 1, spec->fp);

    switch (opt_code) {
    case SkipLinesOp:
      /* end of line */
      return count;

    case SetChannelOp:
      /* select channel */
      ptr = row + spec->Xres * (int)op;
      count = 0;
      break;
    case RawDataOp:
      num = 1 + (int)op;

      fread(ptr, num, 1, spec->fp);
      if ((num % 2) == 1)
        fread(&val, 1, 1, spec->fp);

      ptr += num;
      count += num;
      break;

    case RleDataOp:
      num = 1 + (int)op;

      fread(&val, 1, 1, spec->fp);
      for (i = 0; i < num; i++)
        *(ptr++) = val;

      fread(&val, 1, 1, spec->fp);

      count += num;
      break;

    case EofOp:
      /* end of file */
      return count;

    default:
      Message("RLE error - unknown opt code %d.\n", (int)opt_code);
      return -1;
    }
  }
}

/************************************************************/
int saveRLErow(RLE_params_type *spec, int y, unsigned char *row) {
  unsigned char channel;

  if (spec->current_row != y) {
    Message("RLE error - row number %d doesn't correspond to the "
            "current row %d!\n",
            y, spec->current_row);
    return -1;
  }

  if ((spec->current_row < 0) || (spec->current_row >= spec->Yres)) {
    Message("RLE error - row is out of the given Yres!\n");
    return -1;
  }

  spec->current_row++;

  for (channel = 0; channel < 3; channel++) {
    /* set the current channel */
    savebyte(SetChannelOp, spec->fp);
    savebyte(channel, spec->fp);

    RleCodeRGBRow(row, spec->Xres, spec->fp);
    row += spec->Xres;
  }

  /* increment the line number */
  savebyte(SkipLinesOp, spec->fp);
  savebyte(1, spec->fp);

  return -1;
}

/************************************************************/
int loadRLErow(RLE_params_type *spec, int y, unsigned char *row) {
  if (spec->current_row != y) {
    Message("RLE error - row number %d doesn't correspond to the "
            "current row %d!\n",
            y, spec->current_row);
    return -1;
  }

  if ((spec->current_row < 0) || (spec->current_row >= spec->Yres)) {
    Message("RLE error - row is out of the given Yres!\n");
    return -1;
  }

  spec->current_row++;

  return RleDecodeRGBRow(row, spec);
}

/************************************************************/
int saveRLEfinish(RLE_params_type *spec) {
  savebyte(EofOp, spec->fp);
  savebyte(0, spec->fp);
  fclose(spec->fp);
  return 0;
}

/************************************************************/
int loadRLEfinish(RLE_params_type *spec) {
  fclose(spec->fp);
  return 0;
}
