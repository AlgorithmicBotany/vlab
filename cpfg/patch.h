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



#ifndef __PATCH_H__
#define __PATCH_H__

#include "opengl.h"
#include <string.h>

/* Maximum Number of patches + 1 allowed per surface */
#define PATCHES 65

/* Maximum Number of surfaces */
#define SURFACES 15

/* prototypes */
#ifdef INVENTOR
#include "inventor.h"

#ifdef __cplusplus
extern "C" {
#endif
SoSep *iv_patch(TURTLE *tu, int id);
SoSep *iv_object(TURTLE *tu, char desired_surface, double scale_factor);
int iv_objects(TURTLE *tu, DRAWPARAM *dr, VIEWPARAM *vw);
#ifdef __cplusplus
}
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif
void InitializeSurfaces(void);
int read_surface(const char *datafile, float surface_size, char surface_id,
                 int s_input_precision, int t_input_precision, int s_texture);
int FindSurfaceIndex(char surface_id);
void InitSurfacePatches(void);
void SurfacePatchInit(const StringModule *module);
void SurfacePatchControlPoint(const StringModule *module, const TURTLE *turtle);
void determine_end_point(TURTLE *tu, char desired_surface, double scale);
void draw_surface_patch(
    TURTLE *tu, DRAWPARAM *dr, VIEWPARAM *vw, char desired_surface,
    void (*StartPatches)(const TURTLE *, DRAWPARAM *dr, VIEWPARAM *vw, int, int,
                         int),
    void (*RenderPatch)(TURTLE *, DRAWPARAM *dr, VIEWPARAM *vw, double[4][4],
                        double[4][4], double[4][4]),
    void (*EndPatches)(TURTLE *, DRAWPARAM *dr, VIEWPARAM *vw));
void SurfacePatchDraw(
    StringModule *module, TURTLE *tu, DRAWPARAM *dr, VIEWPARAM *vw,
    void (*StartPatches)(TURTLE *, DRAWPARAM *dr, VIEWPARAM *vw, int, int, int),
    /* parameters: tu, dr, vw, s- and t-precision, basisID */
    void (*RenderPatch)(TURTLE *, DRAWPARAM *dr, VIEWPARAM *vw, double[4][4],
                        double[4][4], double[4][4]),
    /* parameters: tu, dr, vw, x-, y- and z-control points*/
    void (*EndPatches)(TURTLE *, DRAWPARAM *dr, VIEWPARAM *vw)
    /* parameters: tu, dr, vw */);
void SurfaceTmeshDraw(StringModule *module, TURTLE *tu, DRAWPARAM *dr,
                      VIEWPARAM *vw,
                      void (*StartTmesh)(void), /* no parameters */
                      void (*TmeshVertex)(const float *, const DRAWPARAM *),
                      /* parameters: point(pos, normal, and texels), color,
                         and renderind specification */
                      void (*EndTmesh)(void)); /* no parameters */
void draw_surface_tmesh(TURTLE *tu, char desired_surface, DRAWPARAM *dr,
                        VIEWPARAM *vw,
                        void (*StartTmesh)(void), /* no parameters */
                        void (*TmeshVertex)(const float *, const DRAWPARAM *dr),
                        void (*EndTmesh)(void)); /* no parameters */
int GetSurfaceTexture(char desired_surface, TURTLE *tu);
void r_objects(FILE *fp, TURTLE *tu, const DRAWPARAM *dr, VIEWPARAM *vw);
int r_use_second_object(char desired_surface);
void o_objects(FILE *fp, TURTLE *tu, DRAWPARAM *dr, VIEWPARAM *vw);

#ifdef __cplusplus
}
#endif

#endif
