/********************************************************************/
/*                           irisGL.c               		        */
/*                                                                  */
/* irisGL provides interpret routines for rendering with IRIS GL    */
/* Routines are dispatched via the turtleDrawDispatcher             */
/*                                                                  */
/********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define GL_SILENCE_DEPRECATION
#ifdef WIN32
#include "warningset.h"
#endif

#ifndef NOGRAPHICS

#include "platform.h"
#include "control.h"
#include "interpret.h"
#include "utility.h"
#include "irisGL.h"
#include "sphere.h"
#include "generate.h"
#include "patch.h"
#include "textures.h"
#include "indices.h"

#include "test_malloc.h"

GLUquadricObj *pQuad = NULL;
GLUtesselator *tess = NULL;

#ifndef CALLBACK
#define CALLBACK
#endif

extern void MyExit(int status);

/************************ local prototypes **************************/
void iGlStartTmesh(void);
void iGlTmeshVertex(const float *point, const DRAWPARAM *dr);
void iGlEndTmesh(void);
void DrawExpired();

static CYLINDER *MakeCylinder(int sides);

/* The structure for dispatching rendering routines */
static turtleDrawDispatcher irisGLDrawRoutines = {
    iGlSetup,        iGlStartNode,         iGlEndNode,         iGlStartBranch,
    iGlEndBranch,    iGlStartPolygon,      iGlEndPolygon,      iGlSetColour,
    iGlSetLineWidth, iGlCircle2D,          iGlCircle3D,        iGlSphere,
    iGlBlackBox,     iGlPredefinedSurface, iGlLdefinedSurface, iGlLabel,
    iGlFinishUp,     iGlTriangle,          iGLStartTexture,    iGLEndTexture,
    iGlStartTmesh,   iGlTmeshVertex,       iGlEndTmesh,        iGlCircleB2D,
    iGlCircleB3D}; /* added iGlLabel - JH1 */

/* flag indicating that nodes should be joined */
static int connectNode;

/* flag indicating that colour should be changed */
static int changeColour;

/* flag indicating that linewidth should be changed */
static int changeLineWidth;

/* unit rectangle for drawing */
float unitRectTopCoord[2][3];
float unitRectBaseCoord[2][3];
float unitRectNormal[3];

CYLINDER *cylinders = NULL;

SPHERE *spheres = NULL;

/* type of quadric normals */
static int glu_normals;

/* view normal for the current rotation of the object */
double viewNormal[3];

/* variables to hold start node information */
static TURTLE startNode;
static float segmentLength;

/* view parameters */
extern DRAWPARAM drawparam;

/* if any transparent material appears, enable blending */
int doBlend = 0; /* so far ignored */

extern int expired;

/********************************************************************/
/*
  structure for light sources
  */
light_parms gl_lights[NUMLIGHTS];

int gl_numlights;

/* MATERIALS and COLORMAPS */
static int first_free_of_materials = 1;
static int first_free_of_colormap = 1; /* first time it cannot be freed, what*/
                                       /* if initially, there are no NULLs */

typedef colormap_item_type colormap_type[MAXINCOLORMAP];

colormap_type *my_colormaps[MAXCOLORMAPS];

/* local materials */
typedef material_type *set_of_materials_type[MAXINCOLORMAP];

static set_of_materials_type *my_materials[MAXCOLORMAPS];

/* default material */
static material_type default_material;
/* default colormap item  */
static colormap_item_type default_colormap_item = {200, 200, 200, 255};

/************************************************************************/
/*
  reads .map file. Index is guaranteed to be within the correct range
*/
// [Pascal] keep opening the file until the size is stable
static FILE *testOpenFile(char *fname) {

#ifndef WIN32
  int counter = 0;
  int current_size = -1;
  int size = 0;
  // get size of file
  FILE *control_fp = fopen(fname, "r");
  // MIK - THis is a big hack. Need to add a counter to avoid infinite loop if
  // view file is missing
  while ((control_fp == NULL) && (counter < 10)) {
    control_fp = fopen(fname, "rb");
    counter++;
  }
  if (counter == 10)
    fprintf(stderr, "WARNING (irisGL.c): Can't open the view file %s - using defaults.\n",
            fname);
  else {
    fseek(control_fp, 0, SEEK_END); // seek to end of file
    size = ftell(control_fp);       // get current file pointer

    while ((size == 0) || (current_size != size)) {
      current_size = size;
      fclose(control_fp);

      control_fp = fopen(fname, "r");
      while (control_fp == NULL) {
        control_fp = fopen(fname, "r");
        counter++;
      }
      fseek(control_fp, 0, SEEK_END); // seek to end of file
      size = ftell(control_fp);       // get current file pointer
    }
  }
  fseek(control_fp, 0L, SEEK_SET);
#else
  FILE *control_fp = fopen(fname, "rb");
#endif

  return control_fp;
}

int load_in_colormaps(void) {
  FILE *fp;
  int i, index;
  colormap_type *my_colormap;
  FreeColormapSpace();

  for (index = 0; index < MAXCOLORMAPS; index++)
    if (clp.colormapname[index] != NULL) {

      if ((my_colormaps[index] =
               (colormap_type *)Malloc(sizeof(colormap_type))) == NULL) {
        Message("Not enough memory for colormap %d, ignored!\n", index);
        return 0;
      }
      my_colormap = my_colormaps[index];

      if ((fp = testOpenFile(clp.colormapname[index])) == NULL) {
        Message("Cannot open colormap file %s, default values used.\n",
                clp.colormapname[index]);

        (*my_colormap)[0][0] = (*my_colormap)[0][1] = (*my_colormap)[0][2] =
            (*my_colormap)[0][3] = 255;
        /* white background */
        return 0;

        /* ramp blac-white */
        for (i = 1; i < 256; i++) {
          (*my_colormap)[i][0] = (unsigned char)i;
          (*my_colormap)[i][1] = (unsigned char)i;
          (*my_colormap)[i][2] = (unsigned char)i;
          (*my_colormap)[i][3] = (unsigned char)255;
        }
      } else {
        for (i = 0; i < 256; i++) {
          fread((*my_colormap)[i], 3, 1, fp);
          (*my_colormap)[i][3] = 255;
        }
        fclose(fp);
      }

      if (clp.verbose) {
        Message("Colormap %d:\n", index);

        for (i = 0; i < MAXINCOLORMAP; i++)
          Message(" color %u: red %u, green %u, blue %u\n, alpha %u", i,
                  (*my_colormap)[i][0], (*my_colormap)[i][1],
                  (*my_colormap)[i][2], (*my_colormap)[i][3]);
      }
    }
  return 1;
}

/************************************************************************/
void FreeColormapSpace(void) {
  int i;

  if (first_free_of_colormap) {
    first_free_of_colormap = 0;
    for (i = 0; i < MAXCOLORMAPS; i++)
      my_colormaps[i] = NULL;
  } else {
    for (i = 0; i < MAXCOLORMAPS; i++) {
      if (my_colormaps[i] != NULL) {
        Free(my_colormaps[i]);
        my_colormaps[i] = NULL;
      }
    }
  }
}

/************************************************************************/
void FreeMaterialSpace(void) {
  int i, j;

  if (first_free_of_materials) {
    first_free_of_materials = 0;
    for (i = 0; i < MAXCOLORMAPS; i++)
      my_materials[i] = NULL;
  } else {
    for (i = 0; i < MAXCOLORMAPS; i++)
      if (my_materials[i] != NULL) {
        for (j = 0; j < MAXINCOLORMAP; j++) {
          if ((*my_materials[i])[j] != NULL) {
            Free((*my_materials[i])[j]);
            (*my_materials[i])[j] = NULL;
          }
        }
        Free(my_materials[i]);
        my_materials[i] = NULL;
      }
  }
}

