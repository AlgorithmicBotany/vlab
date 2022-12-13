/*
   Author: Radomir Mech, September 1995
   */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#ifdef VLAB_MACX
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif /* MAC */

#include "radiosity.h"
#include "comm_lib.h"
#include "scene3d.h"
#include "matrix.h"

struct syntax_item {
  int flag;
  char *keyword;
};

struct syntax_item syn[] = {
    {POLYGON, "polygon"},       {TRIANGLE, "triangle"},
    {MATERIAL, "material"},     {PUSHMATRIX, "pushmatrix"},
    {POPMATRIX, "popmatrix"},   {TRANSLATE, "translate"},
    {ROTATE, "rotate"},         {SCALE, "scale"},
    {MULTMATRIX, "multmatrix"}, {-1, NULL} /* must be the last one */
};

/** array of primitives **/
#define ARRAY_SIZE 500

struct array_item {
  primitive_type primitive_array[ARRAY_SIZE];
  int num_primitives;
  int index;
  struct array_item *next;
};
typedef struct array_item array_item;

/****** obstacle array ******/

array_item *obstacles = NULL;
array_item *last_obstacle_array = NULL;

/****** object array ******/
/* held separately, so it can be rebuilt from scratch if necessary */

array_item *objects = NULL;
array_item *last_object_array = NULL;

extern char verbose;

#define EPSILON 0.0001

/* local prototypes */
void DrawPrimitives(int run);
void SetPrimitiveCells(grid_type *grid, primitive_type *prim);

/*************************************************************************/
float DotProduct(float *vec1, float *vec2) {
  return vec1[X] * vec2[X] + vec1[Y] * vec2[Y] + vec1[Z] * vec2[Z];
}

/*************************************************************************/
void FreePrimitives(array_item *prim) {
  array_item *next;

  for (;;) {
    if (prim == NULL)
      return;
    next = prim->next;
    free(prim);

    prim = next;
  }
}

/*************************************************************************/
void AllocatePrimitives(array_item **prim, int index) {
  if ((*prim = (array_item *)malloc(sizeof(array_item))) == NULL) {
    fprintf(stderr, "Not enough memory for array of primitives!\n");
    exit(0);
  }
  (*prim)->num_primitives = 0;
  (*prim)->next = NULL;
  (*prim)->index = index;
}

/*************************************************************************/
void InitializeObjects(void) {
  FreePrimitives(objects);
  AllocatePrimitives(&objects, 0);
  last_object_array = objects;

  if (verbose)
    fprintf(stderr, "scene3d - objects initialized.\n");
}

/*************************************************************************/
void InitializeStructures(void) {
  FreePrimitives(obstacles);
  AllocatePrimitives(&obstacles, 0);
  last_obstacle_array = obstacles;

  FreePrimitives(objects);
  AllocatePrimitives(&objects, 0);
  last_object_array = objects;
}

/*************************************************************************/
/* reads floats from an input text file (up to the max_num) and returns the
   number of successfully read items */
int ReadFloats(FILE *fp, float *data, int max_num) {
  int i;

  for (i = 0; i < max_num; i++)
    if (fscanf(fp, "%f", &data[i]) != 1)
      return i;

  return i;
}

/*************************************************************************/
/* selects 4 points specifying a bounding box in the primitive space,
   transforms them into grid space and creates a bounding box around them.
   Is there an easy way to make it tighter? */
/*
  bbox[3+3];  bounding box (coordinates of left lower front corner (mins)
               and right uper back corner (maxs) in
               world coordinates - coordinates of the grid) */
