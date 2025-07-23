/* Routines for reading a background scene from an input text file and then
   drawing it after each cpfg redraw (called from iGlFinishUp() in irisGL.c).
   Handles properly translucent objects.
   Cannot instantiate.

   Author: Radomir Mech, August 1995
   */

#define AT_ONCE
#ifdef WIN32
#include "warningset.h"
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef WIN32
#include <unistd.h>
#endif

#include "opengl.h"

#include "control.h"
#include "turtleDrawDispatcher.h"
#include "interpret.h"
#include "background.h"
#include "platform.h"
#include "matrix.h"
#include "utility.h"
#include "irisGL.h"
#include "comlineparam.h"

#include "test_malloc.h"

struct syntax_item {
  int flag;
  char *keyword;
};

enum bgItem {
  bPolygon,
  bMesh,
  bPrism,
  bCone,
  bSphere,
  bMaterial,
  bPushmatrix,
  bPopmatrix,
  bTranslate,
  bRotate,
  bScale,
  bMultmatrix,
  bRectangle,
  bPolygonuv,
  bCylinder
};

struct syntax_item syn[] = {
    {bPolygon, "polygon"},
    {bPolygonuv, "polygonuv"},
    {bRectangle, "rectangle"},
    {bMesh, "mesh"},
    {bPrism, "prism"},
    {bPrism, "box"}, /* ala prism */
    {bCone, "cone"},
    {bCylinder, "cylinder"},
    {bSphere, "sphere"},
    {bMaterial, "material"},
    {bPushmatrix, "pushmatrix"},
    {bPopmatrix, "popmatrix"},
    {bTranslate, "translate"},
    {bRotate, "rotate"},
    {bScale, "scale"},
    {bMultmatrix, "multmatrix"},
    {-1, NULL} /* must be the last one */
};

struct primitive_type {
  int flag;
  int num_items;
  unsigned int data; /* index in array of floats */
};
typedef struct primitive_type primitive_type;

primitive_type *primitive = NULL;

int num_primitives;
int primitive_array_size;

#define NUM_PRIMITIVES 4000
#define NUM_FLOATS (NUM_PRIMITIVES * 12)
/* as if there were only prisms with 12 floats per primitive */

float *data = NULL;
int num_floats;
int float_array_size;

/* dealing with colors */
material_type *mat;           /* points to material structure to be used */
colormap_item_type *cmapitem; /* points to colormap item structure to be used*/

char backgroundscene = 0; /* for rayshade - determining whether background
                             scene is being drawn */

/* prototypes */
void DrawPrimitives(int run, DRAWPARAM *dr);
void MyExit(int);

/*************************************************************************/
void FreePrimitives(void) {
  if (primitive != NULL) {
    Free(primitive);
    primitive = NULL;
  }

  num_primitives = 0;
  primitive_array_size = 0;

  if (data != NULL) {
    Free(data);
    data = NULL;
  }
  num_floats = 0;
  float_array_size = 0;
}

/*************************************************************************/
void AllocatePrimitives(void) {
  assert(NULL == primitive);
  if ((primitive = (primitive_type *)Malloc(primitive_array_size *
                                            sizeof(primitive_type))) == NULL) {
    Message("Not enough memory for array of primitives!\n");
    MyExit(0);
  }

  assert(NULL == data);
  if ((data = (float *)Malloc(float_array_size * sizeof(float))) == NULL) {
    Message("Not enough memory for array of floats!\n");
    MyExit(0);
  }
}

/*************************************************************************/
void InitializeBackgroundScene(void) {
  FreePrimitives();
  primitive_array_size = NUM_PRIMITIVES;
  float_array_size = NUM_FLOATS;
  AllocatePrimitives();
}

/*************************************************************************/
#define LINELEN 1024
static char line[LINELEN];

extern char defaultPreprocessor[8];

/*************************************************************************/