/************************************************************************/
/*
  reads materials file (output of Mark's material editor - medit)
*/
int load_in_materials(void) {
  FILE *fp;
  int i, index;
  unsigned char c;
#ifndef WIN32
  int counter = 0;
  int current_size = -1;
  int size = 0;
#endif

  struct material_load_type {
    /* 14 bytes per material as in Mark's file */
    unsigned char transparency;
    unsigned char ambient[3];
    unsigned char diffuse[3];
    unsigned char emissive[3];
    unsigned char specular[3];
    unsigned char shininess;
  } mat;
  float t;
  set_of_materials_type *my_material;
  int nbLines = 0;
  FreeMaterialSpace();

  /* default material */
  for (c = 0; c < 3; c++)
    default_material.ambient[c] = 0.0;
  default_material.ambient[3] = 1.0;
  for (c = 0; c < 4; c++)
    default_material.diffuse[c] = 1.0;
  for (c = 0; c < 3; c++)
    default_material.emissive[c] = 0;
  default_material.emissive[3] = 1.0;
  for (c = 0; c < 3; c++)
    default_material.specular[c] = 0;
  default_material.specular[3] = 1.0;
  default_material.shininess = 0;

  for (index = 0; index < MAXCOLORMAPS; index++)
    if (clp.materialname[index] != NULL) {

      if ((my_materials[index] = (set_of_materials_type *)Malloc(
               sizeof(set_of_materials_type))) == NULL) {
        Message("Not enough memory for material set %d, ignored!\n", index);
        return 0;
      }
      my_material = my_materials[index];

      for (i = 0; i < MAXINCOLORMAP;)
        (*my_material)[i++] = NULL;

        //////////////////////////////////////////////
        // wait until files are stabilized

// [PASCAL]
// open the file twice to check if the size is consistent
#ifndef WIN32
      // get size of file
      fp = fopen(clp.materialname[index], "r");
      // MIK - THis is a big hack. Need to add a counter to avoid infinite loop
      // if view file is missing
      while ((fp == NULL) && (counter < 1000)) {
        fp = fopen(clp.materialname[index], "r");
        counter++;
      }
      if (counter == 1000)
        fprintf(stderr,
                "WARNING (irisGL.c 2): Can't open the view file %s - using defaults.\n",
                clp.materialname[index]);
      else {
        fseek(fp, 0, SEEK_END); // seek to end of file
        size = ftell(fp);       // get current file pointer
        fclose(fp);

        while (current_size != size) {
          current_size = size;
          fp = fopen(clp.materialname[index], "r");
          while (fp == NULL) {
            fp = fopen(clp.materialname[index], "r");
            counter++;
          }
          fseek(fp, 0, SEEK_END); // seek to end of file
          size = ftell(fp);       // get current file pointer
          fclose(fp);
        }
      }
#endif

      /////////////////////////////////////////
      if ((fp = fopen(clp.materialname[index], "rb")) == NULL) {
        Message("Cannot open material file %s, default values used.\n",
                clp.materialname[index]);
        return 0;
      } else {

        while (!feof(fp)) {
          c = (unsigned char)fgetc(fp); /* index of a material */

          if (fread(&mat, sizeof(struct material_load_type), 1, fp) != 1) {
            break;
          }
          nbLines++;

          if (((*my_material)[c] = (material_type *)Malloc(
                   sizeof(struct material_type))) != NULL) {
            t = 1.0f - ((float)mat.transparency / 255.0f);
            doBlend = t < 1.0;

            for (i = 0; i < 3; i++)
              (*my_material)[c]->ambient[i] = (float)mat.ambient[i] / 255.0f;
            (*my_material)[c]->ambient[3] = t;
            for (i = 0; i < 3; i++)
              (*my_material)[c]->diffuse[i] = (float)mat.diffuse[i] / 255.0f;
            (*my_material)[c]->diffuse[3] = t;
            for (i = 0; i < 3; i++)
              (*my_material)[c]->emissive[i] = (float)mat.emissive[i] / 255.0f;
            (*my_material)[c]->emissive[3] = t;
            for (i = 0; i < 3; i++)
              (*my_material)[c]->specular[i] = (float)mat.specular[i] / 255.0f;
            (*my_material)[c]->specular[3] = t;

            (*my_material)[c]->shininess = (float)mat.shininess;
          }
        }
        fclose(fp);
      }

      if (clp.verbose) {
        Message("Material set %d:\n", index);

        for (i = 0; i < MAXINCOLORMAP; i++)
          if ((*my_material)[i] != NULL) {
            Message(
                "Material %03u: ambient (%f,%f,%f,%f)\n", i,
                (*my_material)[i]->ambient[0], (*my_material)[i]->ambient[1],
                (*my_material)[i]->ambient[2], (*my_material)[i]->ambient[3]);
            Message(
                "              diffuse (%f,%f,%f,%f)\n",
                (*my_material)[i]->diffuse[0], (*my_material)[i]->diffuse[1],
                (*my_material)[i]->diffuse[2], (*my_material)[i]->diffuse[3]);
            Message(
                "              emissive (%f,%f,%f,%f)\n",
                (*my_material)[i]->emissive[0], (*my_material)[i]->emissive[1],
                (*my_material)[i]->emissive[2], (*my_material)[i]->emissive[3]);
            Message(
                "              specular (%f,%f,%f,%f)\n",
                (*my_material)[i]->specular[0], (*my_material)[i]->specular[1],
                (*my_material)[i]->specular[2], (*my_material)[i]->specular[3]);
            Message("              shininess %f\n",
                    (*my_material)[i]->shininess);
          }
      }
    }

  return 1;
}

/************************************************************************/
int is_material(Colorindex colindex) {
  int index;

  index = (colindex) / MAXINCOLORMAP;

  if ((index < 0) || (index >= MAXCOLORMAPS))
    return 0;

  if (my_materials[index] == NULL)
    return 0;

  if ((*my_materials[index])[colindex % MAXINCOLORMAP] == NULL)
    return 0;

  return 1;
}

/************************************************************************/
/* if the desired material was not specified, returns pointer to
   the default one
*/
void my_getmaterial(Colorindex colindex, material_type **mat) {
  int index;

  *mat = &default_material;

  index = (colindex) / MAXINCOLORMAP;

  if ((index < 0) || (index >= MAXCOLORMAPS))
    return;

  if (my_materials[index] == NULL)
    return;

  if ((*my_materials[index])[colindex % MAXINCOLORMAP] == NULL)
    return;

  *mat = (*my_materials[index])[colindex % MAXINCOLORMAP];
}

/************************************************************************/
/* if the desired colormap item was not specified, returns pointer to
   the default one
*/
void my_getcolormapitem(Colorindex colindex, colormap_item_type **item) {
  int index;

  *item = &default_colormap_item;

  index = (colindex) / MAXINCOLORMAP;

  if ((index < 0) || (index >= MAXCOLORMAPS))
    return;

  if (my_colormaps[index] == NULL)
    return;

  *item = &(*my_colormaps[index])[colindex % MAXINCOLORMAP];
}

/********************************************************************/
/*
 Initalizes the light structure
 */
void read_light(char *input_line) {
  char *token;
  int index = gl_numlights;

  if (index >= NUMLIGHTS) {
    Message("Too many lights!\n");
    return;
  }

  /* defaults */
  gl_lights[index].ambient[0] = gl_lights[index].ambient[1] =
      gl_lights[index].ambient[2] = gl_lights[index].ambient[3] = 1.0;

  gl_lights[index].diffuse[0] = gl_lights[index].diffuse[1] =
      gl_lights[index].diffuse[2] = gl_lights[index].diffuse[3] = 1.0;

  gl_lights[index].specular[0] = gl_lights[index].specular[1] =
      gl_lights[index].specular[2] = gl_lights[index].specular[3] = 1.0;

  gl_lights[index].position[0] = gl_lights[index].position[1] = 0.0;
  gl_lights[index].position[2] = gl_lights[index].position[3] = 1.0;

  gl_lights[index].spot_direction[0] = gl_lights[index].spot_direction[1] = 0.0;
  gl_lights[index].spot_direction[2] = -1.0;
  gl_lights[index].spot_exponent = 0.0;
  gl_lights[index].spot_cutoff = 180.0;

  gl_lights[index].constant_attenuation = 1.0;
  gl_lights[index].linear_attenuation = 0.0;
  gl_lights[index].quadratic_attenuation = 0.0;

  token = strtok(input_line, " \t:");
  for (;;) {
    if (token == NULL)
      break;

    switch (token[0]) {
    case 'A': /* ambient light */
      gl_lights[index].ambient[0] = (float)get_double();
      gl_lights[index].ambient[1] = (float)get_double();
      gl_lights[index].ambient[2] = (float)get_double();
      gl_lights[index].ambient[3] = (float)1.0;
      break;

    case 'D': /* diffuse light */
      gl_lights[index].diffuse[0] = (float)get_double();
      gl_lights[index].diffuse[1] = (float)get_double();
      gl_lights[index].diffuse[2] = (float)get_double();
      gl_lights[index].diffuse[3] = (float)1.0;
      break;

    case 'S': /* specular light */
      gl_lights[index].specular[0] = (float)get_double();
      gl_lights[index].specular[1] = (float)get_double();
      gl_lights[index].specular[2] = (float)get_double();
      gl_lights[index].specular[3] = (float)1.0;
      break;

    case 'O': /* point light */
      gl_lights[index].position[0] = (float)get_double();
      gl_lights[index].position[1] = (float)get_double();
      gl_lights[index].position[2] = (float)get_double();
      gl_lights[index].position[3] = (float)1.0;
      break;

    case 'V': /* directional light */
      gl_lights[index].position[0] = (float)get_double();
      gl_lights[index].position[1] = (float)get_double();
      gl_lights[index].position[2] = (float)get_double();
      gl_lights[index].position[3] = (float)0.0;
      break;

    case 'P': /* spot light direction, exponent and cutoff */
      gl_lights[index].spot_direction[0] = (float)get_double();
      gl_lights[index].spot_direction[1] = (float)get_double();
      gl_lights[index].spot_direction[2] = (float)get_double();
      gl_lights[index].spot_exponent = (float)get_double();
      gl_lights[index].spot_cutoff = (float)get_double();
      break;

    case 'T': /* attenuation */
      gl_lights[index].constant_attenuation = (float)get_double();
      gl_lights[index].linear_attenuation = (float)get_double();
      gl_lights[index].quadratic_attenuation = (float)get_double();
      break;

    default:
      Message("Light: unknown command '%c'.\n", token[0]);
      return;
    }
    token = strtok(NULL, " \t:\n");
  }
  /* parsing OK */

  if (++gl_numlights == 1) {
    int i;

    /* set drawparam.light_dir to the diretion towards the first light */
    for (i = 0; i < 3; i++)
      drawparam.light_dir[i] = gl_lights[0].position[i];

    Normalize(drawparam.light_dir);
  }
}

/********************************************************************/
void FreeGL(void) {
  CYLINDER *ptr, *ptr2;
  SPHERE *ptr3, *ptr4;

  ptr = cylinders;

  while (ptr != NULL) {
    Free(ptr->top);
    ptr->top = NULL;
    Free(ptr->base);
    ptr->base = NULL;

    ptr2 = ptr;
    ptr = ptr->next;

    Free(ptr2);
    ptr2 = NULL;
  }

  ptr3 = spheres;

  while (ptr3 != NULL) {
    Free(ptr3->sphere);
    ptr3->sphere = NULL;

    ptr4 = ptr3;
    ptr3 = ptr3->next;

    Free(ptr4);
    ptr4 = NULL;
  }
}

/********************************************************************/
light_parms *get_light(int index) {
  if ((index < 0) || (index >= gl_numlights))
    return NULL;

  return &gl_lights[index];
}

/********************************************************************/
void set_gl_lights(void) {
  int i;
  int li[NUMLIGHTS] = {GL_LIGHT0, GL_LIGHT1, GL_LIGHT2, GL_LIGHT3,
                       GL_LIGHT4, GL_LIGHT5, GL_LIGHT6, GL_LIGHT7};
  char buff[256];

  for (i = 0; i < NUMLIGHTS; i++)
    glDisable(li[i]);

  if (gl_numlights == 0) {

    sprintf(buff, " V:%f,%f,%f", drawparam.light_dir[0], drawparam.light_dir[1],
            drawparam.light_dir[2]);

    read_light(buff);
  }

  for (i = 0; i < gl_numlights; i++) {
    /* this works if GL_LIGHTi == li[i] */
    glEnable(li[i]);

    glLightfv(li[i], GL_AMBIENT, gl_lights[i].ambient);
    glLightfv(li[i], GL_DIFFUSE, gl_lights[i].diffuse);
    glLightfv(li[i], GL_SPECULAR, gl_lights[i].specular);

    glLightfv(li[i], GL_POSITION, gl_lights[i].position);

    glLightfv(li[i], GL_SPOT_DIRECTION, gl_lights[i].spot_direction);
    glLightf(li[i], GL_SPOT_EXPONENT, gl_lights[i].spot_exponent);
    glLightf(li[i], GL_SPOT_CUTOFF, gl_lights[i].spot_cutoff);

    glLightf(li[i], GL_CONSTANT_ATTENUATION, gl_lights[i].constant_attenuation);
    glLightf(li[i], GL_LINEAR_ATTENUATION, gl_lights[i].linear_attenuation);
    glLightf(li[i], GL_QUADRATIC_ATTENUATION,
             gl_lights[i].quadratic_attenuation);
  }
}

