/* ouputs the graphics in gls format */

#ifdef WIN32
#include "warningset.h"
#endif

#include "platform.h"
#include <string.h>
#include <stdio.h>

#ifndef WIN32
#include <unistd.h>
#endif

#include "control.h"
#include "interpret.h"
#include "outputgls.h"
#include "patch.h"
#include "textures.h"
#include "mesh.h"
#include "irisGL.h"
#ifdef CPFG_ENVIRONMENT
#include "comm_lib.h"
#endif
#include "matrix.h"
//#include "comlineparam.h"

/*** local prototypes ***/
int glsSetup(TURTLE *, DRAWPARAM *, VIEWPARAM *);
void glsStartNode(TURTLE *, DRAWPARAM *, VIEWPARAM *, float /* length */,
                  char /* symbol */);
void glsEndNode(TURTLE *, DRAWPARAM *, VIEWPARAM *, char /* symbol */);
void glsStartBranch(TURTLE *, DRAWPARAM *, VIEWPARAM *);
void glsEndBranch(TURTLE *, DRAWPARAM *, VIEWPARAM *);
void glsStartPolygon(POLYGON *, TURTLE *, DRAWPARAM *, VIEWPARAM *);
void glsEndPolygon(POLYGON *, TURTLE *, DRAWPARAM *, VIEWPARAM *);
void glsSetColour(const TURTLE *, const DRAWPARAM *, const VIEWPARAM *);
void glsSetLineWidth(const TURTLE *, const DRAWPARAM *, const VIEWPARAM *);
void glsCircle2D(const TURTLE *, const DRAWPARAM *, VIEWPARAM *,
                 float /* radius */);
void glsCircle3D(const TURTLE *, const DRAWPARAM *, VIEWPARAM *,
                 float /* radius */);
void glsCircleB2D(const TURTLE *tu, const DRAWPARAM *dr, const VIEWPARAM *vw,
                  float diameter, float width);
void glsCircleB3D(const TURTLE *tu, const DRAWPARAM *dr, const VIEWPARAM *vw,
                  float diameter, float width);
void glsSphere(const TURTLE *, const DRAWPARAM *, VIEWPARAM *,
               float /* radius */);
void glsLabel(const TURTLE *, DRAWPARAM *, const VIEWPARAM *,
              const char * /* label */, int /* parameters */,
              const float * /* values */); /* JH1 */
void glsBlackBox(const TURTLE *, const DRAWPARAM *, const VIEWPARAM *,
                 const StringModule * /*module*/,
                 const StringModule * /*submodule*/);
void glsPredefinedSurface(TURTLE *, DRAWPARAM *, VIEWPARAM *, char /* ID */,
                          double, double, double);
void glsLdefinedSurface(StringModule *, TURTLE *, DRAWPARAM *, VIEWPARAM *);
void glsFinishUp(TURTLE *, DRAWPARAM *, VIEWPARAM *);
void glsRenderTriangle(const float *p1, const float *p2, const float *p3,
                       const DRAWPARAM *);

void glsColor(int color);
void glsStartTmesh(void);
void glsNormal(const float normal[3]);
void glsTmeshVertex(const float position[3]);
void glsEndTmesh(void);
int glsStartTexture(int index);
void glsSetTexCoord(const float coords[2]);
void glsEndTexture(int index);
void glsStartNewGrid(TURTLE *tu, DRAWPARAM *dr, VIEWPARAM *vw, int *size);
static void output_material(int index);

/* The structure for dispatching rendering routines */
turtleDrawDispatcher glsDrawRoutines = {
    glsSetup,        glsStartNode,         glsEndNode,         glsStartBranch,
    glsEndBranch,    glsStartPolygon,      glsEndPolygon,      glsSetColour,
    glsSetLineWidth, glsCircle2D,          glsCircle3D,        glsSphere,
    glsBlackBox,     glsPredefinedSurface, glsLdefinedSurface, glsLabel,
    glsFinishUp,     glsRenderTriangle,    glsStartTexture,    glsEndTexture,
    StartTmeshT,     TmeshVertexT,         EndTmeshT,          glsCircleB2D,
    glsCircleB3D};

