

/********************************************************************/
/*                           rayshade.c               		    */
/*                                                                  */
/* rayshade provides interpret routines for producing               */
/* rayshade readable ascii output                                   */
/* Routines are dispatched via the turtleDrawDispatcher             */
/*                                                                  */
/********************************************************************/
/*
        CREATED: August 1991 BY: Jim at Apple
        MODIFIED: March-June 1994 BY: Radek
                ansi standard + prepared for C++
       FIX: temporary file is not created in OpenGL lighting mode

*/

#ifdef WIN32
#include "warningset.h"
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

#ifndef WIN32
#include <unistd.h>
#else
#include <io.h>
#endif

#include "platform.h"
#include "control.h"
#include "interpret.h"
#include "generate.h"
#include "rayshade.h"
#include "utility.h"
#include "patch.h"
#include "textures.h"
#include "mesh.h"
#include "irisGL.h"
#include "hash.h"
#include "indices.h"

#include "test_malloc.h"

/* The structure for dispatching rendering routines */
turtleDrawDispatcher rayshadeDrawRoutines = {
    rsSetup,        rsStartNode,         rsEndNode,         rsStartBranch,
    rsEndBranch,    rsStartPolygon,      rsEndPolygon,      rsSetColour,
    rsSetLineWidth, rsCircle2D,          rsCircle3D,        rsSphere,
    rsBlackBox,     rsPredefinedSurface, rsLdefinedSurface, rsLabel,
    rsFinishUp,     rsRenderTriangle,    rsStartTexture,    rsEndTexture,
    StartTmeshT,    TmeshVertexT,        EndTmeshT,         rsCircleB2D,
    rsCircleB3D};

/* flag indicating that nodes should be joined */
static int connectNode;
static int usedSurface[4096];

/* prevents multiple image file conversions when a texture used more times */
static char texture_rle_file_created[NUMTEXTURES];

static char *object_name;
static int object_number;

static int current_index; /* currrent color/material index */

/* local prototypes */
static void r_finishup_surfaces(FILE *fp);

static void r_setup_surfaces(void);
static void r_circle(FILE *fp, const TURTLE *tu, double width);
static void r_sphere(FILE *fp, const TURTLE *tu, double width);
static void r_start_cylinder(FILE *fp, TURTLE *tu);
static void r_end_cylinder(FILE *fp, TURTLE *tu);
static void r_header(FILE *fp, VIEWPARAM *vw);
static void r_trailer(FILE *fp, VIEWPARAM *vw, DRAWPARAM *dr);
static void r_object(FILE *fp, TURTLE *tu, char desired_surface, double, double,
                     double);
static void PrintSurface(FILE *fp, int x);
void print_surface_use(FILE *fp, int color_index);
void rsOutputObjects(void);
void r_tsurfaces(FILE *, TURTLE *, const DRAWPARAM *, VIEWPARAM *);

/* precision of output for point coordinates and normals */
/* Both must start with space !!! */
char coordinate_format[] = " %lg";
char normal_format[] = " %g";

static char triangle_cformat[20];
static char triangle_nformat[20];

FILE *tmpfp;
int default_material;

char creating_surfaces;

char pass, output;

/********************************************************************/
/* Function: rsSetup                                                */
/* return non-zero if there are problems                            */
/********************************************************************/

int rsSetup(TURTLE *tu, DRAWPARAM *dr, VIEWPARAM *vw) {
  char tmpnm[255], *ptr;
  int i;
  float min[3], max[3];
  char format[30];
  extern char isObjectProduction;

  if (clp.savefp[SAVE_RAYSHADE] == NULL) {
    return 1;
  }

  if (pass == 1 || !isObjectProduction) {

    if (isObjectProduction)
      InitializeHashTable();

    /* create format strings for triangles */
    strcpy(triangle_cformat, coordinate_format + 1);
    strcpy(triangle_nformat, normal_format);
    for (i = 0; i < 2; i++) {
      strcat(triangle_cformat, coordinate_format);
      strcat(triangle_nformat, normal_format);
    }

    if (clp.ismaterialfile) {
      tmpfp = clp.savefp[SAVE_RAYSHADE];

#ifndef WIN32
      if (tmpfp == stdout) {
        setlinebuf(stdout);
      }
#endif

      fprintf(tmpfp, "#ifndef NOSURFACES\n");

      default_material = -1;
      /* save all surfaces now */
      for (i = 0; i < MAXCOLORMAPS * MAXINCOLORMAP; i++) {
        if (is_material((Colorindex)i)) {
          PrintSurface(tmpfp, i);
          usedSurface[i] = 1;
        } else {
          if (default_material == -1) {
            default_material = i;
            PrintSurface(tmpfp, default_material);
          }
          usedSurface[i] = 0;
        }
      }

      fprintf(tmpfp, "#endif\n");
    } else {
      strcpy(tmpnm, "/tmp/cpfg.ray.XXXXXX");
#ifdef LINUX
      tmpfp = fdopen(mkstemp(tmpnm), "w+");
#else
      mktemp(tmpnm);
      tmpfp = fopen(tmpnm, "w+");
#endif

      unlink(tmpnm);

      r_setup_surfaces();
    }

    r_header(tmpfp, vw);

    /**/
    fprintf(tmpfp, "\n#ifdef BBOX\n");

    /* Radek - stripping directory from the name */
    if ((ptr = strrchr(clp.savefilename[SAVE_RAYSHADE], '/')) == NULL)
      ptr = clp.savefilename[SAVE_RAYSHADE];
    else
      ptr++;

    object_name = Strdup(ptr);
    object_number = 1;

    /* object number is not added to the first object */
    if (dr->rayshade_scale == 1)
      fprintf(tmpfp, "name %s\n", object_name);
    else
      fprintf(tmpfp, "name %s.orig\n", object_name);

    for (i = 0; i < 3; i++) {
      min[i] = vw->min[i];
      max[i] = vw->max[i];
      if ((max[i] - min[i]) < 0.00001)
        max[i] = min[i] + 0.00001;
    }

    strcpy(format, "box");
    for (i = 0; i < 6; i++)
      strcat(format, coordinate_format);
    strcat(format, "\n");

    fprintf(tmpfp, format, min[eX], min[eY], min[eZ], max[eX], max[eY],
            max[eZ]);
    fprintf(tmpfp, "#else\n");

    creating_surfaces = 1;
    output = 1;
    r_objects(tmpfp, tu, dr, vw);   /* output surface objects */
    r_tsurfaces(tmpfp, tu, dr, vw); /* output tsurface objects */
    creating_surfaces = 0;

    for (i = 0; i < NUMTEXTURES; i++)
      texture_rle_file_created[i] = 0;
  }

  if (pass == 2) {
    /* object number is not added to the first object */
    if (dr->rayshade_scale == 1)
      fprintf(tmpfp, "name %s\n", object_name);
    else
      fprintf(tmpfp, "name %s.orig\n", object_name);

    fprintf(tmpfp, "grid 40 40 40 \n");

    r_use_surface(tu->color_index);
    fprintf(tmpfp, "applysurf ");
    print_surface_use(tmpfp, tu->color_index);
    fprintf(tmpfp, "\n");

    current_index = tu->color_index;

    /* no segment created yet */
    connectNode = FALSE;
  }

  output = pass == 1 ? 0 : 1;

  return 0;
}