int ReadBackgroundSceneFile(const char *nms) {
  FILE *fp;
  char *token;
  struct syntax_item *ptr;
  static char names[LINELEN];

  if (nms == NULL)
    return 0;

  strcpy(names, nms);

  token = strtok(names, " ,;\n");

  /* initialization */
  InitializeBackgroundScene();

  while (token != NULL) {
    /* preprocess the background scene file */
    // by default cpfg uses vlabcpp, but that preprocessor inserts a space between a minus sign '-' and a user-defined macro
    // for example, "#define STEP 35" in "translate -STEP 0 0", produces "translate - 35 0 0"
    // the space between "-" and "35" causes cpfg to misinterpret the translation and print a warning message.
    // Why does vlabcpp, insert a space there?
    // A workaround is to use 'preproc', a different preprocessor for now...
    clp.preprocessor = "preproc"; 
    if ((fp = PreprocessFile(token, "")) == NULL) {
      Message("Cannot open preprocessed data file %s.\n", token);
      return 0;
    }
    clp.preprocessor = defaultPreprocessor;

    VERBOSE("Processing the primitive file.\n");

    /* read in the primitive file */
    while (!feof(fp)) {
      ptr = syn;

      /* get one command */
      if (fscanf(fp, "%s", line) != 1)
        break;

      while (ptr->flag != -1) {
        /* skip lines left by preprocessor */
        if (line[0] == '#') {
          fgets(line, LINELEN, fp);
          break;
        }

        if (strcmp(line, ptr->keyword) == 0) {
          if (num_primitives >= primitive_array_size) {
            /* realloc */
            primitive_array_size *= 2;
            if (clp.warnings)
              Message("Reallocating array of primitives. New size is %d.\n",
                      primitive_array_size);
            if ((primitive = (primitive_type *)Realloc(
                     primitive,
                     primitive_array_size * sizeof(primitive_type))) == NULL) {
              Message("Not enough memory for array of primitives!\n");
              goto out;
            }
          }

          primitive[num_primitives].flag = ptr->flag;
          primitive[num_primitives].num_items = 0;
          primitive[num_primitives].data = num_floats;

          /* read in the data - floats */

          for (;;) {
            if (num_floats >= float_array_size) {
              float_array_size *= 2;
              if (clp.warnings)
                Message("Reallocating array of floats. New size is %d.\n",
                        float_array_size);
              if ((data = (float *)Realloc(data, float_array_size *
                                                     sizeof(float))) == NULL) {
                Message("Not enough memory for array of floats!\n");
                goto out;
              }
            }

            if (fscanf(fp, "%f", &data[num_floats]) != 1)
              break;
            num_floats++;
            primitive[num_primitives].num_items++;
          }
          num_primitives++;
          break; /* out of the loop going through all keywords */
        }
        ptr++;
      }

      if (ptr->flag == -1)
        Message("Uknown keyword: %s.\n", line);
    }
    fclose(fp);

    token = strtok(NULL, " ,;\n");
  }

  VERBOSE("%d primitives read.\n", num_primitives);
out:

  return 1;
}

/*************************************************************************/

void SetTurtle(TURTLE *tu) {
  float pt[3], ptc[3];
  float rotmat[16];
  int i;

  GetMatrix(rotmat);

  /* position */
  pt[0] = pt[1] = pt[2] = 0;

  Transform3Point(pt, rotmat, ptc);
  for (i = 0; i < 3; i++)
    tu->position[i] = ptc[i];

  /* heading */
  pt[0] = 0;
  pt[1] = 1;
  pt[2] = 0;

  Transform3Point(pt, rotmat, ptc);
  for (i = 0; i < 3; i++)
    tu->heading[i] = ptc[i] - tu->position[i];

  /* left */
  pt[0] = -1;
  pt[1] = 0;
  pt[2] = 0;

  Transform3Point(pt, rotmat, ptc);
  for (i = 0; i < 3; i++)
    tu->left[i] = ptc[i] - tu->position[i];

  /* up */
  pt[0] = 0;
  pt[1] = 0;
  pt[2] = -1;

  Transform3Point(pt, rotmat, ptc);
  for (i = 0; i < 3; i++)
    tu->up[i] = ptc[i] - tu->position[i];
}

/*************************************************************************/

