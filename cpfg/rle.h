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



#ifndef __RLE_H__
#define __RLE_H__

#ifdef __cplusplus
extern "C" {
#endif

struct RLE_params_type {
  FILE *fp;
  int Xorigin, Yorigin;
  int Xres, Yres;

  /* internal */
  int current_row;
};
typedef struct RLE_params_type RLE_params_type;

/*** prototypes ***/
int saveRLEhead(RLE_params_type *spec);
/* row contains Xres bytes of red, Xres bytes of green and Xres bytes of blue
   channel */
int saveRLErow(RLE_params_type *spec, int y, unsigned char *row);
int saveRLEfinish(RLE_params_type *spec);

int loadRLEhead(const char *filename, RLE_params_type *spec);
/* row contains Xres bytes of red, Xres bytes of green and Xres bytes of blue
   channel */
int loadRLErow(RLE_params_type *spec, int y, unsigned char *row);
int loadRLEfinish(RLE_params_type *spec);

#ifdef __cplusplus
}
#endif

#endif
