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



#ifndef _CPFG_VIEWVOL_
#define _CPFG_VIEWVOL_

#ifdef __cplusplus
extern "C" {
#endif

turtleDrawDispatcher *vvSetDispatcher(const DRAWPARAM *, const VIEWPARAM *);

int vvSetup(TURTLE *, DRAWPARAM *, VIEWPARAM *);
void vvStartNode(TURTLE *, DRAWPARAM *, VIEWPARAM *, float /* length */,
                 char /* symbol */);

void vvEndNode(TURTLE *, DRAWPARAM *, VIEWPARAM *, char /* symbol */);
void vvStartBranch(TURTLE *, DRAWPARAM *, VIEWPARAM *);
void vvEndBranch(TURTLE *, DRAWPARAM *, VIEWPARAM *);
void vvStartPolygon(POLYGON *, TURTLE *, DRAWPARAM *, VIEWPARAM *);
void vvEndPolygon(POLYGON *, TURTLE *, DRAWPARAM *, VIEWPARAM *);
void vvSetColour(const TURTLE *, const DRAWPARAM *, const VIEWPARAM *);
void vvSetLineWidth(const TURTLE *, const DRAWPARAM *, const VIEWPARAM *);
void vvCircle2D(const TURTLE *, const DRAWPARAM *, VIEWPARAM *,
                float /* radius */);
void vvCircle3D(const TURTLE *, const DRAWPARAM *, VIEWPARAM *,
                float /* radius */);
void vvSphere(const TURTLE *, const DRAWPARAM *, VIEWPARAM *,
              float /* radius */);
void vvLabel(const TURTLE *, DRAWPARAM *, const VIEWPARAM *,
             const char * /* label */, int /* parameters */,
             const float * /* values */); /* JH1 */
void vvBlackBox(const TURTLE *, const DRAWPARAM *, const VIEWPARAM *,
                const StringModule * /*module*/,
                const StringModule * /*submodule*/);
void vvPredefinedSurface(TURTLE *, DRAWPARAM *, VIEWPARAM *, char /* ID */,
                         double, double, double);
void vvStartPatches(const TURTLE *, DRAWPARAM *, VIEWPARAM *,
                    int /* sPrecision */, int /* tPrecision */,
                    int /* basis ID */);
void vvRenderPatch(TURTLE *, DRAWPARAM *, VIEWPARAM *, double[4][4],
                   double[4][4], double[4][4]);
void vvEndPatches(TURTLE *, DRAWPARAM *, VIEWPARAM *);
void vvLdefinedSurface(StringModule *, TURTLE *, DRAWPARAM *, VIEWPARAM *);
void vvFinishUp(TURTLE *, DRAWPARAM *, VIEWPARAM *);

void vvUpdate(const double * /* position */, VIEWPARAM *);

void vvRenderTriangle(const float *, const float *, const float *,
                      const DRAWPARAM *);
void vvTmeshVertex(const float *point, const DRAWPARAM *dr);
void vvCircleB2D(const TURTLE *, const DRAWPARAM *, const VIEWPARAM *,
                 float radius, float width);
void vvCircleB3D(const TURTLE *, const DRAWPARAM *, const VIEWPARAM *,
                 float radius, float width);

#ifdef __cplusplus
}
#endif

#endif /*_CPFG_VIEWVOL_*/
