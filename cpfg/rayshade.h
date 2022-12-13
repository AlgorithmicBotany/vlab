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



#ifndef _CPFG_RAYSHADE_
#define _CPFG_RAYSHADE_

#ifdef __cplusplus
extern "C" {
#endif

turtleDrawDispatcher *rsSetDispatcher(DRAWPARAM *, VIEWPARAM *);

int rsSetup(TURTLE *, DRAWPARAM *, VIEWPARAM *);
int rsSetPass(int pass_no);
void rsStartNode(TURTLE *, DRAWPARAM *, VIEWPARAM *, float /* length */,
                 char /* symbol */);
void rsEndNode(TURTLE *, DRAWPARAM *, VIEWPARAM *, char /* symbol */);
void rsStartBranch(TURTLE *, DRAWPARAM *, VIEWPARAM *);
void rsEndBranch(TURTLE *, DRAWPARAM *, VIEWPARAM *);
void rsStartPolygon(POLYGON *, TURTLE *, DRAWPARAM *, VIEWPARAM *);
void rsEndPolygon(POLYGON *, TURTLE *, DRAWPARAM *, VIEWPARAM *);
void rsSetColour(const TURTLE *, const DRAWPARAM *, const VIEWPARAM *);
void rsSetLineWidth(const TURTLE *, const DRAWPARAM *, const VIEWPARAM *);
void rsCircle2D(const TURTLE *, const DRAWPARAM *, VIEWPARAM *,
                float /* radius */);
void rsCircle3D(const TURTLE *, const DRAWPARAM *, VIEWPARAM *,
                float /* radius */);
void rsCircleB2D(const TURTLE *tu, const DRAWPARAM *dr, const VIEWPARAM *vw,
                 float diameter, float width);
void rsCircleB3D(const TURTLE *tu, const DRAWPARAM *dr, const VIEWPARAM *vw,
                 float diameter, float width);
void rsSphere(const TURTLE *, const DRAWPARAM *, VIEWPARAM *,
              float /* radius */);
void rsLabel(const TURTLE *, DRAWPARAM *, const VIEWPARAM *,
             const char * /* label */, int /* parameters */,
             const float * /* values */); /* JH1 */
void rsBlackBox(const TURTLE *, const DRAWPARAM *, const VIEWPARAM *,
                const StringModule * /*module*/,
                const StringModule * /*submodule*/);
void rsPredefinedSurface(TURTLE *, DRAWPARAM *, VIEWPARAM *, char /* ID */,
                         double, double, double);
void rsLdefinedSurface(StringModule *, TURTLE *, DRAWPARAM *, VIEWPARAM *);
void rsFinishUp(TURTLE *, DRAWPARAM *, VIEWPARAM *);
void rsRenderTriangle(const float *p1, const float *p2, const float *p3,
                      const DRAWPARAM *);

void rsColor(int color);
void rsStartTmesh(void);
void rsNormal(const float normal[3]);
void rsTmeshVertex(const float position[3]);
void rsEndTmesh(void);
int rsStartTexture(int index);
void rsSetTexCoord(const float coords[2]);
void rsEndTexture(int index);
void rsStartNewGrid(TURTLE *tu, DRAWPARAM *dr, VIEWPARAM *vw, int *size);

int rsStartObject(TURTLE *tu, int depth, char *curPtr, Production *prodPtr);
int rsEndObject(TURTLE *tu, int depth, char *curPtr, Production *prodPtr);
void rsReadViewCommand(char *input_line, VIEWPARAM *vw);
int rsInsertObject(char *name, float scale, TURTLE *tu, VIEWPARAM *w,
                   DRAWPARAM *dr);

int r_patch_color(int n);
void r_use_surface(int n);

#ifdef __cplusplus
}
#endif

#endif /* _CPFG_RAYSHADE_ */