/********************************************************************/
/* draws a wireframe of a circle (with spikes)
 */

static void circf(GLfloat x, GLfloat y, GLdouble rad, const TURTLE *tu) {
  if (rad == 0)
    return;
  glFrontFace(GL_CCW);
  gluQuadricDrawStyle(pQuad, GLU_FILL);
  gluQuadricNormals(pQuad, glu_normals);
  glPushMatrix();
  glTranslatef(x, y, 0);
  gluDisk(pQuad, (GLdouble)0.0, rad, tu->cylinder_sides, 1);
  glPopMatrix();
  glFrontFace(GL_CW);
}
static void circbf(GLfloat x, GLfloat y, GLdouble rad, float width,
                   const TURTLE *tu) {
  if (rad == 0)
    return;
  glFrontFace(GL_CCW);
  gluQuadricDrawStyle(pQuad, GLU_FILL);
  gluQuadricNormals(pQuad, glu_normals);
  glPushMatrix();
  glTranslatef(x, y, 0);
  gluDisk(pQuad, (GLdouble)(rad - width / 2), (GLdouble)(rad + width / 2),
          tu->cylinder_sides, 1);
  glPopMatrix();
  glFrontFace(GL_CW);
}

/********************************************************************/
/* draws a circle
 */

static void circ(GLfloat x, GLfloat y, GLdouble rad, const TURTLE *tu) {
  if (rad == 0)
    return;
  gluQuadricDrawStyle(pQuad, GLU_SILHOUETTE);
  gluQuadricNormals(pQuad, glu_normals);
  glPushMatrix();
  glTranslatef(x, y, 0);
  gluDisk(pQuad, (GLdouble)0.0, rad, tu->cylinder_sides, 1);
  glPopMatrix();
}
static void circb(GLfloat x, GLfloat y, GLdouble rad, float width,
                  const TURTLE *tu) {
  if (rad == 0)
    return;
  gluQuadricDrawStyle(pQuad, GLU_SILHOUETTE);
  gluQuadricNormals(pQuad, glu_normals);
  glPushMatrix();
  glTranslatef(x, y, 0);
  gluDisk(pQuad, (GLdouble)(rad - width / 2), (GLdouble)(rad + width / 2),
          tu->cylinder_sides, 1);
  glPopMatrix();
}

/********************************************************************/
/* Function: iGlSetup                                                */
/********************************************************************/

int iGlSetup(TURTLE *tu, DRAWPARAM *dr, VIEWPARAM *vw) {
  Matrix rotation;
  float volumeCentre[3];
  int i;

  /* Initialize drawing parameters. */

  /* handle rotations */
  for (i = 0; i <= 2; i++) {
    volumeCentre[i] = (vw->min[i] + vw->max[i]) / 2.0f;
  }
  /* determine transformation */

  if (NULL == pQuad)
    pQuad = gluNewQuadric();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(volumeCentre[0], volumeCentre[1], volumeCentre[2]);
  glTranslatef(-vw->xPan, -vw->yPan, 0.0f);
  glRotated(0.1 * vw->zRotation, 0.0, 0.0, 1.0);
  glRotated(0.1 * vw->xRotation, 1.0, 0.0, 0.0);
  glRotated(0.1 * vw->yRotation, 0.0, 1.0, 0.0);

  glTranslatef(-volumeCentre[0], -volumeCentre[1], -volumeCentre[2]);
  glGetFloatv(GL_MODELVIEW_MATRIX, &rotation[0][0]);

  /* apply the rotation to the view normal */
  /* note the column?? major order organization of the matrix */
  for (i = 0; i < 3; i++) {
    viewNormal[i] = vw->view_normal[0] * rotation[i][0] +
                    vw->view_normal[1] * rotation[i][1] +
                    vw->view_normal[2] * rotation[i][2];
  }

  set_gl_lights();

  /* set line width */
  if (dr->render_mode == RM_WIREFRAME)
    glLineWidth((vw->max[0] - vw->min[0]) / clp.xsize / 10);
  else
    glLineWidth(tu->line_width);

    /* set depthcueing if desired */
#ifdef DEPTHCUE
  if (getdcm() == TRUE) {
    lshaderange((Colorindex)(tu->color_index - dr->cue_range),
                (Colorindex)(tu->color_index + dr->cue_range), 0, 0x7FFFFF);
  } else
#endif
    my_color(tu->color_index, tu->color_index_back);

  glRotatef(vw->objectRotation[0], 1.0, 0.0, 0.0);
  glRotatef(vw->objectRotation[1], 0.0, 1.0, 0.0);
  glRotatef(vw->objectRotation[2], 0.0, 0.0, 1.0);

  glScalef(vw->objectScale[0], vw->objectScale[1], vw->objectScale[2]);

  /* no segment created yet */
  connectNode = FALSE;

  /* colour and linewidth don't need changing */
  changeColour = FALSE;
  changeLineWidth = FALSE;

  return 0;
}

/********************************************************************/
/* Function: iGlStartNode                                           */
/********************************************************************/
char set_gllighting_back;

void iGlStartNode(TURTLE *tu, DRAWPARAM *dr,
                  __attribute__((unused)) VIEWPARAM *vw,
                  __attribute__((unused)) float length,
                  __attribute__((unused)) char symbol) {
  if (dr->gllighting) {
    /* switch off gl lighting  */
    glDisable(GL_LIGHTING);
    set_gllighting_back = 1;
    dr->gllighting = 0;

    my_color(tu->color_index, tu->color_index_back);
  } else
    set_gllighting_back = 0;

  if (dr->line_style == LS_PIXEL) {
    glLineWidth(tu->line_width);
    changeLineWidth = TRUE;
  }
  //  printf("Begin Lines\n");

  glBegin(GL_LINES);

  glVertex3dv(tu->position);

  /* this node should be connected to the next */
  connectNode = TRUE;
}

/********************************************************************/
/* Function: iGlEndNode                                             */
/* Complete the segment; change colour and linewidth if necessary   */
/********************************************************************/

void iGlEndNode(TURTLE *tu, DRAWPARAM *dr, VIEWPARAM *vw,
                __attribute__((unused)) char symbol) {
  if (connectNode) {
    if (dr->render_mode == RM_INTERPOLATED)
      if (changeColour) {
        my_color(tu->color_index, tu->color_index_back);
        changeColour = FALSE;
      }

    glVertex3dv(tu->position);
    glEnd();
    //    printf("gl End\n");

    if (set_gllighting_back) {
      /* switch back on gl lighting */
      dr->gllighting = 1;
      glEnable(GL_LIGHTING);
    }

    connectNode = FALSE;

    /* colour and line width may need changing */
    if (changeColour) {
#ifdef DEPTH_CUEING
      if (dr->shade_mode > SIMPLE_FILL && dr->cue_range != 0) {
        depthcue(TRUE);
        lshaderange((Colorindex)(tu->color_index - dr->cue_range),
                    (Colorindex)(tu->color_index + dr->cue_range), 0, 0x7FFFFF);
      } else
#endif
        my_color(tu->color_index, tu->color_index_back);

      changeColour = FALSE;
    }
    if (changeLineWidth) {
      if (dr->render_mode != RM_WIREFRAME)
        glLineWidth(tu->line_width);
      else if (dr->line_style == LS_PIXEL)
        glLineWidth((vw->max[0] - vw->min[0]) / clp.xsize / 10);

      changeLineWidth = FALSE;
    }
  }
}

/********************************************************************/
/* Function: iGlStartSurfaceNode                                    */
/* Handles drawing of segments specified as a surface: Obsolete??   */
/********************************************************************/

void iGlStartSurfaceNode(__attribute__((unused)) TURTLE *tu,
                         __attribute__((unused)) DRAWPARAM *dr,
                         __attribute__((unused)) VIEWPARAM *vw, float length,
                         __attribute__((unused)) char symbol) {
#ifdef DEPTH_CUEING
  if (dr->shade_mode > SIMPLE_FILL)
    depthcue(FALSE);
#endif

  if (length != 0.0) {
    /* add code */ /* should this disappear ??*/
    /* scaleing isn't handled right for lines in iGl routines */
    /* draw_surface(tu, dr, vw, dr->line_type, length);*/
  }

#ifdef DEPTH_CUEING
  if (dr->shade_mode > SIMPLE_FILL && dr->cue_range != 0) {
    depthcue(TRUE);
    lshaderange((Colorindex)(tu->color_index - dr->cue_range),
                (Colorindex)(tu->color_index + dr->cue_range), 0, 0x7FFFFF);
  }
#endif
}

/********************************************************************/
/* Function: iGlEndFlatNode                                         */
/* Complete the segment; change colour and linewidth if necessary   */
/********************************************************************/

