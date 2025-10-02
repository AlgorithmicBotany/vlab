/********************************************************************/
/*                           postscript.c              		        */
/*                                                                  */
/* postscript provides interpret routines for producing postscript  */
/* compatible output files                                          */
/* Postscript output is correct only for 2D figures!!               */
/* For 3D Z is ignored.                                             */
/* Routines are dispatched via the turtleDrawDispatcher             */
/*                                                                  */
/********************************************************************/
/*
        CREATED: August 1991 BY: Jim at Apple
        UPDATED: September 1992 BY: Mark Hammel
                Added proper header and shading for polygons
   MODIFIED: March-June 1994 BY: Radek
             ansi standard + prepared for C++
             output in parallel projection exactly matches the screen (even
             after rotation by mouse)

             by May 95  BY: Radek
             triangles for generalized cylinders and surfaces
             labels - although font names in postscript are often different
             view is the same as
             routines for flat and cylindrical nodes
*/
// to avoid GLu Warning due to deprecated function on MacOs
#define GL_SILENCE_DEPRECATION

#ifdef WIN32
#include "warningset.h"
#endif

#include <stdio.h>
#include <math.h>
#include <ctype.h>

int custom_isnan(double var) {
  volatile double d = var;
  if (d != d)
    return 1;
  else
    return 0;
}

#include "platform.h"
#include "control.h"
#include "interpret.h"
#include "postscript.h"
#include "utility.h"
#include "irisGL.h"
#include "patch.h"
#include "mesh.h"
#include "textures.h"
#include "indices.h"
#define WARNING_LVL 0

/*********** prototypes ***************/
int psSetup(TURTLE *, DRAWPARAM *, VIEWPARAM *);
void psStartNode(TURTLE *, DRAWPARAM *, VIEWPARAM *, float /* length */,
                 char /* symbol */);
void psEndNode(TURTLE *, DRAWPARAM *, VIEWPARAM *, char /* symbol */);
void psStartBranch(TURTLE *, DRAWPARAM *, VIEWPARAM *);
void psEndBranch(TURTLE *, DRAWPARAM *, VIEWPARAM *);
void psStartPolygon(POLYGON *, TURTLE *, DRAWPARAM *, VIEWPARAM *);
void psEndPolygon(POLYGON *, TURTLE *, DRAWPARAM *, VIEWPARAM *);
void psSetColour(const TURTLE *, const DRAWPARAM *, const VIEWPARAM *);
void psSetTexture(TURTLE *, DRAWPARAM *, VIEWPARAM *);
void psSetLineWidth(const TURTLE *, const DRAWPARAM *, const VIEWPARAM *);
void psCircle2D(const TURTLE *, const DRAWPARAM *, VIEWPARAM *,
                float /* radius */);
void psCircle3D(const TURTLE *, const DRAWPARAM *, VIEWPARAM *,
                float /* radius */);
void psCircleB2D(const TURTLE *tu, const DRAWPARAM *dr, const VIEWPARAM *vw,
                 float diameter, float width);
void psCircleB3D(const TURTLE *tu, const DRAWPARAM *dr, const VIEWPARAM *vw,
                 float diameter, float width);
void psSphere(const TURTLE *, const DRAWPARAM *, VIEWPARAM *,
              float /* radius */);
void psLabel(const TURTLE *, DRAWPARAM *, const VIEWPARAM *,
             const char * /* label */, int /* parameters */,
             const float * /* values */); /* JH1 */
void psBlackBox(const TURTLE *, const DRAWPARAM *, const VIEWPARAM *,
                const StringModule * /*module*/,
                const StringModule * /*submodule*/);
void psPredefinedSurface(TURTLE *, DRAWPARAM *, VIEWPARAM *, char /* ID */,
                         double, double, double /* scale */);
void psLdefinedSurface(StringModule *, TURTLE *, DRAWPARAM *, VIEWPARAM *);
void psFinishUp(TURTLE *, DRAWPARAM *, VIEWPARAM *);
void psRenderTriangle(const float *p1, const float *p2, const float *p3,
                      const DRAWPARAM *);

/* The structure for dispatching rendering routines */
turtleDrawDispatcher postscriptDrawRoutines = {
    psSetup,        psStartNode,         psEndNode,         psStartBranch,
    psEndBranch,    psStartPolygon,      psEndPolygon,      psSetColour,
    psSetLineWidth, psCircle2D,          psCircle3D,        psSphere,
    psBlackBox,     psPredefinedSurface, psLdefinedSurface, psLabel,
    psFinishUp,     psRenderTriangle,    voidStartTexture,  voidEndTexture,
    StartTmeshT,    TmeshVertexT,        EndTmeshT,         psCircleB2D,
    psCircleB3D};

/* flag used by StartNode and EndNode indicating that nodes should be joined */
static int connectNode;

/* flag indicating that colour should be changed */
static int changeColour;

static short currentColor;

/* flag indicating that linewidth should be changed - used only for pixel nodes
 */
static int changeLineWidth;

/* model view matrix - to keep object properly oriented (as on the screen) */
static GLdouble mat[16];

/* poiter, because for surfaces another matrix is used */
static GLdouble *model_mat = mat;

/* number of triangles that make up sphere */
extern int noSphereTris;

/********************************************************************/
static double transform_width(const TURTLE *tu, double width) {
  extern double viewNormal[3];
  extern VIEWPARAM viewparam;
  double wend, wstart, perp[3];
  double res[2];
  int i;

  if (!clp.graphics_output)
    for (i = 0; i < 3; i++)
      viewNormal[i] = viewparam.view_normal[i];

  /* get a vector perpendicular to viewNormal and line direction */
  DCrossProduct(tu->heading, viewNormal, perp);

  DNormalize(perp);

  if (perp[0] * perp[0] + perp[1] * perp[1] + perp[2] * perp[2] == 0) {
    perp[0] = tu->up[0];
    perp[1] = tu->up[1];
    perp[2] = tu->up[2];
  }

  perp[0] = tu->position[0] + perp[0] * width;
  perp[1] = tu->position[1] + perp[1] * width;
  perp[2] = tu->position[2] + perp[2] * width;

  /* project the vector start- and end- point to the drawing plane */
  wend = model_mat[3] * perp[0] + model_mat[3 + 4] * perp[1] +
         model_mat[3 + 8] * perp[2] + model_mat[3 + 12];

  if (wend != 0)
    wend = 1.0 / wend;
  else
    wend = 1.0; /* just to prevent a crash - shouldn't happen */

  wstart = model_mat[3] * tu->position[0] + model_mat[3 + 4] * tu->position[1] +
           model_mat[3 + 8] * tu->position[2] + model_mat[3 + 12];

  if (wstart != 0)
    wstart = 1.0 / wstart;
  else
    wstart = 1.0; /* just to prevent a crash - shouldn't happen */

  for (i = 0; i < 2; i++) /* z is ignored */
    res[i] =
        (model_mat[i] * perp[0] + model_mat[i + 4] * perp[1] +
         model_mat[i + 8] * perp[2] + model_mat[i + 12]) *
            wend -
        (model_mat[i] * tu->position[0] + model_mat[i + 4] * tu->position[1] +
         model_mat[i + 8] * tu->position[2] + model_mat[i + 12]) *
            wstart;

  return sqrt(res[0] * res[0] + res[1] * res[1]);
}