/********************************************************************/
/* Function: rsStartNode                                            */
/********************************************************************/

void rsStartNode(TURTLE *tu, DRAWPARAM *dr,
                 VIEWPARAM *vw,
                 float length,
                 char symbol) {
  if (!output)
    return;

  r_start_cylinder(tmpfp, tu);
  r_use_surface(tu->color_index);
  /* this node should be connected to the next */
  connectNode = TRUE;
}

/********************************************************************/
/* Function: rsEndNode                                              */
/********************************************************************/

void rsEndNode(TURTLE *tu, DRAWPARAM *dr,
               VIEWPARAM *vw,
               char symbol) {
  if (!output)
    return;

  if (connectNode) {
    r_end_cylinder(tmpfp, tu);
    connectNode = FALSE;
  }
}

/********************************************************************/
/* Function: rsStartBranch                                          */
/* No action                                                        */
/********************************************************************/

void rsStartBranch(TURTLE *tu,
                   DRAWPARAM *dr,
                   VIEWPARAM *vw) {}

/********************************************************************/
/* Function: rsEndBranch                                            */
/* No action                                                        */
/********************************************************************/

void rsEndBranch(TURTLE *tu, DRAWPARAM *dr, VIEWPARAM *vw) {
  if (!output)
    return;

  if (current_index != tu->color_index)
    rsSetColour(tu, dr, vw);
}

/********************************************************************/
/* Function: rsStartPolygon                                         */
/* No action                                                        */
/********************************************************************/

void rsStartPolygon(POLYGON *polygon,
                    TURTLE *tu,
                    DRAWPARAM *dr,
                    VIEWPARAM *vw) {}

/********************************************************************/
/* Function: rsEndPolygon                                           */
/* Output polygon vertices                                          */
/********************************************************************/

void rsEndPolygon(POLYGON *polygon, TURTLE *tu, DRAWPARAM *dr,
                  VIEWPARAM *vw) {
  int i, j;

  if (!output)
    return;

  dr->texture = is_valid_texture_index(polygon->vertex[0].texture);

  if (dr->texture) {
    /* textured polygons must be output as a mesh */
    PolygonToMesh(polygon, dr);
  } else {
    r_use_surface(tu->color_index);

    fprintf(tmpfp, "poly ");
    print_surface_use(tmpfp, polygon->vertex[0].color_index);

    for (i = 0; i < polygon->edge_count; i++) {
      for (j = 0; j < 3; j++) {
        fprintf(tmpfp, coordinate_format, (polygon->vertex[i]).position[j]);
      }
      fprintf(tmpfp, "\n");
    }
  }
}

/********************************************************************/
/* Function: rsSetColour                                            */
/* No action                                                        */
/********************************************************************/

void rsSetColour(const TURTLE *tu, const DRAWPARAM *dr,
                 const VIEWPARAM *vw) {
  extern char backgroundscene;

  if (!output)
    return;

  if (backgroundscene)
    PrintSurface(tmpfp, tu->color_index);
  else
    r_use_surface(tu->color_index);

  fprintf(tmpfp, "applysurf ");
  print_surface_use(tmpfp, tu->color_index);
  fprintf(tmpfp, "\n");

  current_index = tu->color_index;
}

/********************************************************************/
/* Function: rsSetLineWidth                                         */
/* No action                                                        */
/********************************************************************/

void rsSetLineWidth(const TURTLE *tu,
                    const DRAWPARAM *dr,
                    const VIEWPARAM *vw) {}

/********************************************************************/
/* Function: rsCircle2D                                             */
/********************************************************************/

void rsCircle2D(const TURTLE *tu, const DRAWPARAM *dr,
                VIEWPARAM *vw, float diameter) {
  if (!output)
    return;

  r_use_surface(tu->color_index);
  r_circle(tmpfp, tu, (double)(diameter / 2.0));
}

/********************************************************************/
/* Function: rsCircle3D                                             */
/********************************************************************/

void rsCircle3D(const TURTLE *tu, const DRAWPARAM *dr,
                VIEWPARAM *vw, float diameter) {
  if (!output)
    return;

  r_use_surface(tu->color_index);
  r_circle(tmpfp, tu, (double)(diameter / 2.0));
}
/********************************************************************/
/* Function: objCircleB2D                                            */
/********************************************************************/

void rsCircleB2D(const TURTLE *tu,
                 const DRAWPARAM *dr,
                 const VIEWPARAM *vw,
                 float diameter,
                 float width) {}

void rsCircleB3D(const TURTLE *tu,
                 const DRAWPARAM *dr,
                 const VIEWPARAM *vw,
                 float diameter,
                 float width) {}

/********************************************************************/
/* Function: rsSphere                                               */
/********************************************************************/