/* also updates the global bounding sphere */
void DetermineBoundingBox(primitive_type *prim, float *bbox, grid_type *grid) {
  float pt[4][3]; /* bounding box in primitive/grid space */
  float size[3], vec[3];
  float coord;
  int i, j, x, y, z;
  int num_vertices = 0;
  float aux, rad;

  /* set 4 points specifying a bounding box in primitive space */
  switch (prim->flag) {
  case POLYGON:
    num_vertices = 4;

  case TRIANGLE:
    if (prim->flag == TRIANGLE)
      num_vertices = 3;

    /* for each coordinate */
    for (i = X; i <= Z; i++) {
      pt[0][i] = prim->data[0 * 3 + i];
      size[i] = fabs(prim->data[1 * 3 + i] - prim->data[0 * 3 + i]);

      for (j = 1; j <= num_vertices - 1; j++) {
        if (pt[0][i] > prim->data[j * 3 + i]) {
          pt[0][i] = prim->data[j * 3 + i];
        }

        if ((aux = fabs(prim->data[((j + 1) % num_vertices) * 3 + i] -
                        pt[0][i])) > size[i])
          size[i] = aux;
      }
    }

    /* test if inside the grid */
    for (j = 0; j <= num_vertices - 1; j++)
      for (i = X; i <= Z; i++) {
        if (prim->data[j * 3 + i] < grid->pos[i])
          fprintf(stderr,
                  "WARNING! %d-th coordinate of %d-th vertex is "
                  " below the grid interval!\n",
                  i + 1, j + 1);
        if (prim->data[j * 3 + i] > grid->pos[i] + grid->range[i])
          fprintf(stderr,
                  "WARNING! %d-th coordinate of %d-th vertex is "
                  " above the grid interval!\n",
                  i + 1, j + 1);
      }
    break;

  default:
    fprintf(stderr,
            "Cannot determine the bounding box of primitive of type %d\n",
            (int)prim->flag);
  }

  /* update the bounding sphere */
  for (i = 0; i < num_vertices; i++) {
    /* is it the first point? */
    if (grid->bsph_r < 0) {
      /* set the center to the point and radius to 0 */
      for (j = X; j <= Z; j++)
        grid->bsph_C[j] = prim->data[i * 3 + j];

      grid->bsph_r = 0;

      if (verbose)
        fprintf(stderr, "New bounding sphere: (%g,%g,%g) rad %g\n",
                grid->bsph_C[0], grid->bsph_C[1], grid->bsph_C[2],
                grid->bsph_r);
      continue;
    }

    for (j = X; j <= Z; j++)
      vec[j] = prim->data[i * 3 + j] - grid->bsph_C[j];

    if (verbose)
      fprintf(stderr, "vec= (%g,%g,%g)\n", vec[X], vec[Y], vec[Z]);

    /* is the point outside the bounding sphere */
    if ((aux = vec[X] * vec[X] + vec[Y] * vec[Y] + vec[Z] * vec[Z]) >
        grid->bsph_r * grid->bsph_r) {

      /* radius increase */
      rad = 0.5 * ((aux = sqrt((double)aux)) - grid->bsph_r);
      grid->bsph_r += rad;

      /* it is necessary to move the center towards the new point by
         the radius increase */

      for (j = X; j <= Z; j++)
        grid->bsph_C[j] += vec[j] / aux * rad;

      if (verbose)
        fprintf(stderr, "New bounding sphere: (%g,%g,%g) rad %g\n",
                grid->bsph_C[0], grid->bsph_C[1], grid->bsph_C[2],
                grid->bsph_r);
    } else if (verbose)
      fprintf(stderr, "The same bounding sphere\n");
  }

  if (verbose)
    fprintf(stderr,
            "Primitive's bbox in primitive's space: (%g,%g,%g) %gx%gx%g\n",
            pt[0][0], pt[0][1], pt[0][2], size[0], size[1], size[2]);

  /* make those 4 points */
  for (j = 0; j < 3; j++)
    for (i = 0; i < 3; i++)
      pt[1 + j][i] = pt[0][i] + (i == j ? size[j] : 0);

  /* get bounding box in grid space (aligned with axes x,y, and z) */

  /* initialize */
  for (i = 0; i < 3; i++) {
    bbox[i] = pt[0][i];     /* front lower left corner */
    bbox[i + 3] = pt[0][i]; /* back upper right corner */
  }

  for (j = 1; j < 4; j++)
    for (i = 0; i < 3; i++)
      pt[j][i] -= pt[0][i]; /* make a vector */

  /* check all 8 points of the bounding box transformed from primitive space */
  for (x = 0; x <= 1; x++)
    for (y = 0; y <= 1; y++)
      for (z = 0; z <= 1; z++)
        for (i = 0; i < 3; i++) {
          coord = pt[0][i] + x * pt[1][i] + y * pt[2][i] + z * pt[3][i];

          if (coord < bbox[i])
            bbox[i] = coord;

          if (coord > bbox[i + 3])
            bbox[i + 3] = coord;
        }

  /* bounding box set */
}