/********************************************************************/
static void transform_point(const double *x, double *res) {
  double w;
  int i;

  w = model_mat[3] * x[0] + model_mat[3 + 4] * x[1] + model_mat[3 + 8] * x[2] +
      model_mat[3 + 12];

  if (w != 0)
    w = 1.0 / w;
  else
    w = 1.0; /* just to prevent a crash - shouldn't happen */

  for (i = 0; i < 2; i++) /* z is ignored */
    res[i] = (model_mat[i] * x[0] + model_mat[i + 4] * x[1] +
              model_mat[i + 8] * x[2] + model_mat[i + 12]) *
             w;
}

static void transform_pointf(const float *x, double *res) {
  double w;
  int i;

  w = model_mat[3] * x[0] + model_mat[3 + 4] * x[1] + model_mat[3 + 8] * x[2] +
      model_mat[3 + 12];

  if (w != 0)
    w = 1.0 / w;
  else
    w = 1.0; /* just to prevent a crash - shouldn't happen */

  for (i = 0; i < 2; i++) /* z is ignored */
    res[i] = (model_mat[i] * x[0] + model_mat[i + 4] * x[1] +
              model_mat[i + 8] * x[2] + model_mat[i + 12]) *
             w;
}

/********************************************************************/
/* Function: psSetup                                                */
/* return non-zero if there are problems                            */
/********************************************************************/

// extern void CalculateWindow(VIEWPARAM *viewPtr, RECTANGLE *windowPtr); // MC
// - moved to control.h

float wx, wy; /* size of the drawing area */

int psSetup(TURTLE *tu, DRAWPARAM *dr, VIEWPARAM *vw) {
  float scale, sx, sy, tmp;
  int x1, x2, y1, y2;
  short red, green, blue;
  float volumeCentre[3];
  int i;
  RECTANGLE viewWindow;
  float aspect, font_size = 12.0;
  const char *ptr = NULL;
  char fontname[80] = "Courier-Bold";
  material_type *mat;

  if (clp.savefp[SAVE_POSTSCRIPT] == NULL) {
    return 1;
  }

  fprintf(clp.savefp[SAVE_POSTSCRIPT], "%%!PS-Adobe\n");

  for (i = 0; i <= 2; i++) {
    volumeCentre[i] = (vw->min[i] + vw->max[i]) / 2.0f;
  }

  /* Calculate size of window (wx, wy), translation of L-system object
  (tx, ty), and factor to scale L-system object (scale) */

  sx = clp.xsize;
  sy = clp.ysize;

  scale = 468.0 / sx;
  tmp = 648.0 / sy;
  if (tmp >= scale) {
    wx = 468;
    wy = sy * scale;
  } else {
    scale = tmp;
    wy = 648;
    wx = sx * scale;
  }

  /* Any inaccuracies in the bounding box should be <=1 and are due to the
  assumption that the page size is 468x648pts and roundoff error. */
  x1 = (int)floor((468.0 - wx) / 2 + 72);
  x2 = (int)floor((float)x1 + wx);
  y1 = (int)floor((648.0 - wy) / 2 + 72);
  y2 = (int)floor((float)y1 + wy);

  fprintf(clp.savefp[SAVE_POSTSCRIPT], "%%%%BoundingBox: %d %d %d %d\n", x1, y1,
          x2, y2);

  fprintf(clp.savefp[SAVE_POSTSCRIPT], "%d setlinecap\n", dr->ps_linecap);

  /* creating the projection matrix - as in SetView() in control.c */

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  
  /* set desired projection */
  glLoadIdentity();

  /* simulating glViewport command */
  glTranslatef((float)x1, (float)y1, 0.0);
  glScalef((float)(x2 - x1) * 0.5f, (float)(y2 - y1) * 0.5f, 1.0f);
  glTranslatef(1.0, 1.0, 0.0);

  if (vw->parallel_projection_on) {
    aspect = (float)(clp.ysize) / (float)(clp.xsize);

    CalculateWindow(vw, &viewWindow, aspect);

    glOrtho(viewWindow.left, viewWindow.right, viewWindow.bottom,
            viewWindow.top, vw->front_dist, vw->back_dist);
  } else {
    aspect = (float)(clp.ysize) / (float)(clp.xsize);
    gluPerspective(vw->viewing_angle, aspect, vw->front_dist, vw->back_dist);
  }

  /* distance depends on the scaling */
  gluLookAt(vw->vrp[0] + (vw->viewpoint[0] - vw->vrp[0]) / vw->scale,
            vw->vrp[1] + (vw->viewpoint[1] - vw->vrp[1]) / vw->scale,
            vw->vrp[2] + (vw->viewpoint[2] - vw->vrp[2]) / vw->scale,
            vw->vrp[0], vw->vrp[1], vw->vrp[2], vw->view_up[0], vw->view_up[1],
            vw->view_up[2]);

  glTranslatef(volumeCentre[0], volumeCentre[1], volumeCentre[2]);
  glRotatef(0.1 * vw->zRotation, 0.0, 0.0, 1.0);
  glRotatef(0.1 * vw->xRotation, 1.0, 0.0, 0.0);
  glRotatef(0.1 * vw->yRotation, 0.0, 1.0, 0.0);
  glTranslatef(-volumeCentre[0], -volumeCentre[1], -volumeCentre[2]);

  glGetDoublev(GL_PROJECTION_MATRIX, model_mat);

  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);

  /* set font for labels - needs some work - better correspondence between X */
  /* and postscript font names needed  */

  /* get to the second '-' */
  ptr = dr->fontname;
  for (i = 0; i < 2; i++) {
    if ((ptr = strchr(ptr, '-')) == NULL) {
      Message("Cannot extract font name from %s. Default used!\n");
      goto lab1;
    }
    ptr++;
  }

  /* extract the font name */
  i = 0;
  while ((fontname[i] = *ptr) != '-') {
    if (*(ptr++) == 0)
      goto lab1;
    i++;
  }
  fontname[i] = '\0';

  /* first letter should be capital */
  fontname[0] = toupper(fontname[0]);

