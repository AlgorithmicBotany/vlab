

/********************************************************************/
/*                           viewVol.c              		        */
/*                                                                  */
/* viewVol provides interpret routines for calculating the          */
/* view volume. Routines are dispatched via the turtleDrawDispatcher*/
/*                                                                  */
/********************************************************************/
/*
        CREATED: August 6, 1991 BY: Jim at Apple
        MODIFIED: March-June 1994 BY: Radek
                ansi standard + prepared for C++
                vvUpdate faster
*/

#ifdef WIN32
#include "warningset.h"
#endif

#include <stdio.h>

#include "control.h"
#include "interpret.h"
#include "viewVol.h"
#include "patch.h"
#include "textures.h"
#include "mesh.h"
#include "indices.h"
#include "utility.h"

/* prototypes */
void vvUpdatef(const float *position, VIEWPARAM *vp);
void GetTextExtent(char *str, int *width, int *ascent, int *descent);

/* The structure for dispatching rendering routines */
turtleDrawDispatcher viewVolumeRoutines = {
    vvSetup,        vvStartNode,         vvEndNode,         vvStartBranch,
    vvEndBranch,    vvStartPolygon,      vvEndPolygon,      vvSetColour,
    vvSetLineWidth, vvCircle2D,          vvCircle3D,        vvSphere,
    vvBlackBox,     vvPredefinedSurface, vvLdefinedSurface, vvLabel,
    vvFinishUp,     vvRenderTriangle,    voidStartTexture,  voidEndTexture,
    voidStartTmesh, vvTmeshVertex,       voidEndTmesh,      vvCircleB2D,
    vvCircleB3D};

double surfaceXYZScaleFactor[3] = {0, 0, 0};

TURTLE surfaceTurtle;

/********************************************************************/
/* Function: vvSetup                                                */
/* Initialize volume mins and maxs                                  */
/* return non-zero if there are problems                            */
/********************************************************************/

int vvSetup(__attribute__((unused)) TURTLE *tu,
            __attribute__((unused)) DRAWPARAM *dr, VIEWPARAM *vw) {
  vw->min[0] = vw->min[1] = vw->min[2] = 1.0;
  vw->max[0] = vw->max[1] = vw->max[2] = 0.0;

  return 0;
}

/********************************************************************/
/* Function: vvStartNode                                            */
/* No action                                                        */
/********************************************************************/

void vvStartNode(__attribute__((unused)) TURTLE *tu,
                 __attribute__((unused)) DRAWPARAM *dr,
                 __attribute__((unused)) VIEWPARAM *vw,
                 __attribute__((unused)) float length,
                 __attribute__((unused)) char symbol) {}

/********************************************************************/
/* Function: vvEndNode                                              */
/* Passes the current turtle position in world coordinates          */
/* to the view volume update routines.                              */
/* Should it handle linewidth?? Should it only register for F or G? */
/********************************************************************/

void vvEndNode(TURTLE *tu, __attribute__((unused)) DRAWPARAM *dr, VIEWPARAM *vw,
               __attribute__((unused)) char symbol) {
  vvUpdate(tu->position, vw);
}

/********************************************************************/
/* Function: vvStartBranch                                          */
/* No action                                                        */
/********************************************************************/

void vvStartBranch(__attribute__((unused)) TURTLE *tu,
                   __attribute__((unused)) DRAWPARAM *dr,
                   __attribute__((unused)) VIEWPARAM *vw) {}

/********************************************************************/
/* Function: vvEndBranch                                            */
/* No action                                                        */
/********************************************************************/

void vvEndBranch(__attribute__((unused)) TURTLE *tu,
                 __attribute__((unused)) DRAWPARAM *dr,
                 __attribute__((unused)) VIEWPARAM *vw) {}

/********************************************************************/
/* Function: vvStartPolygon                                         */
/* No action                                                        */
/********************************************************************/

void vvStartPolygon(__attribute__((unused)) POLYGON *polygon,
                    __attribute__((unused)) TURTLE *tu,
                    __attribute__((unused)) DRAWPARAM *dr,
                    __attribute__((unused)) VIEWPARAM *vw) {}

/********************************************************************/
/* Function: vvEndPolygon                                           */
/* No action                                                        */
/********************************************************************/