/*************************************************************************/
int SetPrimitive(unsigned char flag, int float_num, array_item **last_array,
                 float *data) {
  int i, c;
  primitive_type *ptr;
  array_item *prim;
  float vec1[3], vec2[3];

  prim = *last_array;

  /* check if enough room for new primitive */
  if (prim->num_primitives == ARRAY_SIZE) {
    /* add new array */
    AllocatePrimitives(&(prim->next), prim->index + 1);

    *last_array = prim->next;
    prim = prim->next;
  }

  if (float_num > 12) {
    fprintf(stderr, "Not enough memory for %d floats!\n", float_num);
    exit(0);
  }

  ptr = prim->primitive_array + prim->num_primitives;

  for (i = 0; i < float_num; i++)
    ptr->data[i] = data[i];

  /* get the polygon/triangle normal */
  for (c = X; c <= Z; c++) {
    vec2[c] = data[2 * 3 + c] - data[0 * 3 + c];
    vec1[c] = data[1 * 3 + c] - data[0 * 3 + c];
  }

  ptr->normal[0] = vec1[1] * vec2[2] - vec1[2] * vec2[1];
  ptr->normal[1] = vec1[2] * vec2[0] - vec1[0] * vec2[2];
  ptr->normal[2] = vec1[0] * vec2[1] - vec1[1] * vec2[0];

  Normalize(ptr->normal);

  /* find a plane into which the triangle and the two intersections may
     be projected without reducing the triangle to a line segment */
  for (c = Z; c >= X; c--)
    if (fabs(ptr->normal[c]) > 0.5) {
      /* project to plane [c]=0 */
      ptr->ci = c;
      break;
    }

  ptr->flag = flag;
  ptr->ray_signature = 0;

  for (i = 0; i < MAX_SPECTRUM_SAMPLES; i++)
    ptr->intensity[i] = 0;

  if (verbose) {
    fprintf(stderr, " %d parameter(s) read:", float_num);
    for (i = 0; i < float_num; i++)
      fprintf(stderr, " %g", ptr->data[i]);
    fprintf(stderr, "\n");
  }

  prim->num_primitives++;

  return 1;
}

/*************************************************************************/
int ReadPrimitive(FILE *fp, unsigned char flag, int float_num,
                  array_item **prim) {
  float local_data[20];
  float invmatrix[16];

  if (ReadFloats(fp, local_data, float_num) != float_num)
    return 0;

  GetMatrix(invmatrix);

  /* !!! transform all points */

  return SetPrimitive(flag, float_num, prim, local_data);
}

/*************************************************************************/
void MatrixFromHeader(CTURTLE *tu, float *invmatrix) {
  float left[3];
  int i;

  MakeUnitMatrix(invmatrix);

  CrossProduct(tu->heading, tu->up, left);
  for (i = 0; i < 3; i++) {
    invmatrix[access(0, i)] = tu->up[i];
    invmatrix[access(1, i)] = tu->heading[i];
    invmatrix[access(2, i)] = left[i];
  }

  for (i = 0; i < 3; i++)
    invmatrix[access(3, i)] = tu->position[i];
}