void DrawBackgroundScene(DRAWPARAM *dr) {
  material_type store_mat;
  colormap_item_type store_cmapitem;
  int i;

  /* Go through the primitives twice, first draw opaque objects,
  then traslucent ones with read-only depth buffer (only when
  drawing on the screen, though) */
  if (num_primitives > 0) {

    if (clp.ismaterialfile) {
      /* material 0 will be used */
      my_getmaterial((Colorindex)clp.colormap, &mat);

      /* store its values */
      store_mat = *mat;
    } else if (clp.iscolormapfile) {
      /* colromap item 0 will be used */
      my_getcolormapitem((Colorindex)clp.colormap, &cmapitem);

      /* store its values */
      for (i = 0; i < 3; i++)
        store_cmapitem[i] = (*cmapitem)[i];
    } else {
      Message("Warning. Background scene cannot be drawn in index mode.\n");
      return;
    }

    backgroundscene = 1;

    DrawPrimitives(1, dr);

    if (dr->output_type == TYPE_OPENGL) {
      glFlush();
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glDepthMask(GL_FALSE);
    }

    /* transparent object are drawn in two parts - first triangles closer to
    the viewer and then those behind the object centre point */
#define AT_ONCE
    DrawPrimitives(2, dr);
#ifndef AT_ONCE
    DrawPrimitives(3, dr);
#endif

    backgroundscene = 0;

    if (dr->output_type == TYPE_OPENGL) {
      glFlush();
      glDepthMask(GL_TRUE);
      glDisable(GL_BLEND);
    }

    if (clp.ismaterialfile) {
      /* restore material values */
      *mat = store_mat;
    } else {
      /* restore colormap item values */
      for (i = 0; i < 3; i++)
        (*cmapitem)[i] = store_cmapitem[i];
      (*cmapitem)[3] = 255;
    }
  }
}

/*************************************************************************/
/* draws triangles. Simply calls dispatcher function unless which is
   1 or 2 (for transparent triangles) when for 1 closer triangles are drawn
   and for 2 triangles behind the centre point (as tested fo rthe first
   triangle vertex). Only for OpenGL output.*/
void DrawTriangle(float *p1, float *p2, float *p3,
                  DRAWPARAM *dr /*, int which*/) {
#ifndef AT_ONCE
  float vec[3];
  int i;
#endif

#ifdef AT_ONCE
  (*dr->tdd->RenderTriangle)(p1, p2, p3, dr);
#else
  if ((which == 0) || (dr->output_type != TYPE_OPENGL))
    (*dr->tdd->RenderTriangle)(p1, p2, p3, dr);

  /* handle rotations */
  for (i = 0; i <= 2; i++) {
    /* p1 minus volume center */
    vec[i] = p1[i] - (viewparam.min[i] + viewparam.max[i]) / 2.0;
  }

  d = viewNormal[0] * vec[0] + viewNormal[1] * vec[1] + viewNormal[2] * vec[2];

  if (((which == 1) && (d < 0)) || ((which == 2) && (d >= 0)))
    (*dr->tdd->RenderTriangle)(p1, p2, p3, dr);
#endif
}

/*************************************************************************/

void DrawRectangle(float *p1, float *p2, float *p3, float *p4, DRAWPARAM *dr
#ifndef AT_ONCE
                   ,
                   int which
#endif
) {
  float vec1[3], vec2[3], norm[3], normc[3];
  float zero[3] = {0}, zeroc[3];
  int j;
  float pt[3][PITEM];
  float rotmat[16];

  for (j = 0; j < 3; j++) {
    vec1[j] = p3[j] - p1[j];
    vec2[j] = p2[j] - p1[j];
  }
  /* get the normal */
  norm[0] = vec1[1] * vec2[2] - vec1[2] * vec2[1];
  norm[1] = vec1[2] * vec2[0] - vec1[0] * vec2[2];
  norm[2] = vec1[0] * vec2[1] - vec1[1] * vec2[0];

  if (dr->output_type == TYPE_RAYSHADE)
    for (j = 0; j < 3; j++)
      norm[j] = -norm[j];

  GetMatrix(rotmat);
  Transform3Point(norm, rotmat, normc);
  Transform3Point(zero, rotmat, zeroc);

  for (j = 0; j < 3; j++) {
    pt[j][NORMAL_X] = normc[0] - zeroc[0];
    pt[j][NORMAL_Y] = normc[1] - zeroc[1];
    pt[j][NORMAL_Z] = normc[2] - zeroc[2];
    pt[j][COLOR_FRONT] = clp.colormap;
    pt[j][COLOR_BACK] = clp.colormap;
    pt[j][DRAW_LINE] = 1;
  }

  Transform3Point(p1, rotmat, pt[0]);
  Transform3Point(p2, rotmat, pt[1]);
  Transform3Point(p3, rotmat, pt[2]);

  pt[2][DRAW_LINE] = 0; /* diagonal is not drawn in wireframe */

  DrawTriangle(pt[0], pt[1], pt[2], dr
#ifndef AT_ONCE
               ,
               which
#endif
  );

  Transform3Point(p4, rotmat, pt[1]);

  pt[2][DRAW_LINE] = 1;
  pt[0][DRAW_LINE] = 0; /* diagonal is not drawn in wireframe */

  DrawTriangle(pt[0], pt[2], pt[1], dr
#ifndef AT_ONCE
               ,
               which
#endif
  );
}