void iGlEndFlatNode(TURTLE *tu, DRAWPARAM *dr,
                    __attribute__((unused)) VIEWPARAM *vw,
                    __attribute__((unused)) char symbol) {
  static Matrix rotate = {{0.0, 0.0, 0.0, 0.0},
                          {0.0, 0.0, 0.0, 0.0},
                          {0.0, 0.0, 0.0, 0.0},
                          {0.0, 0.0, 0.0, 1.0}};

  int coord;
  float topScale, cos_theta, nlightdir[3];
  double colinearTest;
  double epsilon = .000001;
  double tempUpVector[3];
  double tempLeftVector[3];

  extern double viewNormal[3];
  extern TURTLE startNode;
  extern float segmentLength;

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

    /* Perform set up and transformations for surface  */
#ifdef DEPTH_CUEING
    if (dr->shade_mode > SIMPLE_FILL)
      depthcue(FALSE);
#endif
    glPushMatrix();
    glTranslated(startNode.position[eX], startNode.position[eY],
                 startNode.position[eZ]);

    /* Because the orientation vectors from the calculation
    ** are orthogonal unit vectors their elements can be used
    ** directly as a transformation matrix	*/
    for (coord = 0; coord < 3; coord++) {
      rotate[0][coord] = (float)tempLeftVector[coord];
      rotate[1][coord] = (float)startNode.heading[coord];
      rotate[2][coord] = -(float)tempUpVector[coord];
    }
    glMultMatrixf(&rotate[0][0]);

    /* normal must be defined before glScale */
    if (dr->gllighting)
      glNormal3fv(unitRectNormal);
    else if (dr->ourlighting) {
      /* Transform the light direction to the flat node
              coordinate system. */
      VecMatMult(dr->light_dir, rotate, nlightdir);

      cos_theta = DotProduct(unitRectNormal, nlightdir);

      my_color(startNode.color_index +
                   (Colorindex)((float)dr->diff_coef * cos_theta),
               startNode.color_index_back +
                   (Colorindex)((float)dr->diff_coef * cos_theta));
    }

    glScalef((float)startNode.line_width, (float)segmentLength,
             (float)startNode.line_width);

    glFrontFace(GL_CCW);
    /* Draw the cylinder */
    if (dr->tapered_lines_on)
      /* modify top points to give the correct radius */
      /* after scaling by the startNode linewidth */
      topScale = tu->line_width / startNode.line_width;
    else
      topScale = 1.0;

    if (dr->render_mode == RM_WIREFRAME)
      glBegin(GL_POLYGON);
    else
      glBegin(GL_TRIANGLE_FAN);
    glVertex3fv(unitRectBaseCoord[0]);
    glVertex3fv(unitRectBaseCoord[1]);
    unitRectTopCoord[1][eX] = unitRectBaseCoord[1][eX] * topScale;
    unitRectTopCoord[1][eZ] = unitRectBaseCoord[1][eZ] * topScale;

    if (dr->render_mode == RM_INTERPOLATED) {
      my_color(tu->color_index, tu->color_index_back);
      changeColour = FALSE;
    }

    glVertex3fv(unitRectTopCoord[1]);
    unitRectTopCoord[0][eX] = unitRectBaseCoord[0][eX] * topScale;
    unitRectTopCoord[0][eZ] = unitRectBaseCoord[0][eZ] * topScale;
    glVertex3fv(unitRectTopCoord[0]);
    glEnd();
    glFrontFace(GL_CW);

    glPopMatrix();

    connectNode = FALSE;

    /* colour and line width may need changing */
    if (changeColour) {
#ifdef DEPTH_CUEING
      if (dr->shade_mode > SIMPLE_FILL && dr->cue_range != 0) {
        depthcue(TRUE);
        lshaderange((Colorindex)(tu->color_index - dr->cue_range),
                    (Colorindex)(tu->color_index + dr->cue_range), 0, 0x7FFFFF);
      } else
#endif
        my_color(tu->color_index, tu->color_index_back);

      changeColour = FALSE;
    }
    if (changeLineWidth) {
      if (dr->render_mode != RM_WIREFRAME)
        glLineWidth(tu->line_width);
      changeLineWidth = FALSE;
    }
  }
}

/********************************************************************/
/* Function: iGlStartCylOrFlatNode                                  */
/* Saves turtle state for use when the segment is ended             */
/********************************************************************/

void iGlStartCylOrFlatNode(TURTLE *tu, __attribute__((unused)) DRAWPARAM *dr,
                           __attribute__((unused)) VIEWPARAM *vw, float length,
                           __attribute__((unused)) char symbol) {
  TurtleCopy(&startNode, tu);
  segmentLength = length;

  /* this node should be connected to the next */
  connectNode = TRUE;
}

/********************************************************************/
/* Function: iGlEndCylNode                                          */
/* Draw a truncated cone representing the segment;                  */
/* change colour and linewidth if necessary                         */
/********************************************************************/

void iGlEndCylNode(TURTLE *tu, DRAWPARAM *dr,
                   __attribute__((unused)) VIEWPARAM *vw,
                   __attribute__((unused)) char symbol) {
  static Matrix rotate = {{0.0, 0.0, 0.0, 0.0},
                          {0.0, 0.0, 0.0, 0.0},
                          {0.0, 0.0, 0.0, 0.0},
                          {0.0, 0.0, 0.0, 1.0}};

  int vertex;
  int is_tex;
  float topScale, cos_theta, nlightdir[3];
  float width_diff, linelen;
  float normal[3]; /* for rotated normal */
  float *normalptr;
  float normal_xz = 0.0;
  char rot_normal = 0;
  CYLINDER *cylinder;

  extern TURTLE startNode;
  extern float segmentLength;

  if ((connectNode) && (segmentLength != 0.0)) {

    /* Perform set up and transformations for surface  */
#ifdef DEPTH_CUEING
    if (dr->shade_mode > SIMPLE_FILL)
      depthcue(FALSE);
#endif
    /* Because the orientation vectors from the interpretation
    ** are unit vectors their elements can be used directly as
    ** a transformation matrix	*/
    glPushMatrix();
    glTranslated(startNode.position[eX], startNode.position[eY],
                 startNode.position[eZ]);
    for (vertex = 0; vertex < 3; vertex++) {
      rotate[0][vertex] = (float)startNode.left[vertex];
      rotate[1][vertex] = (float)startNode.heading[vertex];
      rotate[2][vertex] = (float)startNode.up[vertex];
    }
    glMultMatrixf(&rotate[0][0]);

    is_tex = is_valid_texture_index(tu->texture);
    if (is_tex)
      iGLStartTexture(tu->texture);

    glScalef((float)startNode.line_width, (float)segmentLength,
             (float)startNode.line_width);

    if (dr->ourlighting) {
      /* Transform the light direction to the cylinder
              coordinate system. */
      VecMatMult(dr->light_dir, rotate, nlightdir);
    } else
      my_color(startNode.color_index, startNode.color_index_back);

    /* Draw the cylinder */
    if (dr->tapered_lines_on)
      /* modify top points to give the correct radius */
      /* after scaling by the startNode linewidth */
      topScale = tu->line_width / startNode.line_width;
    else
      topScale = 1.0;

    if ((dr->gllighting) || (dr->ourlighting)) {
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
        normal[eY] = width_diff / linelen;
        /* this is y coordinates of all normals */

        /* N_XZ/normal_len = segmentLength/line_len */
        normal_xz = segmentLength / linelen;

        /* in case of gllighting, /startNode.line_width and
        /segmentLength  corrects the effect of scaling */
        if (dr->gllighting) {
          normal_xz /= startNode.line_width;
          normal[eY] /= segmentLength;
        }

        /* x and z coordinates of all normals should be multiplied
        by normal_xz */
        rot_normal = 1;
      } else
        /* use precomputed cylinder normal cylinder->base[i][NORMAL_X] */
        rot_normal = 0;
    }
    /* find the appropriate cylinder */
    cylinder = FindCylinder(tu->cylinder_sides);

    glFrontFace(GL_CCW);
    glBegin(GL_TRIANGLE_STRIP);

    for (vertex = 0; vertex <= cylinder->sides; vertex++) {
      if (rot_normal) {
        normal[eX] = normal_xz * cylinder->base[vertex][NORMAL_X];
        normal[eZ] = normal_xz * cylinder->base[vertex][NORMAL_Z];
        normalptr = normal;
      } else
        normalptr = &cylinder->base[vertex][NORMAL_X];

      if (dr->ourlighting) {
        cos_theta = DotProduct(normalptr, nlightdir);

        my_color(startNode.color_index +
                     (Colorindex)((float)dr->diff_coef * cos_theta),
                 startNode.color_index_back +
                     (Colorindex)((float)dr->diff_coef * cos_theta));
      } else if (dr->gllighting)
        glNormal3fv(normalptr);

      if (is_tex) {
        cylinder->base[vertex][TEXTURE_T] = startNode.tex_t;
        glTexCoord2fv(&cylinder->base[vertex][TEXTURE_S]);
      }

      if (dr->render_mode == RM_INTERPOLATED)
        my_color(startNode.color_index, startNode.color_index_back);
      glVertex3fv(&cylinder->base[vertex][POINT_X]);

      cylinder->top[vertex][POINT_X] =
          cylinder->base[vertex][POINT_X] * topScale;
      cylinder->top[vertex][POINT_Z] =
          cylinder->base[vertex][POINT_Z] * topScale;

      if (is_tex) {
        cylinder->base[vertex][TEXTURE_T] = tu->tex_t;
        glTexCoord2fv(&cylinder->base[vertex][TEXTURE_S]);
      }

      if (dr->render_mode == RM_INTERPOLATED)
        my_color(tu->color_index, tu->color_index_back);

      glVertex3fv(&cylinder->top[vertex][POINT_X]);
    }

    glEnd();
    glFrontFace(GL_CW);

    if (is_tex)
      iGLEndTexture(tu->texture);
    glPopMatrix();
  }

  /* colour and line width may need changing */
  if (changeColour) {
#ifdef DEPTH_CUEING
    if (dr->shade_mode > SIMPLE_FILL && dr->cue_range != 0) {
      depthcue(TRUE);
      lshaderange((Colorindex)(tu->color_index - dr->cue_range),
                  (Colorindex)(tu->color_index + dr->cue_range), 0, 0x7FFFFF);
    } else
#endif
      my_color(tu->color_index, tu->color_index_back);

    changeColour = FALSE;
  }
  if (changeLineWidth) {
    if (dr->render_mode != RM_WIREFRAME)
      glLineWidth(tu->line_width);
    changeLineWidth = FALSE;
  }

  connectNode = FALSE;
}

/********************************************************************/
/* Function: iGlStartBranch                                         */
/* No action                                                        */
/********************************************************************/

void iGlStartBranch(__attribute__((unused)) TURTLE *tu,
                    __attribute__((unused)) DRAWPARAM *dr,
                    __attribute__((unused)) VIEWPARAM *vw) {}

/********************************************************************/
/* Function: iGlEndBranch                                           */
/* Reset hardware to current turtle state                           */
/********************************************************************/

void iGlEndBranch(TURTLE *tu, DRAWPARAM *dr,
                  __attribute__((unused)) VIEWPARAM *vw) {
#ifdef DEPTH_CUEING
  if (dr->shade_mode > SIMPLE_FILL && dr->cue_range != 0) {
    depthcue(TRUE);
    lshaderange((Colorindex)(tu->color_index - dr->cue_range),
                (Colorindex)(tu->color_index + dr->cue_range), 0, 0x7FFFFF);
  } else
#endif
    my_color(tu->color_index, tu->color_index_back);

  if (dr->render_mode != RM_WIREFRAME)
    glLineWidth(tu->line_width);
}

/********************************************************************/
/* Function: PolygonToMesh                                          */
/* Outputs polygon as a set of triangles.                           */
/* NOTE: dr->texture has to be already set to 1, if the polygon is  */
/*       textured !!                                                */
/********************************************************************/