#ifdef __APPLE__
  if (strcmp(fontname, "Arial") == 0)
    strncpy(fontname, "Helvetica", 80);
#endif

  char w = *(++ptr);

  if ((w == 'b') || (w == 'm')) /* -bold- or -medium-*/
    strcat(fontname, "-Bold");

  /* extract the size */
  for (i = 0; i < 4; i++) {
    if ((ptr = 1 + strchr(ptr, '-')) == NULL) {
      Message("Cannot extract font size from %s. Default used!\n");
      goto lab1;
    }
  }

  sscanf(ptr, "%f", &font_size);

  font_size = dr->font_size;
  if ((font_size > 100) || (font_size <= 0)) {
    Warning("Warning Font Size cannot be read from XFont, defaut will be used, "
	    "check documentation if any discrepency with the model\n",WARNING_LVL);
    font_size = 12;
  }

lab1:
  font_size *= (float)(wx) / (float)clp.xsize;

  fprintf(clp.savefp[SAVE_POSTSCRIPT], "/%s findfont %.5f scalefont setfont\n",
          fontname, font_size);

  /* Draw background */
  fprintf(clp.savefp[SAVE_POSTSCRIPT], "newpath\n");
  fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f moveto\n", (float)x1, (float)y1);
  fprintf(clp.savefp[SAVE_POSTSCRIPT], "0 %f rlineto\n", wy);
  fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f 0 rlineto\n", wx);
  fprintf(clp.savefp[SAVE_POSTSCRIPT], "0 %f rlineto\n", -wy);
  fprintf(clp.savefp[SAVE_POSTSCRIPT], "closepath clip\n");
  /* everything what follows is clipped by this rectangle */

  if (!clp.ismaterialfile) {
    my_getmcolor((short)clp.colormap, &red, &green, &blue);
    fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f %f setrgbcolor\n",
            (float)red / 255, (float)green / 255, (float)blue / 255);
  } else {
    /* must get emissive color */
    my_getmaterial((short)clp.colormap, &mat);
    fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f %f setrgbcolor \n",
            mat->emissive[0], mat->emissive[1], mat->emissive[2]);
  }
  fprintf(clp.savefp[SAVE_POSTSCRIPT], "fill\n");

  /* no segment created yet */
  connectNode = FALSE;

  /* colour and linewidth should be set */
  changeColour = TRUE;
  currentColor = tu->color_index;

  if (dr->line_style == LS_PIXEL)
    changeLineWidth = TRUE;
  else
    fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f setlinewidth\n",
            0.001f * transform_width(tu, (double)tu->line_width));
  return 0;
}

/********************************************************************/
/* Function: psStartNode                                            */
/********************************************************************/

void psStartNode(TURTLE *tu, DRAWPARAM *dr,
                 VIEWPARAM *vw,
                 float length,
                 char symbol) {
  short red, green, blue;
  double pos[2];
  double width;

  if (changeColour) {
    /* add code */ /* this is GL specific!! */
    my_getmcolor((Colorindex)(tu->color_index), &red, &green, &blue);
    fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f %f setrgbcolor \n",
            red / 255.0f, green / 255.0f, blue / 255.0f);
    changeColour = FALSE;
  }
  if (changeLineWidth) {
    width = transform_width(tu, (double)tu->line_width);

    if (dr->line_style == LS_PIXEL) {
      /* assuming one pixel on clp.xsize */
      fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f setlinewidth\n",
              (float)(tu->line_width * wx / clp.xsize));
    } else if (dr->render_mode == RM_WIREFRAME) {
      fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f setlinewidth\n",
              (float)(width * 0.001));
    } else
      fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f setlinewidth\n", (float)(width));
    changeLineWidth = FALSE;
  }

  fprintf(clp.savefp[SAVE_POSTSCRIPT], "newpath\n");

  transform_point(tu->position, pos);
  if (!custom_isnan((float)pos[0]) && (!custom_isnan((float)pos[1]))) {
    fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f moveto\n", (float)(pos[0]),
            (float)(pos[1]));
    /* this node should be connected to the next */
    connectNode = TRUE;
  } else
    connectNode = FALSE;
}

/********************************************************************/
/* Function: psEndNode                                              */
/********************************************************************/

void psEndNode(TURTLE *tu, DRAWPARAM *dr, VIEWPARAM *vw,
               char symbol) {
  double pos[2];

  if (connectNode) {
    transform_point(tu->position, pos);
    if (!custom_isnan(pos[0]) && (!custom_isnan(pos[1]))) {
      fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f lineto\n", (float)(pos[0]),
              (float)(pos[1]));
      fprintf(clp.savefp[SAVE_POSTSCRIPT], "stroke\n");
    }
    connectNode = FALSE;

    if (dr->line_style == LS_PIXEL) {
      fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f setlinewidth\n",
              (float)(transform_width(tu, (double)tu->line_width) * 0.001));
      changeLineWidth = TRUE;
    }
  }
}

/********************************************************************/
/* Function: psStartCylOrFlatNode                                   */
/* Saves turtle state for use when the segment is ended             */
/********************************************************************/
static TURTLE startNode;
static float segmentLength;

void psStartCylOrFlatNode(TURTLE *tu, DRAWPARAM *dr,
                          VIEWPARAM *vw, float length,
                          char symbol) {
  TurtleCopy(&startNode, tu);

  segmentLength = length;
  /* this node should be connected to the next */
  connectNode = TRUE;
}

/********************************************************************/
/* Function: psEndFlatNode                                          */
/* Complete the segment; change colour and linewidth if necessary   */
/********************************************************************/

