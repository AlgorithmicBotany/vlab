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



#ifndef __TSURFACES_H__
#define __TSURFACES_H__

#ifdef __cplusplus
extern "C" {
#endif

void InitializeTsurfaces(void);
void FreeTsurfaces(void);

int FindTSurfaceIndex(char surface_id);
int read_tsurface(char *name, float surface_size, char surface_id);
void draw_tsurface(const TURTLE *tu, char desired_surface, DRAWPARAM *dr,
                   VIEWPARAM *vw);
void r_tsurfaces(FILE *fp, TURTLE *tu, DRAWPARAM *dr, VIEWPARAM *vw);

#ifdef __cplusplus
}
#endif

#else
#error File already included
#endif