/* flag indicating that nodes should be joined */
static int connectNode;

static int current_material;

FILE *glsfp;
int output_type;

/********************************************************************/
/* Function: gls_puts                                               */
/* return non-zero if there are problems                            */
/********************************************************************/
int gls_puts(char *string) {
  switch (output_type) {
#ifdef CPFG_ENVIRONMENT
  case TYPE_ENVIRONMENT:
    return CMSendString(CMIN_MASTER_INDEX, string);
#endif
#if CPFG_VERSION >= 3200
  case TYPE_SLAVE:
    return CSSendString(0, string);
#endif
  case TYPE_GLS:
    return fputs(string, glsfp) == EOF;
  }
  return 1;
}

int gls_print_float(char *format, float val) {
  char buffer[2048];

  sprintf(buffer, format, val);

  switch (output_type) {
#ifdef CPFG_ENVIRONMENT
  case TYPE_ENVIRONMENT:
    return CMSendString(CMIN_MASTER_INDEX, buffer);
#endif
#if CPFG_VERSION >= 3200
  case TYPE_SLAVE:
    return CSSendString(0, buffer);
#endif
  case TYPE_GLS:
    return fputs(buffer, glsfp) == EOF;
  }
  return 1;
}

/********************************************************************/
/* Function: glsSetup                                               */
/* return non-zero if there are problems                            */
/********************************************************************/

int glsSetup(TURTLE *tu, DRAWPARAM *dr, VIEWPARAM *vw) {
  float volumeCentre[3];
  mtype matrix[16];
  material_type *mat;
  light_parms *light;
  int i, j;

  current_material = -1;

  output_type = dr->output_type;

  switch (output_type) {
  case TYPE_GLS:
    if ((glsfp = clp.savefp[SAVE_GLS]) == NULL) {
      return 1;
    }
    /* no break! */

#if CPFG_VERSION >= 3200
  case TYPE_SLAVE:
#endif
    /* handle rotations */
    for (i = 0; i <= 2; i++)
      volumeCentre[i] = (vw->min[i] + vw->max[i]) / 2.0;

    /* determine transformation */
    glGetFloatv(GL_PROJECTION_MATRIX, matrix);
    InitializeMatrixStack(); /* starts witch identity */
    MultMatrix(matrix);

    Translate(volumeCentre);
    Rotate4(0.1 * vw->zRotation, 0.0, 0.0, 1.0);
    Rotate4(0.1 * vw->xRotation, 1.0, 0.0, 0.0);
    Rotate4(0.1 * vw->yRotation, 0.0, 1.0, 0.0);
    Translate3(-volumeCentre[0], -volumeCentre[1], -volumeCentre[2]);
    GetMatrix(matrix);

    /* output as viewing matrix */
    gls_puts("matrixmode 1\nloadmatrix\n");
    for (i = 0; i < 4; i++) {
      for (j = 0; j < 4; j++)
        gls_print_float("%g ", matrix[access(i, j)]);
      gls_puts("\n");
    }
    gls_puts("matrixmode 0\nloadidentity\n");

    /* lights */
    i = 0;
    while ((light = get_light(i++)) != NULL) {
      gls_puts("light");
      for (j = 0; j < 4; j++)
        gls_print_float(" %g", light->position[j]);
      gls_puts("\n");
    }

    if (clp.ismaterialfile) {
      /* background is the emissive color of material 0 (wrt reference */
      /* colormap) */
      my_getmaterial((Colorindex)clp.colormap, &mat);
      gls_print_float("clear %g ", mat->emissive[0]);
      gls_print_float("%g ", mat->emissive[1]);
      gls_print_float("%g\n", mat->emissive[2]);
    }

    output_material(dr->color_index);
    break;

  case TYPE_ENVIRONMENT:
    /* materials and viewing matrix are not sent to the environment */
    break;
  }

  /* no segment created yet */
  connectNode = FALSE;

  return 0;
}