void vvEndPolygon(__attribute__((unused)) POLYGON *polygon,
                  __attribute__((unused)) TURTLE *tu,
                  __attribute__((unused)) DRAWPARAM *dr,
                  __attribute__((unused)) VIEWPARAM *vw) {}

/********************************************************************/
/* Function: vvSetColour                                            */
/* No action                                                        */
/********************************************************************/

void vvSetColour(__attribute__((unused)) const TURTLE *tu,
                 __attribute__((unused)) const DRAWPARAM *dr,
                 __attribute__((unused)) const VIEWPARAM *vw) {}

/********************************************************************/
/* Function: vvSetLineWidth                                         */
/* No action                                                        */
/********************************************************************/

void vvSetLineWidth(__attribute__((unused)) const TURTLE *tu,
                    __attribute__((unused)) const DRAWPARAM *dr,
                    __attribute__((unused)) const VIEWPARAM *vw) {}

/********************************************************************/
/* Function: vvCircle2D                                             */
/********************************************************************/

void vvCircle2D(const TURTLE *tu, __attribute__((unused)) const DRAWPARAM *dr,
                VIEWPARAM *vw, float diameter) {
  int x, y;
  double vec[3];
  float radius = diameter * 0.5;

  vec[2] = 0.0;

  for (x = -1; x <= 1; x += 2) {
    for (y = -1; y <= 1; y += 2) {
      vec[0] = tu->position[0] + radius * x;
      vec[1] = tu->position[1] + radius * y;

      vvUpdate(vec, vw);
    }
  }
}

/********************************************************************/
/* Function: vvCircle3D                                             */
/* It is not very correct, since the disk can be oriented accoring to
   turtle. But this is good enough
 */
/********************************************************************/

void vvCircle3D(const TURTLE *tu, __attribute__((unused)) const DRAWPARAM *dr,
                VIEWPARAM *vw, float diameter) {
  int x, y;
  double vec[3];
  float radius = diameter * 0.5;

  vec[2] = tu->position[2];

  for (x = -1; x <= 1; x += 2) {
    for (y = -1; y <= 1; y += 2) {
      vec[0] = tu->position[0] + radius * x;
      vec[1] = tu->position[1] + radius * y;

      vvUpdate(vec, vw);
    }
  }
}

/********************************************************************/
/* Function: vvSphere                                               */
/********************************************************************/

void vvSphere(const TURTLE *tu, __attribute__((unused)) const DRAWPARAM *dr,
              VIEWPARAM *vw, float diameter) {
  int x, y, z;
  double vec[3];
  float radius = diameter * 0.5;

  for (x = -1; x <= 1; x += 2) {
    for (y = -1; y <= 1; y += 2) {
      for (z = -1; z <= 1; z += 2) {
        vec[0] = tu->position[0] + radius * x;
        vec[1] = tu->position[1] + radius * y;
        vec[2] = tu->position[2] + radius * z;

        vvUpdate(vec, vw);
      }
    }
  }
}

/********************************************************************/
/* Function: vvLabel          JH1                                   */
/* No action                                                        */
/********************************************************************/

void vvLabel(__attribute__((unused)) const TURTLE *tu,
             __attribute__((unused)) DRAWPARAM *dr,
             __attribute__((unused)) const VIEWPARAM *vw, const char *label,
             int parameters, const float values[]) {
  char *str;
  int width, ascent, descent;

  str = MakeLabel(label, parameters, values);

  GetTextExtent(str, &width, &ascent, &descent);
  /* so now we have the size in pixels */

  /* not finished! */
}

/********************************************************************/
/* Function: vvBLackBox                                             */
/* No action                                                        */
/********************************************************************/

void vvBlackBox(__attribute__((unused)) const TURTLE *tu,
                __attribute__((unused)) const DRAWPARAM *dr,
                __attribute__((unused)) const VIEWPARAM *vw,
                __attribute__((unused)) const StringModule *module,
                __attribute__((unused)) const StringModule *submodule) {}

/********************************************************************/
void vvTmeshVertex(const float *point,
                   __attribute__((unused)) const DRAWPARAM *dr) {
  extern VIEWPARAM viewparam;

  vvUpdatef(point, &viewparam);
}