/*************************************************************************/

void DrawPrimitives(int run, DRAWPARAM *dr) {
  int i, j, x, y, z, c;
  float vec1[3], vec2[3], vec[2], norm[3], normc[3];
  float zero[3] = {0,0,0};
  float zeroc[3] = {0,0,0};
  float rotmat[16];
  float P[2][2][2][3];
  char draw = 1, translucent;
  float rotate90[4] = {90, 1, 0, 0};
  float translate[3] = {0,0,0};
  float pt[4][PITEM];  /* for drawing triangles */
  TURTLE dummy_turtle; /* for positioning spheres */

  dr->texture = 0;

  /* set the dummy turtle */
  InitializeTurtle(dr, &dummy_turtle);
  dummy_turtle.color_index = clp.colormap;

  InitializeMatrixStack();

  /* loop over all primitives */
  for (i = 0; i < num_primitives; i++)
    switch (primitive[i].flag) {
    case bMaterial:
      translucent = 0;

      if (clp.ismaterialfile) {
        if (primitive[i].num_items >= 4)
          for (j = 0; j < 4; j++)
            mat->ambient[j] = data[primitive[i].data + j];
        if (primitive[i].num_items >= 8) {
          for (j = 0; j < 4; j++)
            mat->diffuse[j] = data[primitive[i].data + 4 + j];
          if (data[primitive[i].data + 7] < 1)
            translucent = 1;
        }
        if (primitive[i].num_items >= 12) {
          for (j = 0; j < 4; j++)
            mat->specular[j] = data[primitive[i].data + 8 + j];
          if (data[primitive[i].data + 11] < 1)
            translucent = 1;
        }
        if (primitive[i].num_items >= 16) {
          for (j = 0; j < 4; j++)
            mat->emissive[j] = data[primitive[i].data + 12 + j];

          if (data[primitive[i].data + 15] < 1)
            translucent = 1;
        }
        if (primitive[i].num_items >= 17)
          mat->shininess = data[primitive[i].data + 16];
      } else {
        if (primitive[i].num_items >= 8) {
          for (j = 0; j < 4; j++)
            (*cmapitem)[j] = 255 * data[primitive[i].data + 4 + j];
        }
      }
      draw = run == 1 ? !translucent : translucent;

      {
        if (draw)
          (*dr->tdd->SetColour)(&dummy_turtle, dr, &viewparam);
      }
      break;
    case bPolygon:
      if (draw) {
        if (primitive[i].num_items >= 9) {

          for (j = 0; j < 3; j++) {
            vec1[j] =
                data[primitive[i].data + j] - data[primitive[i].data + 3 + j];
            vec2[j] = data[primitive[i].data + 6 + j] -
                      data[primitive[i].data + 3 + j];
          }
          /* get the normal */
          norm[0] = vec1[1] * vec2[2] - vec1[2] * vec2[1];
          norm[1] = vec1[2] * vec2[0] - vec1[0] * vec2[2];
          norm[2] = vec1[0] * vec2[1] - vec1[1] * vec2[0];

          if (dr->output_type == TYPE_RAYSHADE)
            for (j = 0; j < 3; j++)
              norm[j] = -norm[j];

          GetMatrix(rotmat);
          Transform3Point(norm, rotmat, normc);
          Transform3Point(zero, rotmat, zeroc);

          for (j = 0; j < 3; j++) {
            pt[j][NORMAL_X] = normc[0] - zeroc[0];
            pt[j][NORMAL_Y] = normc[1] - zeroc[1];
            pt[j][NORMAL_Z] = normc[2] - zeroc[2];
            pt[j][COLOR_FRONT] = dummy_turtle.color_index;
            pt[j][COLOR_BACK] = dummy_turtle.color_index_back;
            pt[j][DRAW_LINE] = 1;
          }

          Transform3Point(data + primitive[i].data, rotmat, pt[0]);
          Transform3Point(data + primitive[i].data + 3, rotmat, pt[1]);
          Transform3Point(data + primitive[i].data + 6, rotmat, pt[2]);

          if (primitive[i].num_items >= 12)
            pt[2][DRAW_LINE] = 0;

          DrawTriangle(pt[0], pt[1], pt[2], dr
#ifndef AT_ONCE
                       ,
                       run - 1
#endif
          );

          pt[0][DRAW_LINE] = 0;

          for (j = 9; j <= primitive[i].num_items - 3; j += 3) {
            for (c = 0; c < 3; c++) {
              pt[1][c] = pt[2][c];
            }
            Transform3Point(data + primitive[i].data + j, rotmat, pt[2]);

            if (j == primitive[i].num_items - 3)
              pt[2][DRAW_LINE] = 1;

            DrawTriangle(pt[0], pt[1], pt[2], dr
#ifndef AT_ONCE
                         ,
                         run - 1
#endif
            );
          }
        } else
          Message("At least three points must be specified for polygon.\n");
      }
      break;
    case bPolygonuv:
      if (draw) {
        if (primitive[i].num_items >= 9 * 2) {

          GetMatrix(rotmat);
          Transform3Point(zero, rotmat, zeroc);

          for (j = 0; j < 3; j++) {
            pt[j][COLOR_FRONT] = dummy_turtle.color_index;
            pt[j][COLOR_BACK] = dummy_turtle.color_index_back;
            pt[j][DRAW_LINE] = 1;
          }

          Transform3Point(data + primitive[i].data, rotmat, pt[0]);
          Transform3Vector(data + primitive[i].data + 3, rotmat, pt[0] + 3);
          Transform3Point(data + primitive[i].data + 6, rotmat, pt[1]);
          Transform3Vector(data + primitive[i].data + 9, rotmat, pt[1] + 3);
          Transform3Point(data + primitive[i].data + 12, rotmat, pt[2]);
          Transform3Vector(data + primitive[i].data + 15, rotmat, pt[2] + 3);

          if (dr->output_type == TYPE_RAYSHADE)
            for (j = 0; j < 3; j++) {
              pt[0][3 + j] = -pt[0][3 + j];
              pt[1][3 + j] = -pt[1][3 + j];
              pt[2][3 + j] = -pt[2][3 + j];
            }

          if (primitive[i].num_items >= 12 * 2)
            pt[2][DRAW_LINE] = 0;

          DrawTriangle(pt[0], pt[1], pt[2], dr
#ifndef AT_ONCE
                       ,
                       run - 1
#endif
          );

          pt[0][DRAW_LINE] = 0;

          for (j = 18; j <= primitive[i].num_items - 6; j += 6) {
            for (c = 0; c < 6; c++) {
              pt[1][c] = pt[2][c];
            }
            Transform3Point(data + primitive[i].data + j, rotmat, pt[2]);
            Transform3Point(data + primitive[i].data + j + 3, rotmat,
                            pt[2] + 3);

            if (dr->output_type == TYPE_RAYSHADE)
              for (c = 0; c < 3; c++)
                pt[2][3 + c] = -pt[2][3 + c];

            if (j == primitive[i].num_items - 6)
              pt[2][DRAW_LINE] = 1;

            DrawTriangle(pt[0], pt[1], pt[2], dr
#ifndef AT_ONCE
                         ,
                         run - 1
#endif
            );
          }
        } else
          Message("At least three points with normals must be specified for "
                  "polygonuv.\n");
      }
      break;
    case bRectangle:
      if (draw) {
        if (primitive[i].num_items >= 2) {
          vec[0] = vec[1] = 0;

          if (primitive[i].num_items >= 4) {
            vec[0] = data[primitive[i].data + 2];
            vec[1] = data[primitive[i].data + 3];

            if (primitive[i].num_items >= 5) {
              if (clp.ismaterialfile)
                for (c = 0; c < 3; c++) {
                  mat->emissive[c] *= data[primitive[i].data + 4];
                }
              else
                for (c = 0; c < 3; c++)
                  (*cmapitem)[c] *= data[primitive[i].data + 4];
            }
          }

          for (j = 0; j < 4; j++) {
            pt[j][2] = 0;
          }

          for (j = 0; j < 2; j++) {
            pt[0][j] = vec[j];
          }
          pt[1][0] = vec[0] + data[primitive[i].data];
          pt[1][1] = vec[1];
          pt[2][0] = vec[0] + data[primitive[i].data];
          pt[2][1] = vec[1] + data[primitive[i].data + 1];
          pt[3][0] = vec[0];
          pt[3][1] = vec[1] + data[primitive[i].data + 1];

          DrawRectangle(pt[0], pt[1], pt[2], pt[3], dr
#ifndef AT_ONCE
                        ,
                        run - 1
#endif
          );

          /* set back the original material or colormap item */
          if (primitive[i].num_items >= 5) {
            if (clp.ismaterialfile)
              for (c = 0; c < 3; c++) {
                mat->emissive[c] /= data[primitive[i].data + 4];
              }
            else
              for (c = 0; c < 3; c++)
                (*cmapitem)[c] /= data[primitive[i].data + 4];
          }
        } else
          Message("At least two values must be specified for rectangle.\n");
      }
      break;
    case bMesh:
      if (draw) {
        if (primitive[i].num_items >= 4 * 3)
          for (j = 0; j <= primitive[i].num_items - 4 * 3; j += 2 * 3) {
            DrawRectangle(data + primitive[i].data + j,
                          data + primitive[i].data + j + 6,
                          data + primitive[i].data + j + 9,
                          data + primitive[i].data + j + 3, dr
#ifndef AT_ONCE
                          ,
                          run - 1
#endif
            );
          }
        else
          Message("At least four points must be specified for mesh.\n");
      }
      break;
    case bSphere:
      if (draw) {
        if (primitive[i].num_items >= 1) {
          SetTurtle(&dummy_turtle);
          (*dr->tdd->Sphere)(&dummy_turtle, dr, &viewparam,
                             data[primitive[i].data] * 2);
        } else
          Message("Warning: not enough parameters for sphere!\n");
      }
      break;
    case bCylinder:
      if (draw) {
        if (primitive[i].num_items >= 2) {
          SetTurtle(&dummy_turtle);

          dummy_turtle.line_width = 2 * data[primitive[i].data];

          (*dr->tdd->StartNode)(&dummy_turtle, dr, &viewparam,
                                data[primitive[i].data + 1], '\0');

          PushMatrix();
          Rotate(rotate90);
          SetTurtle(&dummy_turtle);
          (*dr->tdd->Circle3D)(&dummy_turtle, dr, &viewparam,
                               data[primitive[i].data] * 2);
          PopMatrix();

          PushMatrix();
          translate[1] = data[primitive[i].data + 1];
          Translate(translate);
          SetTurtle(&dummy_turtle);

          (*dr->tdd->EndNode)(&dummy_turtle, dr, &viewparam, '\0');

          Rotate(rotate90);
          SetTurtle(&dummy_turtle);

          (*dr->tdd->Circle3D)(&dummy_turtle, dr, &viewparam,
                               data[primitive[i].data] * 2);
          PopMatrix();

          dummy_turtle.position[1] = 0; /* back default 0 */
        } else
          Message("Warning: not enough parameters for cylinder!\n");
      }
      break;

    case bCone:
      if (draw) {
        if (primitive[i].num_items >= 3) {
          SetTurtle(&dummy_turtle);

          dummy_turtle.line_width = 2 * data[primitive[i].data];

          (*dr->tdd->StartNode)(&dummy_turtle, dr, &viewparam,
                                data[primitive[i].data + 2], '\0');

          PushMatrix();
          Rotate(rotate90);
          SetTurtle(&dummy_turtle);
          (*dr->tdd->Circle3D)(&dummy_turtle, dr, &viewparam,
                               data[primitive[i].data] * 2);
          PopMatrix();

          PushMatrix();
          translate[1] = data[primitive[i].data + 2];
          Translate(translate);
          SetTurtle(&dummy_turtle);
          dummy_turtle.line_width = 2 * data[primitive[i].data + 1];

          (*dr->tdd->EndNode)(&dummy_turtle, dr, &viewparam, '\0');

          Rotate(rotate90);
          SetTurtle(&dummy_turtle);

          (*dr->tdd->Circle3D)(&dummy_turtle, dr, &viewparam,
                               data[primitive[i].data + 1] * 2);
          PopMatrix();

          dummy_turtle.position[1] = 0; /* back default 0 */
        } else
          Message("Warning: not enough parameters for cylinder!\n");
      }
      break;
    case bPrism:
      if (draw) {
        if (primitive[i].num_items >= 3) {
          for (x = 0; x < 2; x++)
            for (y = 0; y < 2; y++)
              for (z = 0; z < 2; z++) {
                P[x][y][z][0] = x * data[primitive[i].data];
                P[x][y][z][1] = y * data[primitive[i].data + 1];
                P[x][y][z][2] = z * data[primitive[i].data + 2];
              }

          DrawRectangle(P[0][0][0], P[0][0][1], P[0][1][1], P[0][1][0], dr
#ifndef AT_ONCE
                        ,
                        run - 1
#endif
          );
          DrawRectangle(P[1][0][0], P[1][1][0], P[1][1][1], P[1][0][1], dr
#ifndef AT_ONCE
                        ,
                        run - 1
#endif
          );

          DrawRectangle(P[0][0][0], P[0][1][0], P[1][1][0], P[1][0][0], dr
#ifndef AT_ONCE
                        ,
                        run - 1
#endif
          );
          DrawRectangle(P[0][0][1], P[1][0][1], P[1][1][1], P[0][1][1], dr
#ifndef AT_ONCE
                        ,
                        run - 1
#endif
          );

          DrawRectangle(P[0][0][0], P[0][0][1], P[1][0][1], P[1][0][0], dr
#ifndef AT_ONCE
                        ,
                        run - 1
#endif
          );
          DrawRectangle(P[0][1][0], P[1][1][0], P[1][1][1], P[0][1][1], dr
#ifndef AT_ONCE
                        ,
                        run - 1
#endif
          );
        } else
          Message("Warning: not enough parameterss for prism!\n");
      }
      break;
    case bPushmatrix:
      PushMatrix();
      if (primitive[i].num_items >= 1)
        Message("Warning: pushmatrix doesn't need a parameter.\n");
      break;
    case bPopmatrix:
      PopMatrix();
      if (primitive[i].num_items >= 1)
        Message("Warning: pushmatrix doesn't need a parameter.\n");
      break;
    case bTranslate:
      if (primitive[i].num_items >= 3)
        Translate(data + primitive[i].data);
      else
        Message("Warning: not enough parameters for translate!\n");
      break;
    case bRotate:
      if (primitive[i].num_items >= 4)
        Rotate(data + primitive[i].data);
      else
        Message("Warning: not enough parameters for rotate!\n");
      break;
    case bScale:
      if (primitive[i].num_items >= 3)
        Scale(data + primitive[i].data);
      else
        Message("Warning: not enough parameters for scale!\n");
      break;
    case bMultmatrix:
      if (primitive[i].num_items >= 16)
        MultMatrix(data + primitive[i].data);
      else
        Message("Warning: not enough parameters for scale!\n");
      break;

    default:
      Message("Warning: unknown primitive.\n");
    }
}