void rsSphere(const TURTLE *tu, const DRAWPARAM *dr,
              VIEWPARAM *vw, float diameter) {
  if (!output)
    return;

  r_use_surface(tu->color_index);
  r_sphere(tmpfp, tu, (double)(diameter / 2.0));
}

/********************************************************************/
/* Function: rsBLackBox                                             */
/********************************************************************/

void rsBlackBox(const TURTLE *tu,
                const DRAWPARAM *dr,
                const VIEWPARAM *vw,
                const StringModule *module,
                const StringModule *submodule) {}

/********************************************************************/
/* Function: rsLabel                   JH1                          */
/********************************************************************/

void rsLabel(const TURTLE *tu,
             DRAWPARAM *dr,
             const VIEWPARAM *vw,
             const char *label,
             int parameteri,
             const float values[]) {}

/********************************************************************/
/* Function: rsStartTexture                                         */
/* Initializes texture                                              */
/********************************************************************/

char use_texture = 0;

#define MAP_UV 0
#define MAP_CYL 1

int rsStartTexture(int index) {
  if (!output)
    return 0;

  if (creating_surfaces)
    return 1;

  if (!is_valid_texture_index(index))
    return 0;
  use_texture = 1;

  fprintf(tmpfp, "list\n"); /* list of objects having the same texture */

  return 1;
}

/********************************************************************/
void FinishUpTexture(int index, int map_type, int list_started) {
  struct TEXTURET *tex;
  char rle_name[2256];
  char smooth[10];
  char *name;

  if (!output)
    return;

  tex = GetTexture(index);

  strcpy(rle_name, tex->filename);

  /* strip the possible directory */
  if ((name = strrchr(rle_name, '/')) == NULL)
    name = rle_name;
  else
    name++;

  if (tex->mag_filter == GL_NEAREST)
    strcpy(smooth, " ");
  else
    strcpy(smooth, "smooth");

  /* finish the list started in StartTexture */
  if (map_type == MAP_UV && list_started)
    fprintf(tmpfp, "end\n");

  fprintf(tmpfp, "texture image %s %s\n", name, smooth);

  if (map_type == MAP_UV)
    fprintf(tmpfp, "map uv\n");
}

/********************************************************************/
void rsEndTexture(int index) {
  if (!output)
    return;

  if (use_texture) {
    use_texture = 0;

    FinishUpTexture(index, MAP_UV, 1);
  }
}

/********************************************************************/
/* Function: rsPredefinedSurface                                    */
/********************************************************************/

void rsPredefinedSurface(TURTLE *tu, DRAWPARAM *dr,
                         VIEWPARAM *vw, char id,
                         double sX, double sY, double sZ) {
  if (!output)
    return;
  if (sX != 0.0) {
    r_object(tmpfp, tu, id, sX, sY, sZ);
  }
}

/********************************************************************/
/* Function: rsLdefinedSurface                                      */
/* No action                                                        */
/********************************************************************/

void rsLdefinedSurface(StringModule *module, TURTLE *tu, DRAWPARAM *dr,
                       VIEWPARAM *vw) {
  if (!output)
    return;

  r_use_surface(tu->color_index);
  dr->gllighting = 1;
  dr->vertexbound = 1;
  dr->ourlighting = 0;
  dr->texture = 0;

  SurfaceTmeshDraw(module, tu, dr, vw, StartTmeshT, TmeshVertexT, EndTmeshT);
}

/********************************************************************/
void rsStartNewGrid(TURTLE *tu, DRAWPARAM *dr, VIEWPARAM *vw, int *size) {
  int c;

  if (!output)
    return;

  /* some checking */
  if ((size == NULL) || (size[0] < 0) || (size[1] < 0) || (size[2] < 0))
    return;

  /* end the previous grid - make sure it is not empty */
  fprintf(tmpfp, "sphere transp 1 noshadow 0.0001 0 0 0 end\n");
  r_trailer(tmpfp, vw, dr);

  object_number++;

  if (dr->rayshade_scale == 1)
    fprintf(tmpfp, "name %s%d\n", object_name, object_number);
  else
    fprintf(tmpfp, "name %s%d.orig\n", object_name, object_number);

  for (c = 0; c < 3; c++)
    if (size[c] == 0)
      size[c] = 40;

  fprintf(tmpfp, "grid %d %d %d\n", size[0], size[1], size[2]);

  r_use_surface(tu->color_index);
  fprintf(tmpfp, "applysurf ");
  print_surface_use(tmpfp, tu->color_index);
  fprintf(tmpfp, "\n");

  /* no segment created yet */
  connectNode = FALSE;
}

/********************************************************************/
/* Function: rsFinishUp                                             */
/* No action                                                        */
/********************************************************************/

void rsFinishUp(TURTLE *tu, DRAWPARAM *dr,
                VIEWPARAM *vw) {
  int c;

  if (pass == 1) {
    /* process all objects stored in the hash table */
    rsOutputObjects();

    return;
  }

  if (!clp.ismaterialfile)
    r_finishup_surfaces(clp.savefp[SAVE_RAYSHADE]);

  /* finish the grid and put endif for:
  ifdef BBOX
  box
  else
object */
  fprintf(tmpfp, "end\n#endif\n\n");
  r_trailer(tmpfp, vw, dr);

  if (!clp.ismaterialfile) {
    rewind(tmpfp);
    while ((c = fgetc(tmpfp)) != EOF)
      fputc(c, clp.savefp[SAVE_RAYSHADE]);
    fclose(tmpfp);
  }

  if (clp.savefp[SAVE_RAYSHADE] != stdin)
    fclose(clp.savefp[SAVE_RAYSHADE]);
  clp.savefp[SAVE_RAYSHADE] = NULL;

  if (pass == 2)
    FreeHashTable();
}

/********************************************************************/
/* Function: rsRenderTriangle                                       */
/* "renders" triangle                                               */
/********************************************************************/