/********************************************************************/
/* Function: glsStartNode                                           */
/********************************************************************/
struct {
  float position[3], left[3], up[3], heading[3];
  float line_width;
} startNode;

float segmentLength;

void glsStartNode(TURTLE *tu, DRAWPARAM *dr,
                  VIEWPARAM *vw, float length,
                  char symbol) {
  int c;

  for (c = 0; c < 3; c++) {
    startNode.position[c] = tu->position[c];
    startNode.heading[c] = tu->heading[c];
    startNode.left[c] = tu->left[c];
    startNode.up[c] = tu->up[c];
  }
  startNode.line_width = tu->line_width;

  segmentLength = length;

  /* this node should be connected to the next */
  connectNode = TRUE;

  output_material(tu->color_index);
}

/********************************************************************/
/* Function: glsEndNode                                             */
/********************************************************************/

void glsEndNode(TURTLE *tu, DRAWPARAM *dr,
                VIEWPARAM *vw,
                char symbol) {
  int c;

  if (connectNode) {
    /* finishup the cone */
    connectNode = FALSE;

    gls_puts("pushmatrix\ntranslate");
    for (c = 0; c < 3; c++)
      gls_print_float(" %f", startNode.position[c]);

    gls_puts("\nmultmatrix\n");

    for (c = 0; c < 3; c++) {
      gls_print_float(" %f", startNode.up[c]);
    }
    gls_puts(" 0\n");

    for (c = 0; c < 3; c++) {
      gls_print_float(" %f", startNode.heading[c]);
    }
    gls_puts(" 0\n");

    for (c = 0; c < 3; c++) {
      gls_print_float(" %f", -startNode.left[c]);
    }
    gls_puts(" 0\n");
    gls_puts("0 0 0 1\n");

    if (startNode.line_width == tu->line_width)
      gls_print_float("cylinder %f", startNode.line_width / 2.0);
    else {
      gls_print_float("cone %f", startNode.line_width / 2.0);
      gls_print_float(" %f", tu->line_width / 2.0);
    }
    gls_print_float(" %f\npopmatrix\n", segmentLength);
  }
}

/********************************************************************/
/* Function: glsStartBranch                                         */
/* No action                                                        */
/********************************************************************/

void glsStartBranch(TURTLE *tu,
                    DRAWPARAM *dr,
                    VIEWPARAM *vw) {}

/********************************************************************/
/* Function: glsEndBranch                                           */
/* No action                                                        */
/********************************************************************/

void glsEndBranch(TURTLE *tu,
                  DRAWPARAM *dr,
                  VIEWPARAM *vw) {}

/********************************************************************/
/* Function: glsStartPolygon                                        */
/* No action                                                        */
/********************************************************************/

void glsStartPolygon(POLYGON *polygon,
                     TURTLE *tu,
                     DRAWPARAM *dr,
                     VIEWPARAM *vw) {}

/********************************************************************/
/* Function: glsEndPolygon                                          */
/* Output polygon vertices                                          */
/********************************************************************/

void glsEndPolygon(POLYGON *polygon, TURTLE *tu,
                   DRAWPARAM *dr,
                   VIEWPARAM *vw) {
  int i, j;

  output_material(tu->color_index);

  gls_puts("polygon\n");

  for (i = 0; i < polygon->edge_count; i++) {
    for (j = 0; j < 3; j++)
      gls_print_float(" %g", (polygon->vertex[i]).position[j]);
    gls_puts("\n");
  }
}

/********************************************************************/
/* Function: glsSetColour                                           */
/* No action ???????????                                            */
/********************************************************************/

void glsSetColour(const TURTLE *tu, const DRAWPARAM *dr,
                  const VIEWPARAM *vw) {
  output_material(tu->color_index);
}

/********************************************************************/
/* Function: glsSetLineWidth                                        */
/* No action                                                        */
/********************************************************************/

void glsSetLineWidth(const TURTLE *tu,
                     const DRAWPARAM *dr,
                     const VIEWPARAM *vw) {}

/********************************************************************/
/* Function: glsCircle2D                                            */
/********************************************************************/