/********************************************************************/
void vvRenderTriangle(const float *p1, const float *p2, const float *p3,
                      __attribute__((unused)) const DRAWPARAM *dr) {
  extern VIEWPARAM viewparam;
  double thisPoint[3], x, y, z;
  int i;

  if (surfaceXYZScaleFactor[0] != 0 && surfaceXYZScaleFactor[1] != 0 &&
      surfaceXYZScaleFactor[2] != 0) {
    /* used for tsurfaces */
    x = p1[eX] * surfaceXYZScaleFactor[0];
    y = p1[eY] * surfaceXYZScaleFactor[1];
    z = p1[eZ] * surfaceXYZScaleFactor[2];

    for (i = eX; i <= eZ; i++)
      thisPoint[i] = x * surfaceTurtle.left[i] + y * surfaceTurtle.heading[i] +
                     z * surfaceTurtle.up[i] + surfaceTurtle.position[i];

    vvUpdate(thisPoint, &viewparam);

    x = p2[eX] * surfaceXYZScaleFactor[0];
    y = p2[eY] * surfaceXYZScaleFactor[1];
    z = p2[eZ] * surfaceXYZScaleFactor[2];

    for (i = eX; i <= eZ; i++)
      thisPoint[i] = x * surfaceTurtle.left[i] + y * surfaceTurtle.heading[i] +
                     z * surfaceTurtle.up[i] + surfaceTurtle.position[i];

    vvUpdate(thisPoint, &viewparam);

    x = p3[eX] * surfaceXYZScaleFactor[0];
    y = p3[eY] * surfaceXYZScaleFactor[1];
    z = p3[eZ] * surfaceXYZScaleFactor[2];

    for (i = eX; i <= eZ; i++)
      thisPoint[i] = x * surfaceTurtle.left[i] + y * surfaceTurtle.heading[i] +
                     z * surfaceTurtle.up[i] + surfaceTurtle.position[i];

    vvUpdate(thisPoint, &viewparam);
  } else {
    /* used from generalized cylinders, e.g. */
    vvUpdatef(p1, &viewparam);
    vvUpdatef(p2, &viewparam);
    vvUpdatef(p3, &viewparam);
  }
}

/********************************************************************/
/* Function: vvPredefinedSurface                                    */
/* Passes surface patch evaluation routine name to surface routines */
/********************************************************************/

void vvPredefinedSurface(TURTLE *tu, DRAWPARAM *dr, VIEWPARAM *vw, char id,
                         double sX, double sY, double sZ) {
  if (sX != 0.0 && sY != 0.0 && sZ != 0.0) {
    /* save scale for vvRenderPatch or vvRenderTriangle */
    surfaceXYZScaleFactor[0] = sX;
    surfaceXYZScaleFactor[1] = sY;
    surfaceXYZScaleFactor[2] = sZ;
    surfaceTurtle = *tu;

    draw_surface_patch(tu, dr, vw, id, vvStartPatches, vvRenderPatch,
                       vvEndPatches);
  }

  surfaceXYZScaleFactor[0] = 0;
  surfaceXYZScaleFactor[1] = 0;
  surfaceXYZScaleFactor[2] = 0;
  /* so vvRenderTriangle ignores the surfacTurtle */
}

/********************************************************************/
/* Function: vvStartPatches                                        */
/* Nothing                                                          */
/********************************************************************/

void vvStartPatches(__attribute__((unused)) const TURTLE *tu,
                    __attribute__((unused)) DRAWPARAM *dr,
                    __attribute__((unused)) VIEWPARAM *vw,
                    __attribute__((unused)) int sPrecision,
                    __attribute__((unused)) int tPrecision,
                    __attribute__((unused)) int basisID) {}

/********************************************************************/
/* Function: vvRenderPatch                                          */
/* Renders a wireframe patch                                        */
/********************************************************************/