void rsRenderTriangle(const float *p1, const float *p2, const float *p3,
                      const DRAWPARAM *dr) {
  float vec1[3], vec2[3], normal[3];
  int c;

  if (!output)
    return;

  if (dr->texture)
    fprintf(tmpfp, "triangleuv");
  else
    fprintf(tmpfp, "triangle");

  if (dr->ourlighting) {
    r_use_surface(r_patch_color((int)p1[COLOR_FRONT]));
    fprintf(tmpfp, " ");
    print_surface_use(tmpfp, r_patch_color((int)p1[COLOR_FRONT]));
    fprintf(tmpfp, "\n");
  } else {
    fprintf(tmpfp, "\n");
  }

  fprintf(tmpfp, triangle_cformat, p1[ePOINT * 3], p1[ePOINT * 3 + 1],
          p1[ePOINT * 3 + 2]);

  if (dr->ourlighting || dr->gllighting) {
    /* get the triangle normal */
    for (c = 0; c < 3; c++) {
      vec1[c] = p2[ePOINT * 3 + c] - p1[ePOINT * 3 + c];
      vec2[c] = p3[ePOINT * 3 + c] - p1[ePOINT * 3 + c];
    }
    CrossProduct(vec1, vec2, normal);

    if (DotProduct(normal, p1 + eNORMAL * 3) > 0)
      fprintf(tmpfp, triangle_nformat, p1[eNORMAL * 3], p1[eNORMAL * 3 + 1],
              p1[eNORMAL * 3 + 2]);
    else
      fprintf(tmpfp, triangle_nformat, -p1[eNORMAL * 3], -p1[eNORMAL * 3 + 1],
              -p1[eNORMAL * 3 + 2]);
  }

  if (dr->texture)
    fprintf(tmpfp, " %.4g %.4g", p1[eTEXTURE * 3], p1[eTEXTURE * 3 + 1]);
  fprintf(tmpfp, "\n");

  fprintf(tmpfp, triangle_cformat, p2[ePOINT * 3], p2[ePOINT * 3 + 1],
          p2[ePOINT * 3 + 2]);
  if (dr->ourlighting || dr->gllighting) {
    if (DotProduct(normal, p2 + eNORMAL * 3) > 0)
      fprintf(tmpfp, triangle_nformat, p2[eNORMAL * 3], p2[eNORMAL * 3 + 1],
              p2[eNORMAL * 3 + 2]);
    else
      fprintf(tmpfp, triangle_nformat, -p2[eNORMAL * 3], -p2[eNORMAL * 3 + 1],
              -p2[eNORMAL * 3 + 2]);
  }

  if (dr->texture)
    fprintf(tmpfp, " %.4g %.4g", p2[eTEXTURE * 3], p2[eTEXTURE * 3 + 1]);
  fprintf(tmpfp, "\n");

  fprintf(tmpfp, triangle_cformat, p3[ePOINT * 3], p3[ePOINT * 3 + 1],
          p3[ePOINT * 3 + 2]);
  if (dr->ourlighting || dr->gllighting) {
    if (DotProduct(normal, p3 + eNORMAL * 3) > 0)
      fprintf(tmpfp, triangle_nformat, p3[eNORMAL * 3], p3[eNORMAL * 3 + 1],
              p3[eNORMAL * 3 + 2]);
    else
      fprintf(tmpfp, triangle_nformat, -p3[eNORMAL * 3], -p3[eNORMAL * 3 + 1],
              -p3[eNORMAL * 3 + 2]);
  }

  if (dr->texture)
    fprintf(tmpfp, " %.4g %.4g", p3[eTEXTURE * 3], p3[eTEXTURE * 3 + 1]);
  fprintf(tmpfp, "\n");
}

/* Output routines for rayshade. */

static void r_circle(FILE *fp, const TURTLE *tu, double width) {
  int j;

  if (!output)
    return;

  fprintf(fp, "disc ");
  print_surface_use(fp, tu->color_index);

  fprintf(fp, coordinate_format, (float)(width));
  for (j = 0; j < 3; j++)
    fprintf(fp, coordinate_format, (float)(tu->position[j]));
  for (j = 0; j < 3; j++)
    fprintf(fp, coordinate_format, (float)(tu->up[j]));
  fprintf(fp, "\n");
}

static void r_sphere(FILE *fp, const TURTLE *tu, double width) {
  int j;

  if (!output)
    return;

  fprintf(fp, "sphere ");
  print_surface_use(fp, tu->color_index);

  fprintf(fp, coordinate_format, (float)(width));
  for (j = 0; j < 3; j++)
    fprintf(fp, coordinate_format, (float)(tu->position[j]));
  fprintf(fp, "\n");
}

/*********************************************************************/
double StartPos[3];
double StartH[3];
double StartL[3];
double StartWidth;
int StartIndex;
float StartTexT;

static void r_start_cylinder(FILE *fp, TURTLE *tu) {
  int c;

  if (!output)
    return;

  StartIndex = tu->color_index;
  StartWidth = tu->line_width;

  for (c = 0; c < 3; c++) {
    StartPos[c] = tu->position[c];
    StartH[c] = tu->heading[c];
    StartL[c] = tu->left[c];
  }
  StartTexT = tu->tex_t;
}

static void r_end_cylinder(FILE *fp, TURTLE *tu) {
  int j;
  float add_t;
  double vec[3];

  if (!output)
    return;

  for (j = 0; j < 3; j++)
    vec[j] = tu->position[j] - StartPos[j];

  /* test for degenerated cone */
  if (vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2] <= 0.00001 ||
      tu->line_width <= 0.0)
    return;

  /* cylinder or cone? */
  if (StartWidth == tu->line_width)
    fprintf(fp, "cylinder ");
  else
    fprintf(fp, "cone ");

  print_surface_use(fp, StartIndex);

  fprintf(fp, coordinate_format, (float)(StartWidth / 2.));
  for (j = 0; j < 3; j++)
    fprintf(fp, coordinate_format, (float)(StartPos[j]));
  fprintf(fp, "\n");

  if (StartWidth != tu->line_width)
    fprintf(fp, coordinate_format, (float)(tu->line_width / 2.));

  for (j = 0; j < 3; j++)
    fprintf(fp, coordinate_format, (float)(tu->position[j]));
  fprintf(fp, "\n");

  if (is_valid_texture_index(tu->texture)) {

    add_t = update_segment_texture(tu->texture, tu->line_width, 1.0);

    FinishUpTexture(tu->texture, MAP_CYL, 0);

    fprintf(fp, "  map cylindrical %g %g %g %g %g %g %g %g %g\n", StartPos[0],
            StartPos[1], StartPos[2], StartH[0], StartH[1], StartH[2],
            StartL[0], StartL[1], StartL[2]);
    fprintf(fp, "  translate 0 %g 0 scale 1 %g 1\n", -StartTexT, 1.0 / add_t);
  }
}