void glsCircle2D(const TURTLE *tu, const DRAWPARAM *dr,
                 VIEWPARAM *vw,
                 float diameter) {
  output_material(tu->color_index);
  /* NOT FINISHED! */
}

/********************************************************************/
/* Function: glsCircle3D                                            */
/********************************************************************/

void glsCircle3D(const TURTLE *tu, const DRAWPARAM *dr,
                 VIEWPARAM *vw,
                 float diameter) {
  output_material(tu->color_index);
  /* NOT FINISHED! */
}

/********************************************************************/
/* Function: objCircle2D                                            */
/********************************************************************/

void glsCircleB2D(const TURTLE *tu,
                  const DRAWPARAM *dr,
                  const VIEWPARAM *vw,
                  float diameter,
                  float width) {}

void glsCircleB3D(const TURTLE *tu,
                  const DRAWPARAM *dr,
                  const VIEWPARAM *vw,
                  float diameter,
                  float width) {}

/********************************************************************/
/* Function: glsSphere                                              */
/********************************************************************/

void glsSphere(const TURTLE *tu, const DRAWPARAM *dr,
               VIEWPARAM *vw, float diameter) {
  int j;

  output_material(tu->color_index);

  gls_puts("pushmatrix\ntranslate");
  for (j = 0; j < 3; j++)
    gls_print_float(" %g", tu->position[j]);

  gls_print_float("\nsphere %g\npopmatrix\n", diameter / 2.0);
}

/********************************************************************/
/* Function: glsBLackBox                                             */
/********************************************************************/

void glsBlackBox(const TURTLE *tu,
                 const DRAWPARAM *dr,
                 const VIEWPARAM *vw,
                 const StringModule *module,
                 const StringModule *submodule) {}

/********************************************************************/
/* Function: glsLabel                  JH1                          */
/********************************************************************/

void glsLabel(const TURTLE *tu,
              DRAWPARAM *dr,
              const VIEWPARAM *vw,
              const char *label,
              int parameteri,
              const float values[]) {}

/********************************************************************/
/* Function: glsStartTexture                                        */
/* No action.                                                       */
/********************************************************************/
int glsStartTexture(int index) {
  /* textures ignored */
  return 0;
}

/********************************************************************/
void glsEndTexture(int index) {}

/********************************************************************/
/* Function: glsPredefinedSurface                                    */
/********************************************************************/

void glsPredefinedSurface(TURTLE *tu, DRAWPARAM *dr, VIEWPARAM *vw, char id,
                          double sX, double sY, double sZ) {
  int c;

  if (sX != 0.0) {
    output_material(tu->color_index);

    gls_puts("pushmatrix\n");

    gls_puts("multmatrix\n");

    for (c = 0; c < 3; c++) {
      gls_print_float(" %f", tu->left[c]);
    }
    gls_puts(" 0\n");

    for (c = 0; c < 3; c++) {
      gls_print_float(" %f", tu->heading[c]);
    }
    gls_puts(" 0\n");

    for (c = 0; c < 3; c++) {
      gls_print_float(" %f", tu->up[c]);
    }
    gls_puts(" 0\n");
    for (c = 0; c < 3; c++)
      gls_print_float(" %f", tu->position[c]);
    gls_puts(" 1\n");

    gls_puts("scale ");
    gls_print_float(" %g", sX);
    gls_print_float(" %g", sY);
    gls_print_float(" %g", sZ);

    gls_puts("\n");

    draw_surface_tmesh(tu, id, dr, vw, StartTmeshT, TmeshVertexT, EndTmeshT);

    gls_puts("popmatrix\n");
  }
}

/********************************************************************/
/* Function: glsLdefinedSurface                                      */
/* No action                                                        */
/********************************************************************/

void glsLdefinedSurface(StringModule *module, TURTLE *tu, DRAWPARAM *dr,
                        VIEWPARAM *vw) {
  dr->gllighting = 1;
  dr->vertexbound = 1;
  dr->ourlighting = 0;
  dr->texture = 0;

  output_material(tu->color_index);

  SurfaceTmeshDraw(module, tu, dr, vw, StartTmeshT, TmeshVertexT, EndTmeshT);
}