void psEndFlatNode(TURTLE *tu, DRAWPARAM *dr,
                   VIEWPARAM *vw,
                   char symbol) {
  extern float unitRectTopCoord[2][3];
  extern float unitRectBaseCoord[2][3];
  extern float unitRectNormal[3];
  static Matrix rotate = {{0.0, 0.0, 0.0, 0.0},
                          {0.0, 0.0, 0.0, 0.0},
                          {0.0, 0.0, 0.0, 0.0},
                          {0.0, 0.0, 0.0, 1.0}};
  short red, green, blue;
  GLdouble *old_mat;
  GLdouble local_mat[16];
  int coord;
  float topScale, cos_theta, nlightdir[3];
  double colinearTest;
  double epsilon = .000001;
  double tempUpVector[3];
  double tempLeftVector[3];
  double pos[2];

  extern double viewNormal[3];

  if (connectNode) {
    /* check that view and heading aren't colinear */
    /* if so there's no point displaying the polygon */
    colinearTest = fabs(DDotProduct(startNode.heading, viewNormal));
    if (colinearTest > 1.0 - epsilon && colinearTest < 1.0 + epsilon)
      return;

    /* calculate up and left so that the polygon faces the viewer */
    DCrossProduct(startNode.heading, viewNormal, tempLeftVector);
    DNormalize(tempLeftVector);
    DCrossProduct(tempLeftVector, startNode.heading, tempUpVector);

    /* Perform set up and transformations for the node  */
    /* store current model matrix */
    old_mat = model_mat;
    model_mat = local_mat;

    /* Because the orientation vectors from the interpretation
    ** are unit vectors their elements can be used directly as
    ** a transformation matrix	*/
    glPushMatrix();
    glLoadMatrixd(old_mat);
    glTranslatef(startNode.position[eX], startNode.position[eY],
                 startNode.position[eZ]);

    /* Because the orientation vectors from the calculation
    ** are orthogonal unit vectors their elements can be used
    ** directly as a transformation matrix	*/
    for (coord = 0; coord < 3; coord++) {
      rotate[0][coord] = tempLeftVector[coord];
      rotate[1][coord] = startNode.heading[coord];
      rotate[2][coord] = -tempUpVector[coord];
    }
    glMultMatrixf(&rotate[0][0]);
    glScalef((float)startNode.line_width, (float)segmentLength,
             (float)startNode.line_width);
    glGetDoublev(GL_MODELVIEW_MATRIX, model_mat);
    glPopMatrix();

    if (dr->ourlighting) {
      /* Transform the light direction to the flat node
              coordinate system. */
      VecMatMult(dr->light_dir, rotate, nlightdir);

      cos_theta = DotProduct(unitRectNormal, nlightdir);

      my_getmcolor((Colorindex)(startNode.color_index +
                                (float)dr->diff_coef * cos_theta),
                   &red, &green, &blue);
    } else
      my_getmcolor((Colorindex)startNode.color_index, &red, &green, &blue);

    fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f %f setrgbcolor\n",
            (float)red / 255, (float)green / 255, (float)blue / 255);

    if (dr->tapered_lines_on)
      /* modify top points to give the correct radius */
      /* after scaling by the startNode linewidth */
      topScale = tu->line_width / startNode.line_width;
    else
      topScale = 1.0;

    fprintf(clp.savefp[SAVE_POSTSCRIPT], "newpath\n");

    transform_pointf(unitRectBaseCoord[0], pos);
    if (custom_isnan(pos[0]) || (custom_isnan(pos[1])))
      return;

    fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f moveto\n", (float)pos[0],
            (float)pos[1]);

    transform_pointf(unitRectBaseCoord[1], pos);
    if (!custom_isnan(pos[0]) && (!custom_isnan(pos[1])))
      fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f lineto\n", (float)pos[0],
              (float)pos[1]);

    for (coord = 0; coord < 2; coord++) {
      unitRectTopCoord[coord][eX] = unitRectBaseCoord[coord][eX] * topScale;
      unitRectTopCoord[coord][eZ] = unitRectBaseCoord[coord][eZ] * topScale;
    }

    transform_pointf(unitRectTopCoord[1], pos);
    if (!custom_isnan(pos[0]) && (!custom_isnan(pos[1])))
      fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f lineto\n", (float)pos[0],
              (float)pos[1]);

    transform_pointf(unitRectTopCoord[0], pos);
    if (!custom_isnan(pos[0]) && (!custom_isnan(pos[1])))

      fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f lineto\n", (float)pos[0],
              (float)pos[1]);

    fprintf(clp.savefp[SAVE_POSTSCRIPT], "closepath\n");
    if (dr->render_mode != RM_WIREFRAME)
      fprintf(clp.savefp[SAVE_POSTSCRIPT], "fill\n");
    else
      fprintf(clp.savefp[SAVE_POSTSCRIPT], "stroke\n");

    model_mat = old_mat; /* restore old model matrix */

    connectNode = FALSE;
  }
}

/********************************************************************/
/* Function: psEndCylNode                                           */
/* Draw a truncated cone representing the segment;                  */
/********************************************************************/