static void r_header(FILE *fp, VIEWPARAM *vw) {
  short r, g, b;
  int li;
  double dist, alpha;
  material_type *mat;
  double vrp[3], eye[3];
  float gl_light_pos[4], gl_light_dir[4];
  extern struct light_parms gl_lights[NUMLIGHTS];
  extern int gl_numlights;
  extern DRAWPARAM drawparam;
  extern RECTANGLE viewWindow;
  double parallel_scale = 100.0;

  fprintf(fp, "#ifndef NOHEADERS\n");

  // MC - Dec. 2015 - fix rayshade output when in parallel projection by moving
  // camera far back compute eye position in the same way as cpfg
  eye[0] = vw->vrp[0] + (vw->viewpoint[0] - vw->vrp[0]) / vw->scale;
  eye[1] = vw->vrp[1] + (vw->viewpoint[1] - vw->vrp[1]) / vw->scale;
  eye[2] = vw->vrp[2] + (vw->viewpoint[2] - vw->vrp[2]) / vw->scale;

  // if parrallel projection mode, move eye far back to fake it in perspective
  // mode
  if (vw->parallel_projection_on) {
    eye[0] = eye[0] + (eye[0] - vw->vrp[0]) * parallel_scale * 0.99;
    eye[1] = eye[1] + (eye[1] - vw->vrp[1]) * parallel_scale * 0.99;
    eye[2] = eye[2] + (eye[2] - vw->vrp[2]) * parallel_scale * 0.99;

    // set look at to centre of view volume
    vrp[0] = (vw->max[0] + vw->min[0]) * 0.5; //*vw->scale;
    vrp[1] = (vw->max[1] + vw->min[1]) * 0.5; //*vw->scale;
    vrp[2] = (vw->max[2] + vw->min[2]) * 0.5; //*vw->scale;
  } else {
    // set look at to view reference point in persepective mode
    vrp[0] = vw->vrp[0];
    vrp[1] = vw->vrp[1];
    vrp[2] = vw->vrp[2];
  }

  fprintf(fp, "eyep %f %f %f\n", eye[0], eye[1], eye[2]);
  fprintf(fp, "lookp %f %f %f\n", vrp[0], vrp[1], vrp[2]);
  fprintf(fp, "up %f %f %f\n", vw->view_up[0], vw->view_up[1], vw->view_up[2]);

  if (vw->parallel_projection_on) {
    alpha = atan(fabs((viewWindow.left - viewWindow.right) * 0.5 * vw->scale /
                      (vw->viewpoint[2] - vw->vrp[2] / vw->scale) /
                      parallel_scale)) /
            M_PI * 360.0;

  } else {
    /* perspective */
    /* fov is taken vertically in openGL, horizontally in rayshade */
    dist = (int)(clp.ysize) * 1.0 / tan(vw->viewing_angle * M_PI / 360.0);
    /*  * 1.0/tan(va/2) */
    alpha = atan((int)(clp.xsize) / dist) / M_PI * 360;
  }
  fprintf(fp, "fov %f\n", alpha);

  if ((int)(clp.xsize))
    fprintf(fp, "screen %d %d\n", (int)(clp.xsize), (int)( clp.ysize));

  if (clp.ismaterialfile) {
    /* must get emissive color */
    my_getmaterial((short)clp.colormap, &mat);
    fprintf(fp, "background %f %f %f\n", mat->emissive[0], mat->emissive[1],
            mat->emissive[2]);
  } else {
    my_getmcolor((short)clp.colormap, &r, &g, &b);
    fprintf(fp, "background %f %f %f\n", (float)r / 255.0, (float)g / 255.0,
            (float)b / 255.0);
  }

  /* lights */
  if (gl_numlights == 0) {
    fprintf(fp, "\nlight 1.0 ambient\n");
    fprintf(fp, "light 1.0 directional %f %f %f\n", drawparam.light_dir[0],
            drawparam.light_dir[1], drawparam.light_dir[2]);
  } else {
    float ambient[3] = {0.0, 0.0, 0.0};

    for (li = 0; li < gl_numlights; li++) {
      ambient[0] += gl_lights[li].ambient[0];
      ambient[1] += gl_lights[li].ambient[1];
      ambient[2] += gl_lights[li].ambient[2];
    }

    fprintf(fp, "\nlight %g %g %g ambient\n", ambient[0], ambient[1],
            ambient[2]);

    for (li = 0; li < gl_numlights; li++) {
      // MC - Nov. 2015 - the light position is transformed by modelview
      glGetLightfv(GL_LIGHT0 + li, GL_POSITION, gl_light_pos);

      fprintf(fp, "light %g %g %g ", gl_lights[li].diffuse[0],
              gl_lights[li].diffuse[1], gl_lights[li].diffuse[2]);

      if (gl_lights[li].spot_cutoff == 180.0) {
        if (gl_lights[li].position[3] == 0) {
          fprintf(fp, "directional %g %g %g\n",
                  gl_light_pos[0],  // gl_lights[li].position[0],
                  gl_light_pos[1],  // gl_lights[li].position[1],
                  gl_light_pos[2]); // gl_lights[li].position[2] );
        } else {
          /* point light - position[3] is always 1.0 !*/
          fprintf(fp, "point %g %g %g\n",
                  gl_light_pos[0],  // gl_lights[li].position[0],
                  gl_light_pos[1],  // gl_lights[li].position[1],
                  gl_light_pos[2]); // gl_lights[li].position[2] );
        }
      } else {
        glGetLightfv(GL_LIGHT0 + li, GL_SPOT_DIRECTION, gl_light_dir);
        /* spot light */
        fprintf(fp, "spot %g %g %g %g %g %g %g %g %g\n",
                /* position */
                gl_light_pos[0], // gl_lights[li].position[0],
                gl_light_pos[1], // gl_lights[li].position[1],
                gl_light_pos[2], // gl_lights[li].position[2],
                /* at */
                gl_light_pos[0] +
                    gl_light_dir[0], /*gl_lights[li].position[0] +
                                        gl_lights[li].spot_direction[0],*/
                gl_light_pos[1] +
                    gl_light_dir[1], /*gl_lights[li].position[1] +
                                        gl_lights[li].spot_direction[1],*/
                gl_light_pos[2] +
                    gl_light_dir[2], /*gl_lights[li].position[2] +
                                        gl_lights[li].spot_direction[2],*/
                /* coef, thetain = thetaout */
                gl_lights[li].spot_exponent, gl_lights[li].spot_cutoff,
                gl_lights[li].spot_cutoff);
      }
    }
  }

  fprintf(fp, "#endif\n");
}

