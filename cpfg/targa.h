/* ******************************************************************** *
   Copyright (C) 1990-2022 University of Calgary
  
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
  
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * ******************************************************************** */



#ifndef __TARGA_H__
#define __TARGA_H__

#ifdef __cplusplus
extern "C" {
#endif

/* TYPE OF TARGA FILES */
#define TGA_NOIMAGE 0        /* no image data included */
#define TGA_MAPPED 1         /* uncompressed, color-mapped images */
#define TGA_TRUECOLOR 2      /* uncompressed, RGB images  */
#define TGA_GRAY 3           /* uncompressed, black and white images */
#define TGA_MAPPED_RLE 9     /* runlength encoded color-mapped images */
#define TGA_TRUECOLOR_RLE 10 /* runlength encoded RGB images  */

typedef unsigned char byte;

struct targa_params_type {
  FILE *fp;
  byte type;
  int Xorigin, Yorigin;
  int Xres, Yres;
  char *id_field;
  char *colormap; /* pointer to a field of byte triplets RGB */
  int first_cmap_entry, num_cmap_entries;

  /* internal */
  int current_row;
  signed char orientation; /* plus minus 1 */
  char packet_type;
  int items_left;
  byte packet_color[3];
};
typedef struct targa_params_type targa_params_type;

/*** prototypes ***/
int saveTGAhead(targa_params_type *spec);
/* row contains Xres triples of bytes of blue, green, and red */
int saveTGArow(targa_params_type *spec, int y, byte *row);
int saveTGAfinish(targa_params_type *spec);

int loadTGAhead(const char *filename, targa_params_type *spec);
/* row contains Xres triples of bytes of blue, green, and red */
int loadTGArow(targa_params_type *spec, int y, byte *row);
int loadTGAfinish(targa_params_type *spec);

#ifdef __cplusplus
}
#endif

#else
#error File already included
#endif