/********************************************************************/
/* Function: glsFinishUp                                             */
/* No action                                                        */
/********************************************************************/

void glsFinishUp(TURTLE *tu,
                 DRAWPARAM *dr,
                 VIEWPARAM *vw) {

  if (output_type == TYPE_GLS) {
    if (clp.savefp[SAVE_GLS] != stdin)
      fclose(clp.savefp[SAVE_GLS]);
    clp.savefp[SAVE_GLS] = NULL;
  }
}

/********************************************************************/
/* Function: glsRenderTriangle                                       */
/* "rendegls" triangle                                               */
/********************************************************************/

void glsRenderTriangle(const float *p1, const float *p2, const float *p3,
                       const DRAWPARAM *dr) {
  int j;
  char normals;

  normals =
      (dr->ourlighting || dr->gllighting) && (dr->render_mode == RM_SHADED);

  if (output_type == TYPE_ENVIRONMENT)
    normals = 0;

  if (normals)
    gls_puts("polygonuv\n");
  else
    gls_puts("polygon\n");

  for (j = 0; j < 3; j++)
    gls_print_float(" %.5g", p1[ePOINT * 3 + j]);

  if (normals)
    for (j = 0; j < 3; j++)
      gls_print_float(" %.4g", -p1[eNORMAL * 3 + j]);
  gls_puts("\n");

  for (j = 0; j < 3; j++)
    gls_print_float(" %.5g", p2[ePOINT * 3 + j]);

  if (normals)
    for (j = 0; j < 3; j++)
      gls_print_float(" %.4g", -p2[eNORMAL * 3 + j]);
  gls_puts("\n");

  for (j = 0; j < 3; j++)
    gls_print_float(" %.5g", p3[ePOINT * 3 + j]);

  if (normals)
    for (j = 0; j < 3; j++)
      gls_print_float(" %.4g", -p3[eNORMAL * 3 + j]);
  gls_puts("\n");
}

/********************************************************************/
static void output_material(int index) {
  short r, g, b;
  int j;

  if (index == current_material)
    return;

  current_material = index;

  gls_puts("material\n");

  if (clp.ismaterialfile) {
    struct material_type *mat;

    my_getmaterial((Colorindex)index, &mat);

    /* ambient */
    for (j = 0; j < 4; j++)
      gls_print_float(" %g", mat->ambient[j]);
    gls_puts("\n");

    /* diffuse */
    for (j = 0; j < 4; j++)
      gls_print_float(" %g", mat->diffuse[j]);
    gls_puts("\n");

    /* specular */
    for (j = 0; j < 4; j++)
      gls_print_float(" %g", mat->specular[j]);
    gls_puts("\n");

    /* emissive */
    for (j = 0; j < 4; j++)
      gls_print_float(" %g", mat->emissive[j]);
    gls_puts("\n");

    /* shininess */
    gls_print_float(" %g\n", mat->shininess);
  } else {
    my_getmcolor((Colorindex)index, &r, &g, &b);
    /* ambient */
    gls_print_float(" %g", r / 2550.0);
    gls_print_float(" %g", g / 2550.0);
    gls_print_float(" %g 0\n", b / 2550.0);

    /* diffuse */
    gls_print_float(" %g", r / 255.0);
    gls_print_float(" %g", g / 255.0);
    gls_print_float(" %g 0\n", b / 255.0);

    /* specular and emissive is 0 */
    gls_puts(" 0 0 0 0  0 0 0 0  0\n");
  }
}

/********************************************************************/
/* Puts appropriate routines in the dispatch table and makes other  */
/* settings depending on drawing and viewing parameters, such as    */
/* the drawing parameter shade mode.                                */
/********************************************************************/
turtleDrawDispatcher *glsSetDispatcher(DRAWPARAM *dr,
                                       const VIEWPARAM *vw) {
  return (&glsDrawRoutines);
}