static void r_trailer(FILE *fp, VIEWPARAM *vw, DRAWPARAM *dr) {
  float volumeCentre[3];
  // Matrix rotation;
  Matrix transform;
  int i, mode;

  mode = QueryWorkMode();

  for (i = 0; i <= 2; i++) {
    volumeCentre[i] = (vw->min[i] + vw->max[i]) / 2.0;
  }

#ifndef NOGRAPHICS
  if (mode != NO_GRAPHICS_MODE && mode != PIPE_MODE) {
    // MC - Nov. 2015 - get the current modelview transformation
    // instead of recomputing it (otherwise we need to keep the code in sync)
    glGetFloatv(GL_MODELVIEW_MATRIX, &transform[0][0]);
  }

  if (dr->rayshade_scale != 1) {
    if (object_number == 1)
      fprintf(fp, "name %s object %s.orig ", object_name, object_name);
    else
      fprintf(fp, "name %s%d object %s%d.orig ", object_name, object_number,
              object_name, object_number);

    fprintf(fp, "scale %g %g %g\n\n", dr->rayshade_scale, dr->rayshade_scale,
            dr->rayshade_scale);
  }

  fprintf(fp, "#ifndef NOHEADERS\n");

  if (object_number == 1)
    fprintf(fp, "object %s\n", object_name);
  else
    fprintf(fp, "object %s%d\n", object_name, object_number);

  if (mode != NO_GRAPHICS_MODE && mode != PIPE_MODE) {
    // MC - Nov. 2014 - we need to send the transpose of OpenGL ModelView
    fprintf(fp, " transform\n");
    fprintf(fp, "  %g %g %g\n", transform[0][0], transform[0][1],
            transform[0][2]);
    fprintf(fp, "  %g %g %g\n", transform[1][0], transform[1][1],
            transform[1][2]);
    fprintf(fp, "  %g %g %g\n", transform[2][0], transform[2][1],
            transform[2][2]);

    fprintf(fp, " translate %g %g %g\n", transform[3][0], transform[3][1],
            transform[3][2]);
  }

  fprintf(fp, "#endif\n");
#endif /* NOGRAPHICS */
}

/**********************************************************************/
void PrintTransformation(FILE *fp, TURTLE *tu) {
  float angle;

  if (tu->left[eX] != 1 || tu->left[eY] != 0 || tu->left[eZ] != 0 ||
      tu->heading[eX] != 0 || tu->heading[eY] != 1 || tu->heading[eZ] != 0 ||
      tu->up[eX] != 0 || tu->up[eY] != 0 || tu->up[eZ] != 1) {

    if (tu->left[eX] == 1 && tu->left[eY] == 0 && tu->left[eZ] == 0) {
      /* rotation around left vector */
      angle = asin(tu->heading[eZ]);
      if (tu->heading[eX] < 0)
        angle = M_PI - angle;
      fprintf(fp, " rotate 1 0 0 %g\n", angle * 180 / M_PI);
    } else if (tu->heading[eX] == 0 && tu->heading[eY] == 1 &&
               tu->heading[eZ] == 0) {
      /* rotation around heading vector */
      angle = asin(tu->up[eX]);
      if (tu->up[eZ] < 0)
        angle = M_PI - angle;
      fprintf(fp, " rotate 0 1 0 %g\n", angle * 180 / M_PI);
    } else if (tu->up[eX] == 0 && tu->up[eY] == 0 && tu->up[eZ] == 1) {
      /* rotation around up vector */
      angle = asin(tu->heading[eX]);
      if (tu->heading[eZ] < 0)
        angle = M_PI - angle;

      fprintf(fp, " rotate 0 0 1 %g\n", -angle * 180 / M_PI);
    } else {
      fprintf(fp, " transform\n");
      fprintf(fp, "  %g %g %g", tu->left[eX], tu->left[eY], tu->left[eZ]);
      fprintf(fp, "  %g %g %g", tu->heading[eX], tu->heading[eY],
              tu->heading[eZ]);
      fprintf(fp, "  %g %g %g\n", tu->up[eX], tu->up[eY], tu->up[eZ]);
    }
  }

  if (tu->position[eX] != 0 || tu->position[eY] != 0 || tu->position[eZ] != 0)
    fprintf(fp, " translate %g %g %g\n", tu->position[eX], tu->position[eY],
            tu->position[eZ]);
}

/* r_objects() is defined in patch.c to
output surfaces as a rayshade objects. Each time it is drawn by
the turtle, an instantiation of this object will be added to the file */

/* Instantiate an object/surface at the place specified by the turtle
        for the purpose of ray-tracing. */