/*************************************************************************/
primitive_type *AddObject(grid_type *grid, CTURTLE *tu,
                          Cmodule_type *next_symbol, int *top_mat,
                          int *bottom_mat) {
  float invmatrix[16];
  int c, i, npars;
  float pars[24];
  float pt[4][3];
  primitive_type *prim;
  extern int spectrum_samples;

  switch (next_symbol->symbol[0]) {
  case 'T': /* triangle */
    if (next_symbol->num_params >= (npars = 2)) {
      /* triangle */
      MatrixFromHeader(tu, invmatrix);

      if (verbose == 2)
        PrintMatrix("Primitive's invmatrix", invmatrix);

      if (verbose)
        fprintf(stderr, "Field - adding triangle (%g, %g)\n",
                next_symbol->params[0].value, next_symbol->params[1].value);

      for (i = 0; i < 3; i++)
        for (c = X; c <= Z; c++)
          pt[i][c] = 0;

      /* Y axis is along heading */
      /* thus param[1] should go to Y coordinate */
      /* param[0] to Z coordinate */

      pt[0][Z] = -next_symbol->params[0].value;
      pt[1][Z] = next_symbol->params[0].value;
      pt[2][Y] = next_symbol->params[1].value;

      Transform3Point(pt[0], invmatrix, pars + 0);
      Transform3Point(pt[1], invmatrix, pars + 3);
      Transform3Point(pt[2], invmatrix, pars + 6);

      /* it is faster not to use inv matrix */
      SetPrimitive(TRIANGLE, 9, &last_object_array, pars);

      break;
    }

  case 'P': /* polygon */
    if (next_symbol->num_params >= (npars = 2)) {
      MatrixFromHeader(tu, invmatrix);

      if (verbose == 2)
        PrintMatrix("Primitive's invmatrix", invmatrix);

      if (verbose)
        fprintf(stderr, "Field - adding polygon (%g, %g)\n",
                next_symbol->params[0].value, next_symbol->params[1].value);

      for (i = 0; i < 4; i++)
        for (c = X; c <= Z; c++)
          pt[i][c] = 0;

      /* Y axis is along heading */
      /* thus param[1] should go to Y coordinate */
      /* param[0] to Z coordinate */

      pt[1][Z] = next_symbol->params[0].value;
      pt[3][Z] = -next_symbol->params[0].value;

      pt[1][Y] = pt[3][Y] = next_symbol->params[1].value / 2.0;
      pt[2][Y] = next_symbol->params[1].value;

      Transform3Point(pt[0], invmatrix, pars + 0);
      Transform3Point(pt[1], invmatrix, pars + 3);
      Transform3Point(pt[2], invmatrix, pars + 6);
      Transform3Point(pt[3], invmatrix, pars + 9);

      SetPrimitive(POLYGON, 12, &last_object_array, pars);
      break;
    }

    return NULL;

  default:
    return NULL;
  }

  prim = last_object_array->primitive_array +
         last_object_array->num_primitives - 1;

  *top_mat = *bottom_mat = -1;

  /* next two parameters specify material index (top and bottom). */
  if (next_symbol->num_params >= npars + 1) {
    *top_mat = next_symbol->params[npars].value - 1;

    if (next_symbol->num_params >= npars + 2)
      *bottom_mat = next_symbol->params[npars + 1].value - 1;
  }

  SetPrimitiveCells(grid, prim);

  return prim;
}

/*************************************************************************/
#define LINELEN 1024
static char line[LINELEN];

/*************************************************************************/