void psEndCylNode(TURTLE *tu, DRAWPARAM *dr,
                  VIEWPARAM *vw,
                  char symbol) {
  static Matrix rotate = {{0.0, 0.0, 0.0, 0.0},
                          {0.0, 0.0, 0.0, 0.0},
                          {0.0, 0.0, 0.0, 0.0},
                          {0.0, 0.0, 0.0, 1.0}};

  short red, green, blue;
  GLdouble *old_mat;
  GLdouble local_mat[16];
  int vertex, next;
  float topScale, cos_theta, nlightdir[3];
  float width_diff, linelen;
  float normal[3]; /* for rotated normal */
  float *normalptr;
  float normal_xz = 0.0;
  char rot_normal = 0;
  double old_base_pos[2], old_top_pos[2];
  CYLINDER *cylinder;

  extern TURTLE startNode;
  extern float segmentLength;

  if ((connectNode) && (segmentLength != 0.0)) {
    /* Perform set up and transformations for the node  */
    /* store current model matrix */
    old_mat = model_mat;
    model_mat = local_mat;

    /* Because the orientation vectors from the interpretation
    ** are unit vectors their elements can be used directly as
    ** a transformation matrix	*/
    glPushMatrix();
    glLoadMatrixd(old_mat);
    glTranslatef(startNode.position[eX], startNode.position[eY],
                 startNode.position[eZ]);
    for (vertex = 0; vertex < 3; vertex++) {
      rotate[0][vertex] = startNode.up[vertex];
      rotate[1][vertex] = startNode.heading[vertex];
      rotate[2][vertex] = startNode.left[vertex];
    }
    glMultMatrixf(&rotate[0][0]);

    glScalef((float)startNode.line_width, (float)segmentLength,
             (float)startNode.line_width);

    glGetDoublev(GL_MODELVIEW_MATRIX, model_mat);
    glPopMatrix();

    if (dr->ourlighting) {
      /* Transform the light direction to the cylinder
              coordinate system. */
      VecMatMult(dr->light_dir, rotate, nlightdir);
    } else {
      my_getmcolor((Colorindex)startNode.color_index, &red, &green, &blue);

      fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f %f setrgbcolor \n",
              (float)red / 255, (float)green / 255, (float)blue / 255);
    }

    /* Draw the cylinder */
    if (dr->tapered_lines_on)
      /* modify top points to give the correct radius */
      /* after scaling by the startNode linewidth */
      topScale = tu->line_width / startNode.line_width;
    else
      topScale = 1.0;

    if ((dr->ourlighting)) {
      if (topScale != 1.0) {
        /* necessary to adjust normals */
        /* precompute parameters */

        /* lenght of a line on the segment's surface.
        Consider before transformations:
        - length of segment is 1.0
        - bottom width is 1.0
        - top width is topScale */
        width_diff = startNode.line_width - tu->line_width;
        linelen = (float)sqrt(
            (double)(segmentLength * segmentLength + width_diff * width_diff));

        /* N_Y/normal_len = width_diff/linelen */
        normal[eY] = width_diff / (linelen * segmentLength);
        /* this is y coordinates of all normals
        /segmentLength  corrects the effect of scaling */

        /* N_XZ/normal_len = segmentLength/line_len */
        normal_xz = segmentLength / (linelen * startNode.line_width);
        /* x and z coordinates of all normals should be multiplied
        by normal_xz */
        rot_normal = 1;
      } else
        /* use precomputed normal unitCylBase[i][NORMAL_X] */
        rot_normal = 0;
    }

    cylinder = FindCylinder(tu->cylinder_sides);

    transform_pointf(&cylinder->base[0][POINT_X], old_base_pos);
    cylinder->top[0][POINT_X] = cylinder->base[0][POINT_X] * topScale;
    cylinder->top[0][POINT_Z] = cylinder->base[0][POINT_Z] * topScale;
    transform_pointf(&cylinder->top[0][POINT_X], old_top_pos);

    for (vertex = 0; vertex < cylinder->sides; vertex++) {
      if (rot_normal) {
        normal[eX] = normal_xz * cylinder->base[vertex][NORMAL_X];
        normal[eZ] = normal_xz * cylinder->base[vertex][NORMAL_Z];
        normalptr = normal;
      } else
        normalptr = &cylinder->base[vertex][NORMAL_X];

      if (dr->ourlighting) {
        cos_theta = DotProduct(normalptr, nlightdir);

        my_getmcolor((Colorindex)(startNode.color_index +
                                  (float)dr->diff_coef * cos_theta),
                     &red, &green, &blue);

        fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f %f setrgbcolor \n",
                (float)red / 255, (float)green / 255, (float)blue / 255);
      }
      if (custom_isnan(old_base_pos[0]) || custom_isnan(old_base_pos[1]) ||
          custom_isnan(old_top_pos[0]) || custom_isnan(old_top_pos[1]))
        // we don't draw anything
        continue;

      fprintf(clp.savefp[SAVE_POSTSCRIPT],
              "newpath\n%f %f moveto\n%f %f lineto\n", old_base_pos[0],
              old_base_pos[1], old_top_pos[0], old_top_pos[1]);

      if ((next = vertex + 1) == cylinder->sides)
        next = 0;

      transform_pointf(&cylinder->base[next][POINT_X], old_base_pos);

      cylinder->top[next][POINT_X] = cylinder->base[next][POINT_X] * topScale;
      cylinder->top[next][POINT_Z] = cylinder->base[next][POINT_Z] * topScale;
      transform_pointf(&cylinder->top[next][POINT_X], old_top_pos);

      fprintf(clp.savefp[SAVE_POSTSCRIPT],
              "%f %f lineto\n%f %f lineto\nclosepath\n", old_top_pos[0],
              old_top_pos[1], old_base_pos[0], old_base_pos[1]);

      if (dr->render_mode != RM_WIREFRAME)
        fprintf(clp.savefp[SAVE_POSTSCRIPT], "fill\n");
      else
        fprintf(clp.savefp[SAVE_POSTSCRIPT], "stroke\n");
    }

    model_mat = old_mat; /* restore old model matrix */
    connectNode = FALSE;
  }
}

/********************************************************************/
/* Function: psStartBranch                                          */
/* No action                                                        */
/********************************************************************/

void psStartBranch(TURTLE *tu,
                   DRAWPARAM *dr,
                   VIEWPARAM *vw) {}

/********************************************************************/
/* Function: psEndBranch                                            */
/* Reset for new turtle position                                    */
/********************************************************************/

void psEndBranch(TURTLE *tu,
                 DRAWPARAM *dr,
                 VIEWPARAM *vw) {
  changeColour = TRUE;
  changeLineWidth = TRUE;
}

/********************************************************************/
/* Function: psStartPolygon                                         */
/* No action                                                        */
/********************************************************************/

void psStartPolygon(POLYGON *polygon, TURTLE *tu,
                    DRAWPARAM *dr,
                    VIEWPARAM *vw) {
  short red, green, blue;
  // save polygon colors
  if (changeColour) {
    /* add code */ /* this is GL specific!! */
    my_getmcolor((Colorindex)tu->color_index, &red, &green, &blue);

    fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f %f setrgbcolor\n",
            (float)red / 255, (float)green / 255, (float)blue / 255);
    changeColour = FALSE;
  }
}

/********************************************************************/
/* Function: psEndPolygon                                           */
/* render the polygon according to the last vertex colour           */
/* and the current shade mode                                       */
/********************************************************************/