static void r_object(FILE *fp, TURTLE *tu, char desired_surface, double sx,
                     double sy, double sz) {

  char tex;

  if (!output)
    return;

  r_use_surface(r_patch_color(tu->color_index));
  fprintf(fp, "applysurf ");
  print_surface_use(fp, r_patch_color(tu->color_index));

  tex = GetSurfaceTexture(desired_surface, tu);

  if (r_use_second_object(desired_surface) &&
      texture_type(tex) == TEXELS_PER_PATCH)
    fprintf(fp, "\nobject %c2\n", desired_surface);
  else
    fprintf(fp, "\nobject %c\n", desired_surface);

  fprintf(fp, " scale %g %g %g\n", sx, sy, sz);

  PrintTransformation(fp, tu);

  if (is_valid_texture_index(tex))
    FinishUpTexture(tex, MAP_UV, 0);
}

static void r_setup_surfaces(void) {
  int x;

  for (x = 0; x < 4096; usedSurface[x++] = 0)
    ;
}

void print_surface_use(FILE *fp, int color_index) {
  if (clp.ismaterialfile) {
    if (usedSurface[color_index] == 0) {
      fprintf(fp, "s%03d", default_material);
      return;
    }
  }
  fprintf(fp, "s%03d", color_index);
}

void r_use_surface(int n) {
  if (!clp.ismaterialfile)
    usedSurface[n] = 1;
}

/* try to make a good guess at a color index for a patch to pass to rayshade */

int r_patch_color(int n) {
  int hue;
  float intensity;
  int cindex;

  if (clp.ismaterialfile)
    return n;

  hue = (int)n / 64;

  intensity = (n - hue * 64.0) / 64.0;

  cindex = (int)(64.0 * intensity * 0.25);
  cindex += hue * 64 + clp.colormap;

  return cindex;
}

/********************************************************************/
static void PrintSurface(FILE *fp, int x) {
  short r, g, b;

  fprintf(fp, "surface s%03d", x);

  if (clp.ismaterialfile) {
    struct material_type *mat;

    my_getmaterial((Colorindex)x, &mat);

    fprintf(fp, " ambient %g %g %g\n", mat->ambient[0] * mat->ambient[3],
            mat->ambient[1] * mat->ambient[3],
            mat->ambient[2] * mat->ambient[3]);
    fprintf(fp, "             diffuse %g %g %g\n",
            mat->diffuse[0] * mat->diffuse[3],
            mat->diffuse[1] * mat->diffuse[3],
            mat->diffuse[2] * mat->diffuse[3]);
    fprintf(fp, "             specular %g %g %g\n",
            mat->specular[0] * mat->specular[3],
            mat->specular[1] * mat->specular[3],
            mat->specular[2] * mat->specular[3]);
    fprintf(fp, "             specpow %g\n", mat->shininess);
    /* emissive ignored - but the fourth item used for transparency */
    if (mat->diffuse[3] < 1.0) {
      fprintf(fp, "             transp %g\n", 1 - mat->emissive[3]);
      fprintf(fp, "             index 1.0\n");
    } else {
      fprintf(fp, "             transp 0.007\n");
      fprintf(fp, "             index 1.0\n");
    }
  } else {
    my_getmcolor((Colorindex)x, &r, &g, &b);
    fprintf(fp, " ambient %g %g %g\n", r / 255.0, g / 255.0, b / 255.0);
    fprintf(fp, "             diffuse %g %g %g\n", r / 255.0, g / 255.0,
            b / 255.0);
  }
}

static void r_finishup_surfaces(FILE *fp) {
  int x;

  fprintf(fp, "#ifndef NOSURFACES\n");
  for (x = 0; x < 4096; x++) {
    if (usedSurface[x]) {
      PrintSurface(fp, x);
    }
  }
  fprintf(fp, "#endif\n");
}

/********************************************************************/
/* Puts appropriate routines in the dispatch table and makes other  */
/* settings depending on drawing and viewing parameters, such as    */
/* the drawing parameter shade mode.                                */
/********************************************************************/
turtleDrawDispatcher *rsSetDispatcher(DRAWPARAM *dr,
                                      VIEWPARAM *vw) {
  return (&rayshadeDrawRoutines);
}

/********************************************************************/
/********************************************************************/
/********************************************************************/

char *current_name = NULL;

int rsSetPass(int pass_no) {
  pass = pass_no;
  return 0;
}

/********************************************************************/
char *NameFromString(char *string) {
  char *name;
  char buffer[4096];
  int length, i;
  StringModule module;
  extern VIEWPARAM viewparam;

  length = 0;

  while (*string != 0 && NextStringModule(&string, &module) != 0) {
    if (isalpha(module.symbol))
      buffer[length++] = module.symbol;
    else
      /* the code of the module */
      length += sprintf(buffer + length, "c%d", (int)module.symbol);

    if (module.parameters > 0) {
      buffer[length++] = '_';
      for (i = 0; i < module.parameters; i++) {
        length += sprintf(buffer + length, viewparam.objects_format,
                          module.actual[i].value);
        if (i != module.parameters - 1)
          buffer[length++] = '_';
      }
      buffer[length++] = '_';
    }
  }

  if ((name = Malloc(length + 1)) == NULL) {
    Message("Not enough memory for an object name.\n");
    MyExit(-1);
  }

  for (i = 0; i < length; i++)
    name[i] = buffer[i];

  name[length] = 0;

  return name;
}

/********************************************************************/
short GetObjectName(char **name, char *curPtr, Production *prodPtr) {
  int length, i;
  StringModule module;
  Module *ptr;
  char *last = curPtr;

  length = 0;
  ptr = prodPtr->pred;

  while (ptr != NULL) {
    NextStringModule(&last, &module);

    ptr = ptr->nextModule;
  }

  length = last - curPtr;

  if ((*name = Malloc(length + 1)) == NULL) {
    Message("Not enough memory for an object name.\n");
    MyExit(-1);
  }

  for (i = 0; i < length; i++)
    (*name)[i] = curPtr[i];

#ifdef XXX
  ptr = prodPtr->pred;
  last = curPtr;
  length = 0;

  while (ptr != NULL) {
    NextStringModule(&last, &module);

    (*name)[length++] = module.symbol;

    if (module.parameters > 0) {
      (*name)[length++] = '(';
      for (i = 0; i < module.parameters; i++) {
        /* reduce the precision of the values */
        sprintf(buffer, viewparam.objects_format, module.actual[i].value);
        value = atof(buffer);

        bcopy(&value, *name + length, PARAMSIZE);

        length += PARAMSIZE;

        if (i != module.parameters - 1)
          (*name)[length++] = ',';
      }
      (*name)[length++] = ')';
    }

    ptr = ptr->nextModule;
  }
#endif

  (*name)[length] = 0;

  return length;
}