int ReadPrimitiveFile(char *name, int first_time) {
  FILE *fp;
  struct syntax_item *ptr;
  int n;
  float loc_data[20];
  char buffer[1024]; /* buffer for creating system call */
  char tmpfile[255]; /* temporary file name */

  if (name == NULL)
    return 0;

  /* Set up temporary file name and preprocess.  */
  strcpy(tmpfile, "/tmp/scene.XXXXXX");
  mkstemp(tmpfile);
  sprintf(buffer, "cc -E %s > %s", name, tmpfile);
  system(buffer);

  if ((fp = fopen(tmpfile, "r")) == NULL) {
    fprintf(stderr, "Cannot open preprocessed data file %s.\n", tmpfile);
    return 0;
  }

  unlink(tmpfile); /* unlink the temp file immediately */
                   /* UNIX will keep the file around until it is closed */
                   /* at which point it is removed */

  if (first_time) {
    /* initialization */
    InitializeStructures();
  }

  InitializeMatrixStack();

  /* read in the primitive file */
  while (!feof(fp)) {
    ptr = syn;

    /* get one line */
    if (fscanf(fp, "%s", line) != 1)
      break;

    while (ptr->flag != -1) {
      /* skip lines left by preprocessor */
      if (line[0] == '#') {
        fgets(line, LINELEN, fp);
        break;
      }

      if (strcmp(line, ptr->keyword) == 0) {
        /* check for material, update the current rotation matrix,
           ignore all flat primitives, and store the rest in  particular
           array (obstacles or object) */

        switch (ptr->flag) {
        case MATERIAL:
          n = ReadFloats(fp, loc_data, 17);

          if (verbose)
            fprintf(stderr, "Material read (%d parameters).\n", n);
          break;
        case POLYGON:
          if (verbose)
            fprintf(stderr, "Polygon:");
          if (!ReadPrimitive(fp, ptr->flag, 12, &last_obstacle_array))
            fprintf(stderr, "Error reading polygon.\n");
          break;
        case TRIANGLE:
          if (verbose)
            fprintf(stderr, "Triangle:");
          if (!ReadPrimitive(fp, ptr->flag, 9, &last_obstacle_array))
            fprintf(stderr, "Error reading triangle.\n");
          break;
        case PUSHMATRIX:
          PushMatrix();
          if (verbose)
            fprintf(stderr, "PushMatrix.\n");
          break;
        case POPMATRIX:
          PopMatrix();
          if (verbose)
            fprintf(stderr, "PopMatrix.\n");
          break;
        case TRANSLATE:
          if (ReadFloats(fp, loc_data, 3) == 3) {
            Translate(loc_data);
            if (verbose)
              fprintf(stderr, "Translate: %g %g %g.\n", loc_data[0],
                      loc_data[1], loc_data[2]);
          } else
            fprintf(stderr, "Warning: translate needs three parameters!\n");
          break;
        case ROTATE:
          if (ReadFloats(fp, loc_data, 4) == 4) {
            Rotate(loc_data);
            if (verbose)
              fprintf(stderr, "Rotate: %g %g %g %g.\n", loc_data[0],
                      loc_data[1], loc_data[2], loc_data[3]);
          } else
            fprintf(stderr, "Warning: rotate needs four parameters!\n");
          break;
        case SCALE:
          if (ReadFloats(fp, loc_data, 3) == 3) {
            Scale(loc_data);
            if (verbose)
              fprintf(stderr, "Scale:%g %g %g.\n", loc_data[0], loc_data[1],
                      loc_data[2]);
          } else
            fprintf(stderr, "Warning: scale needs three parameters!\n");
          break;
        case MULTMATRIX:
          if (ReadFloats(fp, loc_data, 16) == 16) {
            MultMatrix(loc_data);
            if (verbose)
              fprintf(stderr, "MultMatrix.\n");
          } else
            fprintf(stderr, "Warning: multmatrix needs 16 parameters!\n");
          break;

        default:
          fprintf(stderr, "Warning: unknown primitive.\n");
        }
        break; /* out of the loop going through all keywords */
      }
      ptr++;
    }

    if (ptr->flag == -1)
      fprintf(stderr, "Uknown keyword: %s.\n", line);
  }

  fclose(fp);

  if (verbose)
    fprintf(stderr, "3D primitive file read.\n");

  return 1;
}