void psEndPolygon(POLYGON *polygon, TURTLE *tu, DRAWPARAM *dr,
                  VIEWPARAM *vw) {
  int i;
  float cos_theta;
  double pos[2];
  short red, green, blue;

  my_getmcolor((Colorindex)tu->color_index, &red, &green, &blue);

  fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f %f setrgbcolor\n",
          (float)red / 255, (float)green / 255, (float)blue / 255);

  fprintf(clp.savefp[SAVE_POSTSCRIPT], "newpath\n");

  transform_point((polygon->vertex[0]).position, pos);

  if (!custom_isnan(pos[0]) && !custom_isnan(pos[1])) {
    fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f moveto\n", (float)pos[0],
            (float)pos[1]);

    for (i = 1; i < polygon->edge_count; i++) {
      transform_point((polygon->vertex[i]).position, pos);
      if (custom_isnan(pos[0]) || custom_isnan(pos[1]))
        continue;

      fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f lineto\n", (float)pos[0],
              (float)pos[1]);
    }
  }
  /* add code */ /* this is GL specific !! (getting colourmap colour) */
  switch (dr->render_mode) {
  case RM_FILLED:
    /* no new colour */
    fprintf(clp.savefp[SAVE_POSTSCRIPT], "closepath fill\n");
    break;

  case RM_INTERPOLATED:
    /* postscript can't handle interpolation */
    fprintf(clp.savefp[SAVE_POSTSCRIPT], "closepath fill\n");
    break;

  case RM_FLAT:
  case RM_SHADED:
    if (clp.ismaterialfile) {
      material_type *mat;
      float fred, fgreen, fblue;

      i = (i - 1) / 2;
      my_getmaterial((Colorindex)(polygon->vertex[i]).color_index, &mat);
      cos_theta = DFDotProduct((polygon->vertex[i]).up, dr->light_dir);
      fred = mat->ambient[0] + mat->diffuse[0] * fabs(cos_theta);
      fgreen = mat->ambient[1] + mat->diffuse[1] * fabs(cos_theta);
      fblue = mat->ambient[2] + mat->diffuse[2] * fabs(cos_theta);

      fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f %f setrgbcolor\n", fred,
              fgreen, fblue);
    }
    fprintf(clp.savefp[SAVE_POSTSCRIPT], "closepath fill\n");
    break;
#ifdef OLD_SHADE_MODES
  case B_SPLINE:
  case CLOSED_B_SPLINE:
    /* must add in spline interpretation */
    width = transform_width(tu, (double)tu->line_width);

    fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f setlinewidth\n", (float)(width));
    fprintf(clp.savefp[SAVE_POSTSCRIPT], "closepath\n");
    break;

  case TWO_SIDED:
    /* postscript doesn't use viewpoint - may get unwanted results */
    i = (i - 1) / 2;
    cos_theta = DFDotProduct((polygon->vertex[i]).up, dr->light_dir);
    cos2_theta = DFDotProduct((polygon->vertex[i]).up, vw->viewpoint);
    my_getmcolor((polygon->vertex[i]).color_index +
                     (Colorindex)((float)dr->diff_coef * cos_theta) -
                     ((cos2_theta >= 0) ? 0 : 256),
                 &Qt::red, &Qt::green, &Qt::blue);
    fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f %f setrgbcolor\n",
            (float)Qt::red / 255, (float)Qt::green / 255,
            (float)Qt::blue / 255);
    fprintf(clp.savefp[SAVE_POSTSCRIPT], "closepath fill\n");
    break;
#endif
  case RM_WIREFRAME:
    fprintf(clp.savefp[SAVE_POSTSCRIPT], "closepath\n");

    fprintf(clp.savefp[SAVE_POSTSCRIPT], "stroke\n");
    break;

  default:
    /* default to SIMPLE_FILL */
    fprintf(clp.savefp[SAVE_POSTSCRIPT], "closepath fill\n");
    break;
  }
}

/********************************************************************/
/* Function: psSetColour                                            */
/* Convert colour to gray scale and output                          */
/********************************************************************/

void psSetColour(const TURTLE *tu, const DRAWPARAM *dr,
                 const VIEWPARAM *vw) {
  changeColour = TRUE;
  currentColor = tu->color_index;
}

/********************************************************************/
void psRenderTriangle(const float *p1, const float *p2, const float *p3,
                      const DRAWPARAM *dr) {
  short red, green, blue;
  extern DRAWPARAM drawparam;
  double pos[2];

  if (dr->ourlighting)
    my_getmcolor((Colorindex)p1[COLOR_FRONT], &red, &green, &blue);
  else
    my_getmcolor((Colorindex)currentColor, &red, &green, &blue);

  fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f %f setrgbcolor\n",
          (float)red / 255, (float)green / 255, (float)blue / 255);

  fprintf(clp.savefp[SAVE_POSTSCRIPT], "newpath\n");

  transform_pointf(p1, pos);
  if (!custom_isnan(pos[0]) && !custom_isnan(pos[1])) {
    fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f moveto\n", (float)pos[0],
            (float)pos[1]);

    transform_pointf(p2, pos);
    if (!custom_isnan(pos[0]) && !custom_isnan(pos[1])) {
      if ((dr->render_mode == RM_WIREFRAME) && (p1[DRAW_LINE] == 0))
        fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f moveto\n", (float)pos[0],
                (float)pos[1]);
      else
        fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f lineto\n", (float)pos[0],
                (float)pos[1]);
    }
    transform_pointf(p3, pos);
    if (!custom_isnan(pos[0]) && !custom_isnan(pos[1])) {
      if ((dr->render_mode == RM_WIREFRAME) && (p2[DRAW_LINE] == 0))
        fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f moveto\n", (float)pos[0],
                (float)pos[1]);
      else
        fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f lineto\n", (float)pos[0],
                (float)pos[1]);
    }
  }

  /* add code */ /* this is GL specific !! (getting colormap color) */
  switch (drawparam.render_mode) {
#ifdef OLD_SHADE_MODES
  case B_SPLINE:
  case CLOSED_B_SPLINE:
    fprintf(clp.savefp[SAVE_POSTSCRIPT], "closepath\n");
    break;
#endif
  case RM_WIREFRAME:
    if ((dr->render_mode != RM_WIREFRAME) || (p3[DRAW_LINE] == 1)) {
      transform_pointf(p1, pos);
      if (!custom_isnan(pos[0]) && !custom_isnan(pos[1])) {
        fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f lineto\n", (float)pos[0],
                (float)pos[1]);
      }
    }

    fprintf(clp.savefp[SAVE_POSTSCRIPT], "stroke\n");
    break;

  default:
    /* default to SIMPLE_FILL */
    fprintf(clp.savefp[SAVE_POSTSCRIPT], "closepath fill\n");
    break;
  }
}

/********************************************************************/
/* Function: psSetLineWidth                                         */
/********************************************************************/

void psSetLineWidth(const TURTLE *tu,
                    const DRAWPARAM *dr,
                    const VIEWPARAM *vw) {
  changeLineWidth = TRUE;
}

/********************************************************************/
/* Function: psCircle2D                                             */
/* Assume wireframe for PostScript                                  */
/* Z coordinate is just dropped                                     */
/********************************************************************/