void PolygonToMesh(POLYGON *polygon, DRAWPARAM *dr) {
  float ptr0[PITEM];
  float ptr[PITEM], ptr2[PITEM];
  float *p, *prev_p, *tmp_p;
  int i;
  /* for texture */
  double t_axis[3], s_axis[3];
  double min_s, min_t, del_s, del_t;

  if (dr->texture) {
    SetPolygonTextureParams(polygon, s_axis, t_axis, &min_s, &min_t, &del_s,
                            &del_t);
    dr->tdd->StartTexture(polygon->vertex[0].texture);
  }

  dr->tdd->StartTmesh();
  /* although mesh is not directly used, it's necessary for inventor groups */

  ptr0[POINT_X] = (float)polygon->vertex[0].position[0];
  ptr0[POINT_Y] = (float)polygon->vertex[0].position[1];
  ptr0[POINT_Z] = (float)polygon->vertex[0].position[2];

  /* normal - for all points the same */
  ptr0[NORMAL_X] = ptr[NORMAL_X] = ptr2[NORMAL_X] =
      (float)polygon->vertex[0].up[0];
  ptr0[NORMAL_Y] = ptr[NORMAL_Y] = ptr2[NORMAL_Y] =
      (float)polygon->vertex[0].up[1];
  ptr0[NORMAL_Z] = ptr[NORMAL_Z] = ptr2[NORMAL_Z] =
      (float)polygon->vertex[0].up[2];

  ptr0[COLOR_FRONT] = ptr[COLOR_FRONT] = ptr2[COLOR_FRONT] =
      (float)polygon->vertex[0].color_index;
  ptr0[COLOR_BACK] = ptr[COLOR_BACK] = ptr2[COLOR_BACK] =
      (float)polygon->vertex[0].color_index_back;

  if (dr->texture) {
    /* texture */
    ptr0[TEXTURE_S] =
        (float)((DDotProduct(polygon->vertex[0].position, s_axis) - min_s) *
                del_s);
    ptr0[TEXTURE_T] =
        (float)((DDotProduct(polygon->vertex[0].position, t_axis) - min_t) *
                del_t);
  }

  p = ptr;
  prev_p = ptr2;

  prev_p[POINT_X] = (float)polygon->vertex[1].position[0];
  prev_p[POINT_Y] = (float)polygon->vertex[1].position[1];
  prev_p[POINT_Z] = (float)polygon->vertex[1].position[2];

  if (dr->texture) {
    /* its texture */
    prev_p[TEXTURE_S] =
        (float)((DDotProduct(polygon->vertex[1].position, s_axis) - min_s) *
                del_s);
    prev_p[TEXTURE_T] =
        (float)((DDotProduct(polygon->vertex[1].position, t_axis) - min_t) *
                del_t);
  }

  for (i = 2; i < polygon->edge_count; i++) {
    /* third point */
    p[POINT_X] = (float)polygon->vertex[i].position[0];
    p[POINT_Y] = (float)polygon->vertex[i].position[1];
    p[POINT_Z] = (float)polygon->vertex[i].position[2];

    if (dr->texture) {
      /* its texture */
      p[TEXTURE_S] =
          (float)((DDotProduct(polygon->vertex[i].position, s_axis) - min_s) *
                  del_s);
      p[TEXTURE_T] =
          (float)((DDotProduct(polygon->vertex[i].position, t_axis) - min_t) *
                  del_t);
    }

    /* draw a triangle */
    dr->tdd->RenderTriangle(ptr0, prev_p, p, dr);

    /* p becomes prev_p */
    tmp_p = p;
    p = prev_p;
    prev_p = tmp_p;
  }

  dr->tdd->EndTmesh();

  if (dr->texture)
    dr->tdd->EndTexture(polygon->vertex[0].texture);
}

/*
 *  Callback functions for tesselation in CPFG.
 */
void CALLBACK tessError(GLenum errorCode) {
  const GLubyte *errorString;
  errorString = gluErrorString(errorCode);
  fprintf(stderr, "[ERROR]: %s\n", errorString);
  exit(0);
}
void CALLBACK tessCombine(GLdouble coords[3], const GLdouble *vertexData[4],
                          const GLfloat weight[4], GLdouble **outData) {
  int i, j;
  GLdouble *newVertex = (GLdouble *)malloc(sizeof(GLdouble) * 6);

  newVertex[0] = coords[0];
  newVertex[1] = coords[1];
  newVertex[2] = coords[2];

  for (i = 3; i < 6; i++) {
    newVertex[i] = 0.0;
    for (j = 0; j < 4; j++) {
      // gluTesselator sets vertexData pointer to NULL if weight != 0.0
      if (weight[j] > 0.0) {
        newVertex[i] += weight[j] * vertexData[j][i];
      }
    }
  }

  *outData = newVertex;
}

/********************************************************************/
/* Function: iGlStartPolygon                                         */
/* No action                                                        */
/********************************************************************/

void iGlStartPolygon(__attribute__((unused)) POLYGON *polygon,
                     __attribute__((unused)) TURTLE *tu,
                     __attribute__((unused)) DRAWPARAM *dr, VIEWPARAM *vw) {
  // Only make a tesselator if we actually need it (hence why this is not in
  //   a constructor), don't make more than one.
  if (vw->concavePolygons && !tess) {
    tess = gluNewTess();

    gluTessCallback(tess, GLU_TESS_BEGIN, (void(CALLBACK *)())glBegin);
    gluTessCallback(tess, GLU_TESS_END, (void(CALLBACK *)())glEnd);

    gluTessCallback(tess, GLU_TESS_ERROR, (void(CALLBACK *)())tessError);

    gluTessCallback(tess, GLU_TESS_VERTEX, (void(CALLBACK *)())glVertex3dv);
    gluTessCallback(tess, GLU_TESS_COMBINE, (void(CALLBACK *)())tessCombine);
  }
}

/********************************************************************/
/* Function: iGlEndPolygon                                          */
/********************************************************************/

void iGlEndPolygon(POLYGON *polygon, __attribute__((unused)) TURTLE *tu,
                   DRAWPARAM *dr, VIEWPARAM *vw) {
  GLdouble coords[3];
  float cos_theta;
  int i;
  double t_axis[3], s_axis[3];
  double min_s, min_t, del_s, del_t;
  float tex[2];
  int useTess = vw->concavePolygons;

  dr->texture = (char)is_valid_texture_index(polygon->vertex[0].texture);

  if (dr->texture) {
    SetPolygonTextureParams(polygon, s_axis, t_axis, &min_s, &min_t, &del_s,
                            &del_t);
    dr->tdd->StartTexture(polygon->vertex[0].texture);
  }

  if (!useTess) {
    if (dr->render_mode == RM_WIREFRAME)
      glBegin(GL_POLYGON);
    else
      glBegin(GL_POLYGON);
  } else {
    gluTessBeginPolygon(tess, NULL);
    gluTessBeginContour(tess);
  }

  if (dr->gllighting) {
    /* turtles up vector when the '{' is hit */
    my_color((polygon->vertex[0]).color_index,
             (polygon->vertex[0]).color_index_back);
    if (!useTess)
      glNormal3dv(polygon->vertex[0].up);
    else
      gluTessNormal(tess, polygon->vertex[0].up[0], polygon->vertex[0].up[1],
                    polygon->vertex[0].up[2]);
  } else if (dr->ourlighting) {
    if (dr->render_mode == RM_FLAT) {
      cos_theta = (float)DFDotProduct((polygon->vertex[0]).up, dr->light_dir);
      my_color((polygon->vertex[0]).color_index +
                   (Colorindex)((float)dr->diff_coef * cos_theta),
               (polygon->vertex[0]).color_index_back +
                   (Colorindex)((float)dr->diff_coef * cos_theta));
    }
  } else
    my_color((polygon->vertex[0]).color_index,
             (polygon->vertex[0]).color_index_back);

  for (i = 0; i < polygon->edge_count; i++) {
    if (dr->ourlighting &&
        (dr->render_mode == RM_SHADED || dr->render_mode == RM_SHADOWS)) {
      cos_theta = (float)DFDotProduct((polygon->vertex[i]).up, dr->light_dir);
      my_color((polygon->vertex[i]).color_index +
                   (Colorindex)((float)dr->diff_coef * cos_theta),
               (polygon->vertex[i]).color_index_back +
                   (Colorindex)((float)dr->diff_coef * cos_theta));
    } else if (dr->render_mode == RM_INTERPOLATED)
      my_color((polygon->vertex[i]).color_index,
               (polygon->vertex[i]).color_index_back);

    if (dr->texture) {
      tex[0] =
          (float)((DDotProduct(polygon->vertex[i].position, s_axis) - min_s) *
                  del_s);
      tex[1] =
          (float)((DDotProduct(polygon->vertex[i].position, t_axis) - min_t) *
                  del_t);
      glTexCoord2fv(tex);
    }
    if (!useTess) {
      glVertex3dv((polygon->vertex[i]).position);
    } else {
      coords[0] = (polygon->vertex[i]).position[0];
      coords[1] = (polygon->vertex[i]).position[1];
      coords[2] = (polygon->vertex[i]).position[2];

      gluTessVertex(tess, coords, (polygon->vertex[i]).position);
    }
  }

  if (!useTess) {
    glEnd();
  } else {
    gluTessEndContour(tess);
    gluTessEndPolygon(tess);
  }

  if (dr->texture)
    dr->tdd->EndTexture(polygon->vertex[0].texture);
}

/********************************************************************/
/* Function: iGlPolyBspline                                         */
/* polygon routine for B_SPLINE and CLOSED_B_SPLINE modes           */
/* - The polygon is drawn as a wireframe with B-spline              */
/*   interpolation using the vertices as control points.            */
/*   The polygon is closed if necessary.                            */
/********************************************************************/

void iGlPolyBspline(POLYGON *polygon, __attribute__((unused)) TURTLE *tu,
                    DRAWPARAM *dr, __attribute__((unused)) VIEWPARAM *vw) {
  int i;
  Coord geom[4][3];

  if (polygon->edge_count < 4) {
    Message("Not enough nodes to interpolate\n");
    MyExit(1);
  }
  /* Initialize the geometry matrix */
  for (i = 0; i < 3; i++)
    NewGeom(geom, (polygon->vertex[i]).position);
  /* Draw subsequent elements of the spline */
  for (i = 3; i < polygon->edge_count; i++) {
    NewGeom(geom, (polygon->vertex[i]).position);
    crv(geom);
  }
  if (dr->shade_mode == CLOSED_B_SPLINE) {
    /* If the contour should be closed, complete it */
    for (i = 0; i < 3; i++) {
      NewGeom(geom, (polygon->vertex[i]).position);
      crv(geom);
    }
  }
}