/*************************************************************************/
int Inside2DPolygon(float pt[2], float vert[][2]) {
  int signA, signB;

  signA =
      ((vert[1][Y] - vert[0][Y]) * pt[X] + (vert[0][X] - vert[1][X]) * pt[Y] +
       (vert[1][X] * vert[0][Y] - vert[1][Y] * vert[0][X])) >= 0
          ? 1
          : -1;

  signB =
      ((vert[2][Y] - vert[1][Y]) * pt[X] + (vert[1][X] - vert[2][X]) * pt[Y] +
       (vert[2][X] * vert[1][Y] - vert[2][Y] * vert[1][X])) >= 0
          ? 1
          : -1;

  if (signA != signB)
    return 0;

  signA =
      ((vert[3][Y] - vert[2][Y]) * pt[X] + (vert[2][X] - vert[3][X]) * pt[Y] +
       (vert[3][X] * vert[2][Y] - vert[3][Y] * vert[2][X])) >= 0
          ? 1
          : -1;

  if (signA != signB)
    return 0;

  signB =
      ((vert[0][Y] - vert[3][Y]) * pt[X] + (vert[3][X] - vert[0][X]) * pt[Y] +
       (vert[0][X] * vert[3][Y] - vert[0][Y] * vert[3][X])) >= 0
          ? 1
          : -1;

  if (signA != signB)
    return 0;

  return 1;
}

/*************************************************************************/
int Inside2DTriangle(float pt[2], float vert[][2]) {
  int sign, prev_sign;

  /* test whether pt is inside the 2d triangle */
  prev_sign =
      ((vert[1][Y] - vert[0][Y]) * pt[X] + (vert[0][X] - vert[1][X]) * pt[Y] +
       (vert[1][X] * vert[0][Y] - vert[1][Y] * vert[0][X])) >= 0
          ? 1
          : -1;

  sign =
      ((vert[2][Y] - vert[1][Y]) * pt[X] + (vert[1][X] - vert[2][X]) * pt[Y] +
       (vert[2][X] * vert[1][Y] - vert[2][Y] * vert[1][X])) >= 0
          ? 1
          : -1;

  if (sign != prev_sign)
    return 0;

  prev_sign =
      ((vert[0][Y] - vert[2][Y]) * pt[X] + (vert[2][X] - vert[0][X]) * pt[Y] +
       (vert[0][X] * vert[2][Y] - vert[0][Y] * vert[2][X])) >= 0
          ? 1
          : -1;

  if (sign != prev_sign)
    return 0;

  return 1;
}

/*************************************************************************/
/* returns <0, when the object specified by *prim and corresponding array
   of floats *float_array, is not intersected by the ray.
   Otherwise returns the distance to the intersection and when this
   distance is less that the given mindist, normal in the intersection is
   computed as well */