int store_depth = -1;

/********************************************************************/
int rsStartObject(TURTLE *tu, int depth, char *curPtr, Production *prodPtr) {
  char *name, *ptr;
  int length, index;
  extern VIEWPARAM viewparam;

  length = GetObjectName(&name, curPtr, prodPtr);
  ptr = NameFromString(name);

  if (!output && pass == 1) {
    if (FindObjectName(name, length, ptr, depth, tu, 0)) {
      /* object found */
      Free(name);
      name = NULL;
      Free(ptr);
      ptr = NULL;
      return 1; /* do not proceed with the production */
    }
    Free(name);
    name = NULL;
    Free(ptr);
    ptr = NULL;
    return 0;
  } else {

    if (!output)
      return 0;

    if (strcmp(current_name, ptr) && (pass != 3 || depth >= 1)) {
      /* if different */
      /* instantiate the object */
      if (viewparam.objects_include_turtle) {
        if ((index = FindObjectIndex(ptr, tu)) < 0)
          Message("Warning! Index of an instantiated object not found! "
                  "Rayshade output will be corrupted!\n");
        else
          fprintf(tmpfp, "object %s_i%d\n", ptr, index);
      } else
        fprintf(tmpfp, "object %s\n", ptr);

      Free(ptr);
      ptr = NULL;
      /* compute transformation according to the turtle position and
      orientation */
      PrintTransformation(tmpfp, tu);

      Free(name);
      name = NULL;
      output = 0; /* has to go through geometry to set turtle properly */

      store_depth = depth;

      return 0;
    }

    Free(ptr);
    ptr = NULL;
    output = 1;

    return 0;
  }
}

/********************************************************************/
int rsEndObject(TURTLE *tu, int depth, char *curPtr, Production *prodPtr) {
  char *name, *ptr;
  int length;

  if (!output && pass == 1) {
    length = GetObjectName(&name, curPtr, prodPtr);
    ptr = NameFromString(name);

    FindObjectName(name, length, ptr, depth, tu, 1);
  }

  if (pass != 1 && output == 0 && depth == store_depth) {
    output = 1;
    store_depth = -1;
  }
  return 0;
}

/********************************************************************/
void rsReadViewCommand(char *input_line, VIEWPARAM *vw) {
  char *token;

  if ((token = strtok(input_line, " \t:\n")) == NULL)
    return;

  /* the first token is the format string */
  strcpy(vw->objects_format, token);

  if ((token = strtok(NULL, " \t:\n")) == NULL)
    return;

  if (!strcmp(token, "turtle")) {
    if ((token = strtok(NULL, " \t:\n")) == NULL)
      return;

    if (!strncmp(token, "considered", 10))
      vw->objects_include_turtle = 1;
  }
}

/********************************************************************/
void rsOutputObjects(void) {
  char *ptr, *curPtr, *name, c;
  char buffer[4096];
  StringModule module;
  int depth, i, index;
  int length;
  long file_pos;
  extern DRAWPARAM *dr;
  extern VIEWPARAM *vw;
  extern TURTLE *tu;
  extern LSYSDATA *currLsysPtr;
  extern LSYSDATA *LsystemList;

  buffer[0] = 0;

  output = 1;
  pass = 3;

  HashTablePassInitialize();

  InitializeTurtle(dr, tu); /* must be before GetHashTableItem */

  while ((length = GetHashTableItem(&name, &index, &depth, tu)) != 0) {
    if (current_name != NULL) {
      Free(current_name);
      current_name = NULL;
    }

    current_name = NameFromString(name);

    if (vw->objects_include_turtle) {
      fprintf(tmpfp, "name %s_i%d list\n", current_name, index + 1);

      VERBOSE("Outputing %s_i%d\n", current_name, index + 1);
    } else {
      fprintf(tmpfp, "name %s list\n", current_name);

      VERBOSE("Outputing %s\n", current_name);
    }

    for (i = 0; i < length; i++)
      buffer[1 + i] = name[i];

    buffer[2 + length] = 0; /* not checking the length!!! */

    curPtr = buffer + 1;
    ptr = buffer + 1;

    c = NextStringModule(&ptr, &module);

    currLsysPtr = LsystemList;

    file_pos = ftell(tmpfp);
#ifdef CONTEXT_SENSITIVE_HOMO
    InterpretHomomorphism(c, buffer + 1, &curPtr, buffer, ptr, depth,
                          InterpretSymbol);
#else
    InterpretHomomorphism(c, buffer + 1, &curPtr, depth, InterpretSymbol);
#endif
    rsEndNode(tu, dr, vw, c);

    if (file_pos == ftell(tmpfp)) {
      /* empty object - rayshade would crash on this */
      /* add a dummy object - a tiny transparent sphere */
      fprintf(tmpfp, "sphere transp 1 noshadow 0.0001 0 0 0\n");
    }

    fprintf(tmpfp, "end\n");

    InitializeTurtle(dr, tu); /* must be before GetHashTableItem */

    tu->left[0] = 1;
    tu->up[2] = 1;

    current_name[0] = 0;
  }

  fprintf(tmpfp, "\n");
}

/********************************************************************/
int rsInsertObject(char *name, float scale, TURTLE *tu,
                   VIEWPARAM *w,
                   DRAWPARAM *dr) {

  fprintf(tmpfp, "object %s\n", name);

  /* compute transformation according to the turtle position and
  orientation */
  PrintTransformation(tmpfp, tu);
  return 0;
}