void psCircle2D(const TURTLE *tu, const DRAWPARAM *dr,
                VIEWPARAM *vw, float diameter) {
  short red, green, blue;
  double pos[2];
  char type[8];

  if (changeColour) {
    /* add code */ /* this is GL specific!! */
    my_getmcolor((Colorindex)tu->color_index, &red, &green, &blue);
    fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f %f setrgbcolor\n",
            (float)red / 255, (float)green / 255, (float)blue / 255);
    changeColour = FALSE;
  }
  fprintf(clp.savefp[SAVE_POSTSCRIPT], "newpath\n");

  transform_point(tu->position, pos);

  if ((dr->render_mode == RM_WIREFRAME) || (dr->render_mode == RM_FAST))
    strcpy(type, "stroke");
  else
    strcpy(type, "fill");

  fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f %f 0 360 arc %s\n", pos[0],
          pos[1], transform_width(tu, diameter) / 2, type);
}

/********************************************************************/
/* Function: psCircleB2D                                             */
/* Assume wireframe for PostScript                                  */
/* Z coordinate is just dropped                                     */
/********************************************************************/

void psCircleB2D(const TURTLE *tu, const DRAWPARAM *dr,
                 const VIEWPARAM *vw, float diameter,
                 float width) {
  short red, green, blue;
  double pos[2];
  char type[8];

  if (changeColour) {
    /* add code */ /* this is GL specific!! */
    my_getmcolor((Colorindex)tu->color_index, &red, &green, &blue);
    fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f %f setrgbcolor\n",
            (float)red / 255, (float)green / 255, (float)blue / 255);
    changeColour = FALSE;
  }
  fprintf(clp.savefp[SAVE_POSTSCRIPT], "newpath\n");
  width = transform_width(tu, (double)tu->line_width);
  fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f setlinewidth\n", width);
  transform_point(tu->position, pos);

  if ((dr->render_mode == RM_WIREFRAME) || (dr->render_mode == RM_FAST))
    strcpy(type, "stroke");
  else
    strcpy(type, "fill");

  fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f %f 0 360 arc stroke\n", pos[0],
          pos[1], transform_width(tu, diameter) / 2);
}

/********************************************************************/
/* Function: psCircle3D                                             */
/* Same as 2D in this case                                          */
/* Assume wireframe for PostScript                                  */
/* Z coordinate is just dropped                                     */
/********************************************************************/

void psCircleB3D(const TURTLE *tu, const DRAWPARAM *dr,
                 const VIEWPARAM *vw, float diameter,
                 float width) {
  short red, green, blue;
  double pos[2];
  char type[8];

  if (changeColour) {
    /* add code */ /* this is GL specific!! */
    my_getmcolor((Colorindex)tu->color_index, &red, &green, &blue);
    fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f %f setrgbcolor\n",
            (float)red / 255, (float)green / 255, (float)blue / 255);
    changeColour = FALSE;
  }
  fprintf(clp.savefp[SAVE_POSTSCRIPT], "newpath\n");
  width = transform_width(tu, (double)tu->line_width);
  fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f setlinewidth\n", width);

  transform_point(tu->position, pos);

  strcpy(type, "stroke");
  fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f %f 0 360 arc %s\n", pos[0],
          pos[1], transform_width(tu, diameter) / 2, type);
}

/********************************************************************/
/* Function: psCircle3D                                             */
/* Same as 2D in this case                                          */
/* Assume wireframe for PostScript                                  */
/* Z coordinate is just dropped                                     */
/********************************************************************/

void psCircle3D(const TURTLE *tu, const DRAWPARAM *dr,
                VIEWPARAM *vw, float diameter) {
  short red, green, blue;
  double pos[2];
  char type[8];

  if (changeColour) {
    /* add code */ /* this is GL specific!! */
    my_getmcolor((Colorindex)tu->color_index, &red, &green, &blue);
    fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f %f setrgbcolor\n",
            (float)red / 255, (float)green / 255, (float)blue / 255);
    changeColour = FALSE;
  }
  fprintf(clp.savefp[SAVE_POSTSCRIPT], "newpath\n");

  transform_point(tu->position, pos);

  if ((dr->render_mode == RM_WIREFRAME) || (dr->render_mode == RM_FAST))
    strcpy(type, "stroke");
  else
    strcpy(type, "fill");

  fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f %f 0 360 arc %s\n", pos[0],
          pos[1], transform_width(tu, diameter) / 2, type);
}

/********************************************************************/
/* Function: psSphere                                               */
/* Spheres are represented by filled circles                        */
/* Z coordinate is just dropped                                     */
/********************************************************************/