void NewGeom(Coord geom[4][3], double position[]) {
  int i, j, k;

  for (i = 0; i < 4; i++) {
    k = i + 1;
    for (j = 0; j < 3; j++)
      geom[i][j] = geom[k][j];
  }

  for (j = 0; j < 3; j++)
    geom[3][j] = (Coord)position[j];
}

/********************************************************************/
/* Function: iGlPolyTwosided                                        */
/* polygon routine for TWO_SIDED mode                               */
/* - The colours from the turtle state at each vertex are           */
/*   Gouraud interpolated. The color at each vertex is calculated   */
/*   taking the turtle "up" and the light direction into account.   */
/*   A distinction is made between the front side and the back      */
/*   side of a surface (the color of the back side is decreased by  */
/*   256, so that it goes to a different map).                      */
/* add code */ /* is this useful???? */
/********************************************************************/

void iGlPolyTwosided(POLYGON *polygon, __attribute__((unused)) TURTLE *tu,
                     DRAWPARAM *dr, VIEWPARAM *vw) {
  float cos_theta, cos2_theta;
  int i;

  if (dr->render_mode == RM_WIREFRAME)
    glBegin(GL_POLYGON);
  else
    glBegin(GL_TRIANGLE_FAN);

  for (i = 0; i < polygon->edge_count; i++) {
    cos_theta = DFDotProduct((polygon->vertex[i]).up, dr->light_dir);
    cos2_theta = DFDotProduct((polygon->vertex[i]).up, vw->viewpoint);
    /* this works OK only if vrp == (0,0,0) since, in general,
    the line of sight is affected by vrp */
    my_color((polygon->vertex[i]).color_index +
                 (Colorindex)((float)dr->diff_coef * cos_theta) -
                 ((cos2_theta >= 0) ? 0 : 256),
             (polygon->vertex[i]).color_index_back +
                 (Colorindex)((float)dr->diff_coef * cos_theta) -
                 ((cos2_theta >= 0) ? 0 : 256));
    glVertex3dv((polygon->vertex[i]).position);
  }
  glEnd();
}

/********************************************************************/
/* Function: iGlTriangle                                            */
/********************************************************************/
/* if the render mode is RM_WIREFRAME, the DRAW item of a vertex
   specifies whether the edge originated from it is drawn */
void iGlTriangle(const float *p1, const float *p2, const float *p3,
                 const DRAWPARAM *dr) {
  if (dr->render_mode == RM_WIREFRAME) {
    glBegin(GL_LINES);

    my_color((GLint)p1[COLOR_FRONT], (GLint)p1[COLOR_BACK]);

    if (dr->gllighting)
      glNormal3fv(p1 + NORMAL_X);

    if (p1[DRAW_LINE] == 1) {
      glVertex3fv(p1 + POINT_X);
      glVertex3fv(p2 + POINT_X);
    }

    if (p2[DRAW_LINE] == 1) {
      glVertex3fv(p2 + POINT_X);
      glVertex3fv(p3 + POINT_X);
    }

    if (p3[DRAW_LINE] == 1) {
      glVertex3fv(p3 + POINT_X);
      glVertex3fv(p1 + POINT_X);
    }

    glEnd();
    return;
  }

  glBegin(GL_TRIANGLE_FAN);

  my_color((GLint)p1[COLOR_FRONT], (GLint)p1[COLOR_BACK]);
  if (dr->gllighting)
    glNormal3fv(p1 + NORMAL_X);

  if (dr->texture)
    glTexCoord2fv(p1 + TEXTURE_S);

  glVertex3fv(p1 + POINT_X);

  if (dr->vertexbound) {
    my_color((GLint)p2[COLOR_FRONT], (GLint)p2[COLOR_BACK]);

    if (dr->gllighting)
      glNormal3fv(p2 + NORMAL_X);
  }

  if (dr->texture)
    glTexCoord2fv(p2 + TEXTURE_S);

  glVertex3fv(p2 + POINT_X);

  if (dr->vertexbound) {
    my_color((GLint)p3[COLOR_FRONT], (GLint)p3[COLOR_BACK]);

    if (dr->gllighting)
      glNormal3fv(p3 + NORMAL_X);
  }

  if (dr->texture)
    glTexCoord2fv(p3 + TEXTURE_S);

  glVertex3fv(p3 + POINT_X);

  glEnd();
}

/********************************************************************/
/* Function: iGlSetColour                                           */
/* Set current colour unless in the middle of a segment             */
/* In that case, set flag for change at end node                    */
/********************************************************************/