void vvRenderPatch(TURTLE *tu, __attribute__((unused)) DRAWPARAM *dr,
                   VIEWPARAM *vw, double xPoints[4][4], double yPoints[4][4],
                   double zPoints[4][4]) {
  int i, j;
  double thisPoint[3], x, y, z;

  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      x = xPoints[i][j] * surfaceXYZScaleFactor[0];
      y = yPoints[i][j] * surfaceXYZScaleFactor[1];
      z = zPoints[i][j] * surfaceXYZScaleFactor[2];

      thisPoint[eX] = x * tu->left[eX] + y * tu->heading[eX] + z * tu->up[eX] +
                      tu->position[eX];

      thisPoint[eY] = x * tu->left[eY] + y * tu->heading[eY] + z * tu->up[eY] +
                      tu->position[eY];

      thisPoint[eZ] = x * tu->left[eZ] + y * tu->heading[eZ] + z * tu->up[eZ] +
                      tu->position[eZ];

      vvUpdate(thisPoint, vw);
    }
  }
}

/********************************************************************/
/* Function: vvEndPatches                                           */
/* Nothing                                                          */
/********************************************************************/

void vvEndPatches(__attribute__((unused)) TURTLE *tu,
                  __attribute__((unused)) DRAWPARAM *dr,
                  __attribute__((unused)) VIEWPARAM *vw) {}

/********************************************************************/
/* Function: vvLdefinedSurface                                      */
/* No action                                                        */
/* since the turtle must have traced the control points             */
/* ie convex hull for the Bezier case                               */
/********************************************************************/

void vvLdefinedSurface(__attribute__((unused)) StringModule *module,
                       __attribute__((unused)) TURTLE *tu,
                       __attribute__((unused)) DRAWPARAM *dr,
                       __attribute__((unused)) VIEWPARAM *vw) {}

/********************************************************************/
/* Function: vvFinishUp                                             */
/* No action                                                        */
/********************************************************************/

void vvFinishUp(__attribute__((unused)) TURTLE *tu,
                __attribute__((unused)) DRAWPARAM *dr,
                __attribute__((unused)) VIEWPARAM *vw) {}

/********************************************************************/
/* Function: vvUpdate                                               */
/* Modifies the current view volume as necessary.                   */
/********************************************************************/

void vvUpdate(const double *position, VIEWPARAM *vp) {
  if (position[0] < vp->min[0])
    vp->min[0] = position[0];
  else if (position[0] > vp->max[0])
    vp->max[0] = position[0];

  if (position[1] < vp->min[1])
    vp->min[1] = position[1];
  else if (position[1] > vp->max[1])
    vp->max[1] = position[1];

  if (position[2] < vp->min[2])
    vp->min[2] = position[2];
  else if (position[2] > vp->max[2])
    vp->max[2] = position[2];
}

/********************************************************************/
/* Function: vvUpdatef                                               */
/* Modifies the current view volume as necessary.                   */
/********************************************************************/

void vvUpdatef(const float *position, VIEWPARAM *vp) {
  if (position[0] < vp->min[0])
    vp->min[0] = position[0];
  else if (position[0] > vp->max[0])
    vp->max[0] = position[0];

  if (position[1] < vp->min[1])
    vp->min[1] = position[1];
  else if (position[1] > vp->max[1])
    vp->max[1] = position[1];

  if (position[2] < vp->min[2])
    vp->min[2] = position[2];
  else if (position[2] > vp->max[2])
    vp->max[2] = position[2];
}

/********************************************************************/
/* Puts appropriate routines in the dispatch table and makes other  */
/* settings depending on drawing and viewing parameters, such as    */
/* the drawing parameter shade mode.                                */
/********************************************************************/
turtleDrawDispatcher *vvSetDispatcher(__attribute__((unused))
                                      const DRAWPARAM *dr,
                                      __attribute__((unused))
                                      const VIEWPARAM *vw) {
  return (&viewVolumeRoutines);
}

/********************************************************************/
/* Function: objCircleB2D                                            */
/********************************************************************/

void vvCircleB2D(const TURTLE *tu, __attribute__((unused)) const DRAWPARAM *dr,
                 const VIEWPARAM *vw, float diameter, float width) {
    vvCircle2D(tu,dr,vw,diameter + width);
}

void vvCircleB3D(const TURTLE *tu, __attribute__((unused)) const DRAWPARAM *dr,
                 const VIEWPARAM *vw, float diameter, float width) {
    vvCircle3D(tu,dr,vw,diameter + width);
}