float IsIntersection(float *pt, float *dir, float mindist, primitive_type *prim,
                     float *norm) {
  float len;
  int i, j;
  float I[3];
  float vert[4][2]; /* 2D vertices */
  float dir_dot_T, d;

  if (prim == NULL)
    return -1;

  len = -1;

  switch (prim->flag) {
  case TRIANGLE: {
    if ((dir_dot_T = dir[X] * prim->normal[X] + dir[Y] * prim->normal[Y] +
                     dir[Z] * prim->normal[Z]) == 0)
      return -1;

    /* if the plane equation is nx*x+ny*y+nz*z+d=0, get d for the plane
       of the triangle (use the first triangle vertex). */
    d = -prim->normal[X] * prim->data[X] - prim->normal[Y] * prim->data[Y] -
        prim->normal[Z] * prim->data[Z];

    /* if the line equation is ptO+t*dir0, get t for the intersection:
       (ptO+t*dir0)Tnorm + d = 0 */
    len = (-d - pt[X] * prim->normal[X] - pt[Y] * prim->normal[Y] -
           pt[Z] * prim->normal[Z]) /
          dir_dot_T;

    if (len > mindist || len < 0)
      /* the  intersection is too close or too far */
      return -1;

    switch (prim->ci) {
    case X:
      /* the intersection */
      I[0] = pt[Y] + len * dir[Y];
      I[1] = pt[Z] + len * dir[Z];

      /* triangle vertices */
      for (i = j = 0; i < 3; i++, j += 3) {
        vert[i][0] = prim->data[j + Y];
        vert[i][1] = prim->data[j + Z];
      }
      break;

    case Y:
      /* the intersection */
      I[0] = pt[Z] + len * dir[Z];
      I[1] = pt[X] + len * dir[X];

      /* triangle vertices */
      for (i = j = 0; i < 3; i++, j += 3) {
        vert[i][0] = prim->data[j + Z];
        vert[i][1] = prim->data[j + X];
      }
      break;

    case Z:
      /* the intersection */
      I[0] = pt[X] + len * dir[X];
      I[1] = pt[Y] + len * dir[Y];

      /* triangle vertices */
      for (i = j = 0; i < 3; i++, j += 3) {
        vert[i][0] = prim->data[j + X];
        vert[i][1] = prim->data[j + Y];
      }
      break;
    }

    /* check whether one of the intersections is inside the triangle
     */
    if (!Inside2DTriangle(I, vert))
      return -1;

    norm[X] = prim->normal[X];
    norm[Y] = prim->normal[Y];
    norm[Z] = prim->normal[Z];

    break;
  }

  case POLYGON: {

    if ((dir_dot_T = dir[X] * prim->normal[X] + dir[Y] * prim->normal[Y] +
                     dir[Z] * prim->normal[Z]) == 0)
      return -1;

    /* if the plane equation is nx*x+ny*y+nz*z+d=0, get d for the plane
       of the triangle (use the first triangle vertex). */
    d = -prim->normal[X] * prim->data[X] - prim->normal[Y] * prim->data[Y] -
        prim->normal[Z] * prim->data[Z];

    /* if the line equation is ptO+t*dir0, get t for the intersection:
       (ptO+t*dir0)Tnorm + d = 0 */
    len = (-d - pt[X] * prim->normal[X] - pt[Y] * prim->normal[Y] -
           pt[Z] * prim->normal[Z]) /
          dir_dot_T;

    if (len > mindist || len < 0)
      /* the  intersection is too close or too far */
      return -1;

    switch (prim->ci) {
    case X:
      /* the intersection */
      I[0] = pt[Y] + len * dir[Y];
      I[1] = pt[Z] + len * dir[Z];

      /* triangle vertices */
      for (i = j = 0; i < 4; i++, j += 3) {
        vert[i][0] = prim->data[j + Y];
        vert[i][1] = prim->data[j + Z];
      }
      break;

    case Y:
      /* the intersection */
      I[0] = pt[Z] + len * dir[Z];
      I[1] = pt[X] + len * dir[X];

      /* triangle vertices */
      for (i = j = 0; i < 4; i++, j += 3) {
        vert[i][0] = prim->data[j + Z];
        vert[i][1] = prim->data[j + X];
      }
      break;

    case Z:
      /* the intersection */
      I[0] = pt[X] + len * dir[X];
      I[1] = pt[Y] + len * dir[Y];

      /* triangle vertices */
      for (i = j = 0; i < 4; i++, j += 3) {
        vert[i][0] = prim->data[j + X];
        vert[i][1] = prim->data[j + Y];
      }
      break;
    }

    /* check whether one of the intersections is inside the triangle
     */
    if (!Inside2DPolygon(I, vert))
      return -1;

    norm[X] = prim->normal[X];
    norm[Y] = prim->normal[Y];
    norm[Z] = prim->normal[Z];

    break;
  }

  default:
    return -1;
  }

  if (len <= 0)
    return -1;

#ifdef FULL_VERBOSE
  if (verbose == 2) {
    fprintf(stderr, "Scene3d - intersection found.\n");
    fprintf(stderr, "Scene3d - returned len = %g, mindist=%g\n", len, mindist);
  }
#endif

  return len;
}