#ifndef DEPTHCUE
void iGlSetColour(const TURTLE *tu, __attribute__((unused)) const DRAWPARAM *dr,
                  __attribute__((unused)) const VIEWPARAM *vw) {
#else
void iGlSetColour(const TURTLE *tu, const DRAWPARAM *dr,
                  __attribute__((unused)) const VIEWPARAM *vw) {
#endif
  /* check that we're not inside a node */
  if (!connectNode) {
#ifdef DEPTHCUE
    if (dr->shade_mode > SIMPLE_FILL && dr->cue_range != 0) {
      depthcue(TRUE);
      lshaderange((Colorindex)(tu->color_index - dr->cue_range),
                  (Colorindex)(tu->color_index + dr->cue_range), 0, 0x7FFFFF);
    } else
#endif
      my_color(tu->color_index, tu->color_index_back);
  } else {
    /* set changeColour flag so that colour can be set after
                a segment is drawn */
    changeColour = TRUE;
  }
}

/********************************************************************/
/* Function: iGlSetLineWidth                                        */
/* Set current line width unless in the middle of a node            */
/* In that case, set flag for change at node end                    */
/********************************************************************/

void iGlSetLineWidth(const TURTLE *tu, const DRAWPARAM *dr,
                     __attribute__((unused)) const VIEWPARAM *vw) {
  if (!connectNode) {
    if (dr->render_mode != RM_WIREFRAME)
      glLineWidth(tu->line_width);
    changeLineWidth = FALSE;
  } else {
    /* set changeLineWidth flag so that linewidth can be set before
                a segment is drawn */
    changeLineWidth = TRUE;
  }
}

/********************************************************************/
/* Function: iGlCircle2D                                            */
/********************************************************************/

void iGlCircle2D(const TURTLE *tu, const DRAWPARAM *dr,
                 __attribute__((unused)) VIEWPARAM *vw, float diameter) {
  switch (dr->render_mode) {
  case RM_FAST:
  case RM_WIREFRAME:
    circ(tu->position[0], tu->position[1], diameter * 0.5f, tu);
    break;

  default:
    circf(tu->position[0], tu->position[1], diameter * 0.5f, tu);
  }
}
void iGlCircleB2D(const TURTLE *tu, const DRAWPARAM *dr,
                  __attribute__((unused)) const VIEWPARAM *vw, float diameter,
                  float width) {
  switch (dr->render_mode) {
  case RM_FAST:
  case RM_WIREFRAME:
    circb(tu->position[0], tu->position[1], diameter * 0.5f, width, tu);
    break;

  default:
    circbf(tu->position[0], tu->position[1], diameter * 0.5f, width, tu);
  }
}
/********************************************************************/
/* Function: iGlCircle3D                                             */
/********************************************************************/

void iGlCircle3D(const TURTLE *tu, const DRAWPARAM *dr,
                 __attribute__((unused)) VIEWPARAM *vw, float diameter) {
  int vertex;
  static Matrix rotate = {{0.0, 0.0, 0.0, 0.0},
                          {0.0, 0.0, 0.0, 0.0},
                          {0.0, 0.0, 0.0, 0.0},
                          {0.0, 0.0, 0.0, 1.0}};

  /* Because the orientation vectors from the interpretation are
  ** unit vectors their elements can be used directly as a
  ** transformation matrix	*/
  glPushMatrix();
  glTranslatef(tu->position[eX], tu->position[eY], tu->position[eZ]);
  for (vertex = 0; vertex < 3; vertex++) {
    rotate[0][vertex] = tu->left[vertex];
    rotate[1][vertex] = tu->heading[vertex];
    rotate[2][vertex] = tu->up[vertex];
  }
  glMultMatrixf(&rotate[0][0]);

  switch (dr->render_mode) {
  case RM_FAST:
  case RM_WIREFRAME:
    circ(0.0, 0.0, diameter * 0.5, tu);
    break;

  default:
    circf(0.0, 0.0, diameter * 0.5, tu);
  }
  glPopMatrix();
}
void iGlCircleB3D(const TURTLE *tu, const DRAWPARAM *dr,
                  __attribute__((unused)) const VIEWPARAM *vw, float diameter,
                  float width) {
  int vertex;
  static Matrix rotate = {{0.0, 0.0, 0.0, 0.0},
                          {0.0, 0.0, 0.0, 0.0},
                          {0.0, 0.0, 0.0, 0.0},
                          {0.0, 0.0, 0.0, 1.0}};

  /* Because the orientation vectors from the interpretation are
  ** unit vectors their elements can be used directly as a
  ** transformation matrix	*/
  glPushMatrix();
  glTranslatef(tu->position[eX], tu->position[eY], tu->position[eZ]);
  for (vertex = 0; vertex < 3; vertex++) {
    rotate[0][vertex] = tu->left[vertex];
    rotate[1][vertex] = tu->heading[vertex];
    rotate[2][vertex] = tu->up[vertex];
  }
  glMultMatrixf(&rotate[0][0]);

  switch (dr->render_mode) {
  case RM_FAST:
  case RM_WIREFRAME:
    circb(0.0, 0.0, diameter * 0.5, width, tu);
    break;

  default:
    circbf(0.0, 0.0, diameter * 0.5, width, tu);
  }
  glPopMatrix();
}

/********************************************************************/
/* Function: iGlSphere                                               */
/********************************************************************/

void iGlSphere(const TURTLE *tu, const DRAWPARAM *dr,
               __attribute__((unused)) VIEWPARAM *vw, float diameter) {
  int t, v;
  float radius, cos_theta, nlightdir[3];

  static Matrix rot = {{0.0, 0.0, 0.0, 0.0},
                       {0.0, 0.0, 0.0, 0.0},
                       {0.0, 0.0, 0.0, 0.0},
                       {0.0, 0.0, 0.0, 1.0}};

  if ((radius = diameter / 2) == 0.0)
    return;

  glPushMatrix();

  /* Because the orientation vectors from the interpretation
  are unit vectors their elements can be used directly as
  a transformation matrix
  *** Normally, a sphere is a sphere, and so rotation does not
make a difference in the final visualization.  But here,
the sphere is polygonized.  So, by re-orienting the sphere,
the shading will better match cylinders it caps.
  */
  glTranslatef(tu->position[eX], tu->position[eY], tu->position[eZ]);

  for (v = 0; v < 3; v++) {
    rot[0][v] = tu->left[v];
    rot[1][v] = tu->heading[v];
    rot[2][v] = -tu->up[v];
  }
  glMultMatrixf(&rot[0][0]);

  switch (dr->render_mode) {
  case RM_WIREFRAME:
  case RM_FAST:
  case RM_FILLED:
  case RM_INTERPOLATED:
    if (clp.ismaterialfile) {
      gluQuadricDrawStyle(pQuad, (dr->render_mode == RM_WIREFRAME) ||
                                         (dr->render_mode == RM_FAST)
                                     ? GLU_SILHOUETTE
                                     : GLU_FILL);
      /* for quadrics it's better to use radius as a parameter, not glScale */

      if (dr->render_mode == RM_FAST)
        gluSphere(pQuad, (GLdouble)radius, 8, 8);
      else
        gluSphere(pQuad, (GLdouble)radius, tu->cylinder_sides,
                  tu->cylinder_sides / 2 + 1);
    } else {
      SPHERE *sphere;
      sphere = FindSphere(tu->cylinder_sides);

      glScalef(radius, radius, radius);

      for (t = 0; t < sphere->noSphereTris; t++) {
        if (dr->render_mode == RM_WIREFRAME)
          glBegin(GL_POLYGON);
        else
          glBegin(GL_TRIANGLE_FAN);

        for (v = 0; v < 3; v++)
          glVertex3fv(sphere->sphere[t][v]);
        glEnd();
      }
    }
    break;

  case RM_FLAT:
  case RM_SHADED:
  case RM_SHADOWS:
    if (clp.ismaterialfile) {
      gluQuadricDrawStyle(pQuad, GLU_FILL);
      gluQuadricNormals(pQuad, glu_normals);

      /* for quadrics it's better to use radius as a parameter, not glScale */
      gluSphere(pQuad, (GLdouble)radius, tu->cylinder_sides,
                tu->cylinder_sides / 2 + 1);
    } else {
      SPHERE *sphere;
      sphere = FindSphere(tu->cylinder_sides);

      glScalef(radius, radius, radius);
      /* Transform the light direction to the cylinder
      coordinate system. */
      VecMatMult(dr->light_dir, rot, nlightdir);

      for (t = 0; t < sphere->noSphereTris; t++) {
        if (dr->render_mode == RM_WIREFRAME)
          glBegin(GL_POLYGON);
        else
          glBegin(GL_TRIANGLE_FAN);

        for (v = 0; v < 3; v++) {
          cos_theta = DotProduct(sphere->sphere[t][v], nlightdir);
          my_color(tu->color_index +
                       (Colorindex)((float)dr->diff_coef * cos_theta),
                   tu->color_index_back +
                       (Colorindex)((float)dr->diff_coef * cos_theta));

          glVertex3fv(sphere->sphere[t][v]);
        }
        glEnd();
      }
    }
  }

  glPopMatrix();
}

/********************************************************************/
/* Function: iGlBLackBox                                             */
/********************************************************************/

void iGlBlackBox(__attribute__((unused)) const TURTLE *tu,
                 __attribute__((unused)) const DRAWPARAM *dr,
                 __attribute__((unused)) const VIEWPARAM *vw,
                 __attribute__((unused)) const StringModule *module,
                 __attribute__((unused)) const StringModule *submodule) {}

/********************************************************************/
/* Function: iGlLabel       JH1                                     */
/********************************************************************/

void iGlLabel(const TURTLE *tu, DRAWPARAM *dr,
              __attribute__((unused)) const VIEWPARAM *vw, const char *label,
              int parameters, const float *values) {
  char *str;

  if (dr->gllighting) {
    /* switch off gl lighting */
    glDisable(GL_LIGHTING);
    dr->gllighting = 0;
    text_color(tu->color_index, tu->color_index_back);
    dr->gllighting = 1;
  }

  glRasterPos3d(tu->position[0], tu->position[1], tu->position[2]);

  str = MakeLabel(label, parameters, values);

  DrawString(str, tu);

  if (dr->gllighting) /* switch on gl lighting */
    glEnable(GL_LIGHTING);
}

/********************************************************************/
/* Function: iGlPredefinedSurface                                   */
/* Passes rendering routine names to surface routines               */
/********************************************************************/

void iGlPredefinedSurface(TURTLE *tu, DRAWPARAM *dr, VIEWPARAM *vw, char id,
                          double sX, double sY, double sZ) {
  static Matrix rotate = {{0.0, 0.0, 0.0, 0.0},
                          {0.0, 0.0, 0.0, 0.0},
                          {0.0, 0.0, 0.0, 0.0},
                          {0.0, 0.0, 0.0, 1.0}};

  int vertex;

  if (sX != 0.0 && sY != 0.0 && sZ != 0.0) {

    /* Perform set up and transformations for surface  */
#ifdef DEPTHCUE
    if (dr->shade_mode > SIMPLE_FILL)
      depthcue(FALSE);
#endif
    /* Because the orientation vectors from the interpretation
    ** are unit vectors their elements can be used directly as
    ** a transformation matrix	*/
    glPushMatrix();
    glTranslatef(tu->position[eX], tu->position[eY], tu->position[eZ]);
    for (vertex = 0; vertex < 3; vertex++) {
      rotate[0][vertex] = tu->left[vertex];
      rotate[1][vertex] = tu->heading[vertex];
      rotate[2][vertex] = tu->up[vertex];
    }
    glMultMatrixf(&rotate[0][0]);
    glScaled(sX, sY, sZ);

    draw_surface_tmesh(tu, id, dr, vw, iGlStartTmesh, iGlTmeshVertex,
                       iGlEndTmesh);

    /* reset the previous matrix*/
    glPopMatrix();

#ifdef DEPTHCUE
    if (dr->shade_mode > SIMPLE_FILL && dr->cue_range != 0) {
      depthcue(TRUE);
      lshaderange((Colorindex)(tu->color_index - dr->cue_range),
                  (Colorindex)(tu->color_index + dr->cue_range), 0, 0x7FFFFF);
    }
#endif
  }
}

/********************************************************************/
/* Function: iGlStartPatches                                        */
/* Sets precision for wireframe patches                             */
/********************************************************************/

void iGlStartPatches(TURTLE *tu, DRAWPARAM *dr,
                     __attribute__((unused)) VIEWPARAM *vw,
                     __attribute__((unused)) int sPrecision,
                     __attribute__((unused)) int tPrecision,
                     __attribute__((unused)) int basisFunctionID) {
  patchbasis(basisFunctionID, basisFunctionID);
  patchprecision(sPrecision, tPrecision);
  patchcurves(sPrecision, tPrecision);
  if (dr->render_mode != RM_WIREFRAME)
    glLineWidth(tu->line_width);
  my_color((Colorindex)tu->color_index, (Colorindex)tu->color_index_back);
}

/********************************************************************/
/* Function: iGlRenderPatch                                         */
/* Renders a wireframe patch                                        */
/********************************************************************/

void iGlRenderPatch(__attribute__((unused)) TURTLE *tu,
                    __attribute__((unused)) DRAWPARAM *dr,
                    __attribute__((unused)) VIEWPARAM *vw, double xPoints[4][4],
                    double yPoints[4][4], double zPoints[4][4]) {
  Message("Patches are not supported %f %f %f\n", xPoints[0][0], yPoints[0][0],
         zPoints[0][0]);
  patch(xPoints, yPoints, zPoints);
}

/********************************************************************/
/* Function: iGlEndPatches                                          */
/* Cleans up and resets graphics state                              */
/********************************************************************/

void iGlEndPatches(TURTLE *tu, DRAWPARAM *dr,
                   __attribute__((unused)) VIEWPARAM *vw) {
  if (dr->render_mode != RM_WIREFRAME)
    glLineWidth(tu->line_width);
}

/********************************************************************/
/* Function: iGlStartTmesh                                          */
/* Initializes tmesh                                                */
/********************************************************************/

void iGlStartTmesh(void) { glBegin(GL_TRIANGLE_STRIP); }

/********************************************************************/
/* Function: iGlTmeshVertex                                         */
/* Sends a colour and vertex down the pipeline                      */
/********************************************************************/

void iGlTmeshVertex(const float *point, const DRAWPARAM *dr) {
  if (dr->vertexbound) {
    if (dr->gllighting) {
      glNormal3fv(point + NORMAL_X);
    } else {
      my_color(point[COLOR_FRONT], point[COLOR_BACK]);
    }
  }

  if (dr->texture)
    glTexCoord2fv(point + TEXTURE_S);

  glVertex3fv(point + POINT_X);
}

/********************************************************************/
/* Function: iGlEndTmesh                                            */
/* ends tmesh                                                       */
/********************************************************************/

void iGlEndTmesh(void) { glEnd(); }

/********************************************************************/
/* Function: iGlLdefinedSurface                                      */
/********************************************************************/

void iGlLdefinedSurface(StringModule *module, TURTLE *tu, DRAWPARAM *dr,
                        VIEWPARAM *vw) {
  dr->texture = 0;

  SurfaceTmeshDraw(module, tu, dr, vw, iGlStartTmesh, iGlTmeshVertex,
                   iGlEndTmesh);
}

/********************************************************************/
/* Function: iGlFinishUp                                             */
/* No action                                                        */
/********************************************************************/

void iGlFinishUp(__attribute__((unused)) TURTLE *tu,
                 __attribute__((unused)) DRAWPARAM *dr,
                 __attribute__((unused)) VIEWPARAM *vw) {
  glFlush();
  if (expired)
    DrawExpired();
  /* if in double buffer mode swap */
  my_swapbuffers();
}

/********************************************************************/
/* Puts appropriate routines in the dispatch table and sets GL      */
/* depending on drawing and viewing parameters, in this case        */
/* drawing parameter shade mode.                                    */
/********************************************************************/

turtleDrawDispatcher *iGlSetDispatcher(DRAWPARAM *dr,
                                       __attribute__((unused)) VIEWPARAM *vw) {
  int shade_model;

  /* set line style function depending on line style as defined
     in view file by "initial line width" */
  switch (dr->line_style) {
  case LS_POLYGON:
    irisGLDrawRoutines.StartNode = iGlStartCylOrFlatNode;
    irisGLDrawRoutines.EndNode = iGlEndFlatNode;
    break;
  case LS_PIXEL:
    irisGLDrawRoutines.StartNode = iGlStartNode;
    irisGLDrawRoutines.EndNode = iGlEndNode;
    break;
  case LS_CYLINDER:
    irisGLDrawRoutines.StartNode = iGlStartCylOrFlatNode;
    irisGLDrawRoutines.EndNode = iGlEndCylNode;
    break;
  }

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
  shade_model = GL_FLAT;

  /* set function pointers particular to a shade mode */
  /* set up GL as appropriate */
  switch (dr->render_mode) {
  case RM_FILLED:
    VERBOSE("render mode: filled\n");
    break;
  case RM_INTERPOLATED:
    shade_model = GL_SMOOTH;
    VERBOSE("render mode: interpolated\n");
    break;
  case RM_FLAT:
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    shade_model = GL_FLAT;
    VERBOSE("render mode: flat\n");
    break;
  case RM_SHADED:
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    shade_model = GL_SMOOTH;
    VERBOSE("render mode: shaded\n");
    break;
  case RM_SHADOWS:
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    shade_model = GL_SMOOTH;
    VERBOSE("render mode: shadows\n");
    break;
#ifdef GL_SPLINES
  case B_SPLINE:
  case CLOSED_B_SPLINE:
    irisGLDrawRoutines.EndPolygon = iGlPolyBspline;

    VERBOSE("shading mode: b-spline\n");
    break;
#endif
  case RM_WIREFRAME:
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    VERBOSE("render mode: wireframe\n");
    break;

  case RM_FAST:
    if (dr->line_style == LS_CYLINDER) {
      irisGLDrawRoutines.StartNode = iGlStartCylOrFlatNode;
      irisGLDrawRoutines.EndNode = iGlEndFlatNode;
    }
    VERBOSE("render mode: fast\n");
    break;
  default:
    Message("WARNING: render mode %d unknown; filled assumed.\n",
            dr->render_mode);
    dr->render_mode = RM_FILLED;

    break;
  }
  glu_normals = GLU_NONE;

  if (clp.ismaterialfile) {
    glu_normals = shade_model == GL_FLAT ? GLU_FLAT : GLU_SMOOTH;
  }
  glShadeModel(shade_model);

  dr->gllighting =
      ((dr->render_mode == RM_FLAT) || (dr->render_mode == RM_SHADED) ||
       (dr->render_mode == RM_SHADOWS)) &&
      (clp.ismaterialfile);
  dr->ourlighting =
      ((dr->render_mode == RM_FLAT) || (dr->render_mode == RM_SHADED) ||
       (dr->render_mode == RM_SHADOWS)) &&
      (!clp.ismaterialfile);
  dr->vertexbound =
      dr->render_mode == RM_SHADED || dr->render_mode == RM_FLAT ||
      dr->render_mode == RM_INTERPOLATED || dr->render_mode == RM_SHADOWS;

  SetGraphics(); /* sets clear color and lighting mode, if necessary */

  /* set up the segment polygons */
  iGlMakeUnitSegment(dr);

  MakeSphere(dr->cylinder_sides);

  return (&irisGLDrawRoutines);
}

/********************************************************************/
CYLINDER *FindCylinder(int sides) {
  CYLINDER *cylinder;
  extern DRAWPARAM drawparam;

  if ((cylinder = cylinders) == NULL)
    Warning("Cylinders not created!\n", INTERNAL_LVL);

  for (;;) {
    if (cylinder->sides == sides)
      break;

    if (cylinder->sides > sides || cylinder->next == NULL) {
      if ((cylinder = MakeCylinder(sides)) == NULL) {
        /* get the closest existing value */
        /* for now, get the default one */
        cylinder = FindCylinder(drawparam.cylinder_sides);
      }

      return cylinder;
    }
    cylinder = cylinder->next;
  }

  return cylinder;
}

/********************************************************************/
static CYLINDER *MakeCylinder(int sides) {
  CYLINDER **cylinder, *ptr;
  double theta, dtheta;
  int vert;
  float tex_s, step_s;

  if (sides < 3)
    return NULL;

  if (cylinders != NULL) {
    /* find a position for the cylinder */
    cylinder = &cylinders;

    while (*cylinder != NULL && (*cylinder)->sides < sides)
      *cylinder = (*cylinder)->next;

    ptr = *cylinder;

    if (ptr != NULL && ptr->sides == sides)
      /* cylinder already exists */
      return ptr;

    if ((*cylinder = Malloc(sizeof(CYLINDER))) == NULL) {
      Message("Cannot allocate memory for a cylinder\n");
      return NULL;
    }

    (*cylinder)->next = ptr;
    ptr = *cylinder; /* to save on referencing */
  } else {
    if ((cylinders = Malloc(sizeof(CYLINDER))) == NULL) {
      Message("Cannot allocate memory for a cylinder\n");
      return NULL;
    }

    ptr = cylinders;
    ptr->next = NULL;
  }

  ptr->sides = sides;

  /* Build the cylinder */
  dtheta = 2.0 * M_PI / sides;
  theta = 0;

  ptr->base = (vertex *)Malloc(sizeof(vertex) * (sides + 1));
  ptr->top = (vertex *)Malloc(sizeof(vertex) * (sides + 1));

  if (ptr->base == NULL || ptr->top == NULL) {
    Message("Cannot allocate memory for a cylinder\n");
    return NULL;
  }

  tex_s = 0;
  step_s = 1.0 / sides;

  for (vert = 0; vert <= sides; vert++) {
    if (vert == sides)
      theta = 0;

    ptr->base[vert][POINT_X] = ptr->top[vert][POINT_X] = cos(theta) * .5;
    ptr->base[vert][POINT_Y] = 0.0;
    ptr->top[vert][POINT_Y] = 1.0;
    ptr->base[vert][POINT_Z] = ptr->top[vert][POINT_Z] = sin(theta) * .5;

    /* normal is the same as in base */
    ptr->base[vert][NORMAL_X] = cos(theta);
    ptr->base[vert][NORMAL_Y] = 0.0;
    ptr->base[vert][NORMAL_Z] = sin(theta);

    /* texture coordinates are the same as in base */
    ptr->base[vert][TEXTURE_S] = tex_s;
    ptr->base[vert][TEXTURE_T] = 0.0;
    tex_s += step_s;

    theta += dtheta;
  }

  return ptr;
}

/********************************************************************/
/*  Make a unit rectangle described by the globals                  */
/*  unitRect[Top|Base][Coords|Normals].                             */
/*	Make a cylinder using the polygonization_level draw         */
/*      parmeters                                                   */
/*  globals unitCyl[Top|Base][Coords|Normals].                      */
/*  Globals are used so that the drawing routines can get at them.  */
/*  The base will stay the same but the top will change radius      */
/*  as appropriate to the relative line width actually forming a    */
/*  truncated triangle or cone .                                    */
/********************************************************************/
void iGlMakeUnitSegment(DRAWPARAM *dr) {
  /* Build the rectangle */
  unitRectTopCoord[0][eX] = -.5;
  unitRectTopCoord[0][eY] = 1.0;
  unitRectTopCoord[0][eZ] = 0.0;
  unitRectTopCoord[1][eX] = .5;
  unitRectTopCoord[1][eY] = 1.0;
  unitRectTopCoord[1][eZ] = 0.0;
  unitRectBaseCoord[0][eX] = -0.5;
  unitRectBaseCoord[0][eY] = 0.0;
  unitRectBaseCoord[0][eZ] = 0.0;
  unitRectBaseCoord[1][eX] = 0.5;
  unitRectBaseCoord[1][eY] = 0.0;
  unitRectBaseCoord[1][eZ] = 0.0;

  unitRectNormal[eX] = 0.0;
  unitRectNormal[eY] = 0.0;
  unitRectNormal[eZ] = -1.0;

  MakeCylinder(dr->cylinder_sides);
}

/********************************************************************/
SPHERE *FindSphere(int sides) {
  SPHERE *sphere;
  extern DRAWPARAM drawparam;

  if ((sphere = spheres) == NULL)
    Warning("Spheres not created!\n", INTERNAL_LVL);

  for (;;) {
    if (sphere->sides == sides)
      break;

    if (sphere->sides > sides || sphere->next == NULL) {
      if ((sphere = MakeSphere(sides)) == NULL) {
        /* get the closest existing value */
        /* for now, get the default one */
        sphere = FindSphere(drawparam.cylinder_sides);
      }

      return sphere;
    }
    sphere = sphere->next;
  }

  return sphere;
}

/********************************************************************/
SPHERE *MakeSphere(int sides) {
  SPHERE **sphere, *ptr;
  int polygonization_level;

  if (sides < 3)
    return NULL;

  if (spheres != NULL) {
    /* find a position for the sphere */
    sphere = &spheres;

    while (*sphere != NULL && (*sphere)->sides < sides)
      *sphere = (*sphere)->next;

    ptr = *sphere;

    if (ptr != NULL && ptr->sides == sides)
      /* sphere already exists */
      return ptr;

    if ((*sphere = Malloc(sizeof(SPHERE))) == NULL) {
      Message("Cannot allocate memory for a sphere\n");
      return NULL;
    }

    (*sphere)->next = ptr;
    ptr = *sphere; /* to save on referencing */
  } else {
    if ((spheres = Malloc(sizeof(SPHERE))) == NULL) {
      Message("Cannot allocate memory for a sphere\n");
      return NULL;
    }

    ptr = spheres;
    ptr->next = NULL;
  }

  ptr->sides = sides;

  polygonization_level = (int)floor(log((double)sides) / log(2.0) - 1);

  if (polygonization_level < 1)
    polygonization_level = 1;

  ptr->noSphereTris =
      (int)floor(0.5 + 8.0 * pow(4.0, (double)(polygonization_level - 1)));
  ptr->sphere = (triangle *)Malloc(sizeof(triangle) * ptr->noSphereTris);
  /* add test whether NULL returned */

  makePolySphere(ptr->sphere, polygonization_level);

  return ptr;
}

/* callback routine to print font names through fmenumerate */
void printFMname(char *str) { Message("  %s\n", str); }

#endif /* NOGRAPHICS */