void psSphere(const TURTLE *tu, const DRAWPARAM *dr,
              VIEWPARAM *vw, float diameter) {
  short red, green, blue;
  double pos[2];
  int v, t, i;
  GLdouble *old_mat;
  GLdouble surface_mat[16];
  GLfloat rotate[4][4];
  float cos_theta, nlightdir[3];
  double pt[3];
  char type[8];
  SPHERE *sphere;

  if (changeColour) {
    /* add code */ /* this is GL specific!! */
    my_getmcolor((Colorindex)tu->color_index, &red, &green, &blue);
    fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f %f setrgbcolor\n",
            (float)red / 255, (float)green / 255, (float)blue / 255);
    changeColour = FALSE;
  }
  fprintf(clp.savefp[SAVE_POSTSCRIPT], "newpath\n");

  if ((dr->render_mode == RM_WIREFRAME) || (dr->render_mode == RM_FAST))
    strcpy(type, "stroke");
  else
    strcpy(type, "fill");

  switch (dr->render_mode) {
  case RM_FAST:
  case RM_FILLED:
  case RM_INTERPOLATED:
    transform_point(tu->position, pos);

    fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f %f 0 360 arc %s\n", pos[0],
            pos[1], transform_width(tu, diameter) / 2, type);
    break;
  case RM_FLAT:
  case RM_SHADED:
  case RM_WIREFRAME:

    /* store current model matrix */
    old_mat = model_mat;
    model_mat = surface_mat;

    /* Because the orientation vectors from the interpretation
    ** are unit vectors their elements can be used directly as
    ** a transformation matrix	*/
    glPushMatrix();
    glLoadMatrixd(old_mat);
    glTranslatef(tu->position[eX], tu->position[eY], tu->position[eZ]);
    for (v = 0; v < 3; v++) {
      rotate[0][v] = tu->left[v];
      rotate[1][v] = tu->heading[v];
      rotate[2][v] = -tu->up[v];
      rotate[3][v] = 0.0;
      rotate[v][3] = 0.0;
    }
    rotate[3][3] = 1.0;
    glMultMatrixf(&rotate[0][0]);
    glGetDoublev(GL_MODELVIEW_MATRIX, model_mat);
    glPopMatrix();

    /* Transform the light direction to the cylinder
    coordinate system. */
    VecMatMult(dr->light_dir, rotate, nlightdir);

    sphere = FindSphere(tu->cylinder_sides);

    for (t = 0; t < sphere->noSphereTris; t++) {
      if (dr->ourlighting) {
        cos_theta = DotProduct(sphere->sphere[t][0], nlightdir);
        my_getmcolor(
            (Colorindex)(tu->color_index + (float)dr->diff_coef * cos_theta),
            &red, &green, &blue);
        fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f %f setrgbcolor\n",
                (float)red / 255, (float)green / 255, (float)blue / 255);
      }

      fprintf(clp.savefp[SAVE_POSTSCRIPT], "newpath\n");
      for (i = 0; i < 3; i++)
        pt[i] = sphere->sphere[t][0][i] * diameter / 2;
      transform_point(pt, pos);

      if (custom_isnan(pos[0]) || custom_isnan(pos[1]))
        // we don't draw anything
        continue;

      fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f moveto\n", pos[0], pos[1]);

      for (v = 1; v < 3; v++) {
        for (i = 0; i < 3; i++)
          pt[i] = sphere->sphere[t][v][i] * diameter / 2;
        transform_point(pt, pos);

        if (custom_isnan(pos[0]) || custom_isnan(pos[1]))
          // we don't draw anything
          continue;

        fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f lineto\n", pos[0], pos[1]);
      }
      fprintf(clp.savefp[SAVE_POSTSCRIPT], "closepath %s\n", type);
    }

    model_mat = old_mat;

    break;
  }
}

/********************************************************************/
/* Function: psBLackBox                                             */
/********************************************************************/

void psBlackBox(const TURTLE *tu,
                const DRAWPARAM *dr,
                const VIEWPARAM *vw,
                const StringModule *module,
                const StringModule *submodule) {}

/********************************************************************/
/* Function: psLabel                          JH1                   */
/********************************************************************/

void psLabel(const TURTLE *tu, DRAWPARAM *dr,
             const VIEWPARAM *vw, const char *label,
             int parameters, const float values[]) {
  short red, green, blue;
  double pos[2];
  char *str;

  str = MakeLabel(label, parameters, values);

  transform_point(tu->position, pos);
  fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f moveto\n", (float)pos[0],
          (float)pos[1]);

  my_getmcolor((Colorindex)tu->color_index, &red, &green, &blue);
  fprintf(clp.savefp[SAVE_POSTSCRIPT], "%f %f %f setrgbcolor\n",
          (float)red / 255, (float)green / 255, (float)blue / 255);

  fprintf(clp.savefp[SAVE_POSTSCRIPT], "(%s)show\n", str);
}

/********************************************************************/
/* Function: psPredefinedSurface                                    */
/********************************************************************/

void psPredefinedSurface(TURTLE *tu, DRAWPARAM *dr, VIEWPARAM *vw, char id,
                         double sX, double sY, double sZ) {
  GLdouble *old_mat;
  GLdouble surface_mat[16];
  GLfloat rotate[4][4];
  int vertex;

  /* store current model matrix */
  old_mat = model_mat;
  model_mat = surface_mat;

  /* Because the orientation vectors from the interpretation
  ** are unit vectors their elements can be used directly as
  ** a transformation matrix	*/
  glPushMatrix();
  glLoadMatrixd(old_mat);
  glTranslatef(tu->position[eX], tu->position[eY], tu->position[eZ]);
  for (vertex = 0; vertex < 3; vertex++) {
    rotate[0][vertex] = tu->left[vertex];
    rotate[1][vertex] = tu->heading[vertex];
    rotate[2][vertex] = tu->up[vertex];
    rotate[3][vertex] = 0.0;
    rotate[vertex][3] = 0.0;
  }
  rotate[3][3] = 1.0;
  glMultMatrixf(&rotate[0][0]);
  glScaled(sX, sY, sZ);
  glGetDoublev(GL_MODELVIEW_MATRIX, model_mat);
  glPopMatrix();

  draw_surface_tmesh(tu, id, dr, vw, StartTmeshT, TmeshVertexT, EndTmeshT);

  model_mat = old_mat;
}

/********************************************************************/
/* Function: psLdefinedSurface                                      */
/* No action                                                        */
/********************************************************************/

void psLdefinedSurface(StringModule *module,
                       TURTLE *tu,
                       DRAWPARAM *dr,
                       VIEWPARAM *vw) {}

/********************************************************************/
/* Function: psFinishUp                                             */
/* close output file                                                */
/********************************************************************/

void psFinishUp(TURTLE *tu,
                DRAWPARAM *dr,
                VIEWPARAM *vw) {
  fprintf(clp.savefp[SAVE_POSTSCRIPT], "showpage\n");
  if (clp.savefp[SAVE_POSTSCRIPT] != stdin)
    fclose(clp.savefp[SAVE_POSTSCRIPT]);
  clp.savefp[SAVE_POSTSCRIPT] = NULL;
}

/********************************************************************/
/* Puts appropriate routines in the dispatch table and makes other  */
/* settings depending on drawing and viewing parameters, such as    */
/* the drawing parameter shade mode.                                */
/********************************************************************/
turtleDrawDispatcher *psSetDispatcher(DRAWPARAM *dr,
                                      VIEWPARAM *vw) {
  switch (dr->line_style) {
  case LS_POLYGON:
    postscriptDrawRoutines.StartNode = psStartCylOrFlatNode;
    postscriptDrawRoutines.EndNode = psEndFlatNode;
    break;
  case LS_PIXEL:
    postscriptDrawRoutines.StartNode = psStartNode;
    postscriptDrawRoutines.EndNode = psEndNode;
    break;
  case LS_CYLINDER:
    postscriptDrawRoutines.StartNode = psStartCylOrFlatNode;
    postscriptDrawRoutines.EndNode = psEndCylNode;
    break;
  }

  MakeSphere(dr->cylinder_sides);

  iGlMakeUnitSegment(dr);

  return (&postscriptDrawRoutines);
}