/****************************************************************************/
/* adds a new item to the beginning of a linked list */
void AddToList(OBJECT_LIST_TYPE **first, primitive_type *prim) {
  OBJECT_LIST_TYPE *ptr;

  if ((ptr = (OBJECT_LIST_TYPE *)malloc(sizeof(OBJECT_LIST_TYPE))) == NULL) {
    fprintf(stderr, "Scene3d - cannot allocate memory for leaf item!\n");
    exit(0);
  }

  ptr->next = (*first);
  ptr->prim = prim;
  *first = ptr;
}

/*************************************************************************/
/* Add primitive to cells iside its bounding box
   SHOULD BE TIGHTER!  */
void SetPrimitiveCells(grid_type *grid, primitive_type *prim) {
  int c, x, y, z;
  float bbox[3 + 3];
  int range[3][2];
  CELL_TYPE *cell;

  DetermineBoundingBox(prim, bbox, grid);

  if (verbose)
    fprintf(stderr, "Primitive bbox: (%g,%g,%g) to (%g,%g,%g)\n", bbox[0],
            bbox[1], bbox[2], bbox[3], bbox[4], bbox[5]);

  /* set margins of the grid - used to limit the number of rays shot */
  for (c = X; c <= Z; c++) {
    if (grid->bbox[c] > bbox[c])
      grid->bbox[c] = bbox[c];

    if (grid->bbox[c + 3] < bbox[3 + c])
      grid->bbox[3 + c] = bbox[3 + c];
  }

  for (c = X; c <= Z; c++) {
    range[c][0] = floor((bbox[c] - 0.0001 - grid->pos[c]) / grid->range[c] *
                        (float)grid->size[c]);
    if (range[c][0] < 0)
      range[c][0] = 0;

    range[c][1] = ceil((bbox[c + 3] + 0.0001 - grid->pos[c]) / grid->range[c] *
                       (float)grid->size[c]);
    if (range[c][1] >= grid->size[c])
      range[c][1] = grid->size[c] - 1;

  }

  if (verbose)
    fprintf(stderr, "Primitive range: x:%d-%d; y:%d-%d; z:%d-%d\n", range[X][0],
            range[X][1], range[Y][0], range[Y][1], range[Z][0], range[Z][1]);

  /* for all nodes in the range */
  for (z = range[Z][0]; z <= range[Z][1]; z++)
    for (y = range[Y][0]; y <= range[Y][1]; y++)
      for (x = range[X][0]; x <= range[X][1]; x++) {
        cell = grid->data + z * grid->size[X] * grid->size[Y] +
               y * grid->size[X] + x;

        AddToList(&(cell->list), prim);
      }
}

/*************************************************************************/
/* fill grid according to the presence of obstacles  */
void FillGrid(grid_type *grid) {
  int obs, i;
  array_item *ptr;

  if (verbose)
    fprintf(stderr, "Start filling the grid.\n");

  /* initialize all nodes */
  for (i = grid->size[X] * grid->size[Y] * grid->size[Z] - 1; i >= 0; i--)
    grid->data[i].list = NULL;

  /* for all obstacles */

  ptr = obstacles;
  while (ptr != NULL) {
    for (obs = 0; obs < ptr->num_primitives; obs++)
      SetPrimitiveCells(grid, ptr->primitive_array + obs);

    ptr = ptr->next;
  }

  if (verbose)
    fprintf(stderr, "Grid filled.\n");
}
