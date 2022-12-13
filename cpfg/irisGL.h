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



#ifndef _CPFG_IRISGL_
#define _CPFG_IRISGL_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NOGRAPHICS

turtleDrawDispatcher *iGlSetDispatcher(DRAWPARAM *, VIEWPARAM *);
void iGlMakeUnitSegment(DRAWPARAM *);
void iGlMakeUnitSphere(DRAWPARAM *);

int iGlSetup(TURTLE *, DRAWPARAM *, VIEWPARAM *);
void iGlStartNode(TURTLE *, DRAWPARAM *, VIEWPARAM *, float /* length */,
                  char /* symbol */);
void iGlEndNode(TURTLE *, DRAWPARAM *, VIEWPARAM *, char /* symbol */);
void iGlEndFlatNode(TURTLE *, DRAWPARAM *, VIEWPARAM *, char /* symbol */);
void iGLStartCylOrFlatNode(TURTLE *, DRAWPARAM *, VIEWPARAM *,
                           float /* length */, char /* symbol */);
void iGLEndCylNode(TURTLE *, DRAWPARAM *, VIEWPARAM *, char /* symbol */);

void iGlStartBranch(TURTLE *, DRAWPARAM *, VIEWPARAM *);
void iGlEndBranch(TURTLE *, DRAWPARAM *, VIEWPARAM *);
void iGlStartPolygon(POLYGON *, TURTLE *, DRAWPARAM *, VIEWPARAM *);
void iGlEndPolygon(POLYGON *, TURTLE *, DRAWPARAM *, VIEWPARAM *);
void iGlSetColour(const TURTLE *, const DRAWPARAM *, const VIEWPARAM *);
void iGlSetLineWidth(const TURTLE *, const DRAWPARAM *, const VIEWPARAM *);
void iGlCircle2D(const TURTLE *, const DRAWPARAM *, VIEWPARAM *,
                 float /* radius */);
void iGlCircle3D(const TURTLE *, const DRAWPARAM *, VIEWPARAM *,
                 float /* radius */);
void iGlCircleB2D(const TURTLE *, const DRAWPARAM *, const VIEWPARAM *,
                  float /* radius */, float /* width */);
void iGlCircleB3D(const TURTLE *, const DRAWPARAM *, const VIEWPARAM *,
                  float /* radius */, float /* width */);
void iGlSphere(const TURTLE *, const DRAWPARAM *, VIEWPARAM *,
               float /* radius */);
void iGlLabel(const TURTLE *, DRAWPARAM *, const VIEWPARAM *,
              const char * /* label */, int /* parameters */,
              const float * /* values */); /* JH1 */
void iGlBlackBox(const TURTLE *, const DRAWPARAM *, const VIEWPARAM *,
                 const StringModule * /*module*/,
                 const StringModule * /*submodule*/);
void iGlPredefinedSurface(TURTLE *, DRAWPARAM *, VIEWPARAM *, char /* ID */,
                          double, double, double);
void iGlLdefinedSurface(StringModule *, TURTLE *, DRAWPARAM *, VIEWPARAM *);
void iGlFinishUp(TURTLE *, DRAWPARAM *, VIEWPARAM *);

/* polygon rendering routines */
void iGlPolyFill(POLYGON *, TURTLE *, DRAWPARAM *, VIEWPARAM *);
void iGlPolyInterpolate(POLYGON *, TURTLE *, DRAWPARAM *, VIEWPARAM *);
void iGlPolyGouraud(POLYGON *, TURTLE *, DRAWPARAM *, VIEWPARAM *);
void iGlPolyBspline(POLYGON *, TURTLE *, DRAWPARAM *, VIEWPARAM *);
void NewGeom(Coord[4][3], double[3]);
void iGlPolyTwosided(POLYGON *, TURTLE *, DRAWPARAM *, VIEWPARAM *);
void iGlPolyWireframe(POLYGON *, TURTLE *, DRAWPARAM *, VIEWPARAM *);
void iGlTriangle(const float *p1, const float *p2, const float *p3,
                 const DRAWPARAM *);

/* surface rendering routines */
void iGlStartPatches(TURTLE *, DRAWPARAM *, VIEWPARAM *, int /* sPrecision */,
                     int /* tPrecision */, int /* basisId */);
void iGlRenderPatch(TURTLE *, DRAWPARAM *, VIEWPARAM *,
                    double[4][4] /* x coords */, double[4][4] /* y coords */,
                    double[4][4] /* z coords */);
void iGlEndPatches(TURTLE *, DRAWPARAM *, VIEWPARAM *);
void iGlStartTmesh(void);
void iGlEndTmesh(void);
void PolygonToMesh(POLYGON *polygon, DRAWPARAM *dr);

#endif /* NOGRAPHICS */

typedef float vertex[PITEM];
typedef float triangle[3][3];

/* unit cylinder for drawing */
struct cylinder_type {
  int sides;
  vertex *top;
  vertex *base;

  triangle *sphere; /* unit sphere for drawing */
  int noSphereTris; /* number of triangles that make up sphere */

  struct cylinder_type *next;
};

typedef struct cylinder_type CYLINDER;

/* unit sphere for drawing */
struct sphere_type {
  int sides;
  triangle *sphere; /* unit sphere for drawing */
  int noSphereTris; /* number of triangles that make up sphere */

  struct sphere_type *next;
};

typedef struct sphere_type SPHERE;

void FreeGL(void);

void my_getmcolor(Colorindex colindex, short *red, short *green, short *blue);
void my_getmaterial(Colorindex colindex, material_type **mat);
int is_material(Colorindex colindex);
void my_getcolormapitem(Colorindex colindex, colormap_item_type **item);
int load_in_colormaps(void);
int load_in_materials(void);
void FreeMaterialSpace(void);
void FreeColormapSpace(void);
CYLINDER *FindCylinder(int sides);
SPHERE *FindSphere(int sides);
SPHERE *MakeSphere(int sides);

#ifdef __cplusplus
}
#endif

#endif /* _CPFG_IRISGL_ */
