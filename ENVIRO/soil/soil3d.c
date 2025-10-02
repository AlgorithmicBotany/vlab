/*
   Author: Radomir Mech, August 1995
   */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef WIN32
#include <unistd.h>
#endif

#include "soil2d.h"
#include "soil3d.h"
#include "matrix.h"
#include "triangulate.h"

struct syntax_item {
  int flag;
  char *keyword;
};

#define POLYGON 1
#define MESH 2
#define PRISM 3
#define CONE 4
#define SPHERE 5
#define MATERIAL 6
#define PUSHMATRIX 7
#define POPMATRIX 8
#define TRANSLATE 9
#define ROTATE 10
#define SCALE 11
#define MULTMATRIX 12
#define RECTANGLE 13

struct syntax_item syn[] = {
    {POLYGON, "polygon"},
    {RECTANGLE, "rectangle"},
    {MESH, "mesh"},
    {PRISM, "prism"},
    {CONE, "cone"},
    {SPHERE, "sphere"},
    {MATERIAL, "material"},
    {PUSHMATRIX, "pushmatrix"},
    {POPMATRIX, "popmatrix"},
    {TRANSLATE, "translate"},
    {ROTATE, "rotate"},
    {SCALE, "scale"},
    {MULTMATRIX, "multmatrix"},
    {-1, NULL} /* must be the last one */
};

struct primitive_type {
  int flag;
  float alpha;
  unsigned int data;   /* index in the array of floats */
  float matrix[16];    /* to convert point into primitive coordinates */
  float invmatrix[16]; /* to convert back (e.g. normals) */
  float bbox[3 + 3];   /* bounding box (coordinates of left lower front
                          corner plus lenghts along axes x,y, and z in
                          world coordinates - coordinates of the grid) */
};
typedef struct primitive_type primitive_type;

/** array of primitives **/
struct array_item {
  primitive_type *primitive_array;
  int primitive_array_size;
  int num_primitives;

  float *float_array; /* pertinent array of floats */
  int float_array_size;
  int num_floats;
};
typedef struct array_item array_item;

/****** obstacle array ******/

#define OBSTACLE_ARRAY_SIZE 250
#define OBSTACLE_FLOAT_SIZE (OBSTACLE_ARRAY_SIZE * 12)
/* as if there were only prisms with 12 floats per primitive */

array_item obstacles = {0};

/****** concentration array ******/
/* held separately, so it can be freed aftes grid nodes are set */

#define CONCENTRATION_ARRAY_SIZE 100
#define CONCENTRATION_FLOAT_SIZE (CONCENTRATION_ARRAY_SIZE * 12)
/* as if there were only prisms with 12 floats per primitive */

array_item concentrations = {0};

extern char verbose;
extern char output_type;
extern float concentration_contour_value;
extern float concentration_alpha;
extern char output_normals;

/* local prototypes */
void DrawPrimitives(int run);

/*************************************************************************/
void FreePrimitives(array_item *prim) {
  if (prim == NULL)
    return;

  if (prim->primitive_array != NULL)
    free(prim->primitive_array);
  prim->primitive_array = NULL;
  prim->num_primitives = 0;

  if (prim->float_array != NULL)
    free(prim->float_array);
  prim->float_array = NULL;
  prim->num_floats = 0;
}

/*************************************************************************/
void AllocatePrimitives(array_item *prim) {
  if ((prim->primitive_array = (primitive_type *)malloc(
           prim->primitive_array_size * sizeof(primitive_type))) == NULL) {
    fprintf(stderr, "Not enough memory for array of primitives!\n");
    exit(0);
  }

  if ((prim->float_array =
           (float *)malloc(prim->float_array_size * sizeof(float))) == NULL) {
    fprintf(stderr, "Not enough memory for array of floats!\n");
    exit(0);
  }
}

/*************************************************************************/
void InitializeStructures(void) {
  FreePrimitives(&obstacles);
  obstacles.primitive_array_size = OBSTACLE_ARRAY_SIZE;
  obstacles.float_array_size = OBSTACLE_FLOAT_SIZE;
  AllocatePrimitives(&obstacles);

  FreePrimitives(&concentrations);
  concentrations.primitive_array_size = CONCENTRATION_ARRAY_SIZE;
  concentrations.float_array_size = CONCENTRATION_FLOAT_SIZE;
  AllocatePrimitives(&concentrations);
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
void DetermineBoundingBox(primitive_type *prim, float *float_array) {
  float pt[4][4]; /* bounding box in primitive space */
  float size[3];
  float ptG[4][4]; /* bounding box in grid space */
  float coord;
  int i, j, x, y, z;
  float rad;

  /* set the fourth coordinate to 1.0 for all points */
  for (i = 0; i < 4; i++)
    pt[i][3] = 1.0;

  /* set 4 points specifying a bounding box in primitive space */
  switch (prim->flag) {
  case SPHERE:
    for (i = 0; i < 3; i++) {
      pt[0][i] = -float_array[prim->data];
      size[i] = 2 * float_array[prim->data];
    }
    break;

  case CONE:
    rad = float_array[prim->data] >= float_array[prim->data + 1]
              ? float_array[prim->data]
              : float_array[prim->data + 1];

    for (i = 0; i <= 2; i += 2)
      pt[0][i] = -rad;
    pt[0][1] = 0;

    size[0] = size[2] = 2 * rad;
    size[1] = float_array[prim->data + 2];
    break;

  case PRISM:
    for (i = 0; i < 3; i++) {
      pt[0][i] = 0;
      size[i] = float_array[prim->data + i];
    }
    break;
  }

  if (verbose)
    fprintf(stderr,
            "Primitive's bbox in primitive's space: (%g,%g,%g) %gx%gx%g\n",
            pt[0][0], pt[0][1], pt[0][2], size[0], size[1], size[2]);

  /* make those 4 points */
  for (j = 0; j < 3; j++)
    for (i = 0; i < 3; i++)
      pt[1 + j][i] = pt[0][i] + (i == j ? size[j] : 0);

  /* transform those 4 points to the grid space */
  for (i = 0; i < 4; i++) {
    TransformPoint(pt[i], prim->invmatrix, ptG[i]);

    if (ptG[i][3] != 1.0)
      if (ptG[i][3] != 0.0)
        for (j = 0; j < 3; j++)
          ptG[i][j] /= ptG[i][3];
  }

  /* get bounding box in grid space (aligned with axes x,y, and z) */

  /* initialize */
  for (i = 0; i < 3; i++) {
    prim->bbox[i] = ptG[0][i]; /* front lower left corner */
    prim->bbox[i + 3] = 0;     /* legth along axes x,y, and z */
  }

  for (j = 1; j < 4; j++)
    for (i = 0; i < 3; i++)
      ptG[j][i] -= ptG[0][i]; /* make a vector */

  /* check all 8 points of the bounding box transformed from primitive space */
  for (x = 0; x <= 1; x++)
    for (y = 0; y <= 1; y++)
      for (z = 0; z <= 1; z++)
        for (i = 0; i < 3; i++) {
          coord = ptG[0][i] + x * ptG[1][i] + y * ptG[2][i] + z * ptG[3][i];

          if (coord < prim->bbox[i]) {
            prim->bbox[i + 3] += prim->bbox[i] - coord;
            prim->bbox[i] = coord;
          }
          if (coord > prim->bbox[i] + prim->bbox[i + 3])
            prim->bbox[i + 3] += coord - (prim->bbox[i] + prim->bbox[i + 3]);
        }

  /* bounding box set */
}

/*************************************************************************/
int ReadPrimitive(FILE *fp, int flag, int float_num, array_item *prim,
                  float alpha) {
  int i;
  float mat[16];
  primitive_type *ptr;

  /* check if enough room for new primitive */
  if (prim->num_primitives == prim->primitive_array_size) {
    /* reallocate the array */
    prim->primitive_array_size *= 2;
    if ((prim->primitive_array = (primitive_type *)realloc(
             prim->primitive_array,
             prim->primitive_array_size * sizeof(primitive_type))) == NULL) {
      fprintf(stderr, "Not enough memory for array of primitives!\n");
      exit(0);
    }
  }

  if (prim->num_floats + float_num >= prim->float_array_size) {
    prim->float_array_size *= 2;
    if ((prim->float_array = (float *)realloc(
             prim->float_array, prim->float_array_size * sizeof(float))) ==
        NULL) {
      fprintf(stderr, "Not enough memory for array of floats!\n");
      exit(0);
    }
  }

  if (ReadFloats(fp, prim->float_array + prim->num_floats, float_num) !=
      float_num)
    return 0;

  ptr = prim->primitive_array + prim->num_primitives;

  GetMatrix(mat);
  for (i = 0; i < 16; i++)
    ptr->invmatrix[i] = mat[i];

  InverseMatrix(mat, ptr->matrix);

  ptr->flag = flag;
  ptr->alpha = alpha;
  ptr->data = prim->num_floats;

  if (verbose) {
    fprintf(stderr, " %d parameter(s) read:", float_num);
    for (i = 0; i < float_num; i++)
      fprintf(stderr, " %g", prim->float_array[ptr->data + i]);
    fprintf(stderr, "\n");
  }

  DetermineBoundingBox(ptr, prim->float_array);

  prim->num_primitives++;

  prim->num_floats += float_num;

  return 1;
}

/*************************************************************************/
#define LINELEN 1024
static char line[LINELEN];

/*************************************************************************/

int ReadPrimitiveFile(char *name, int first_time) {
  FILE *fp;
  struct syntax_item *ptr;
  int n;
  float alpha;
  float loc_data[20];
  char buffer[1024]; /* buffer for creating system call */
  char tmpfile[255]; /* temporary file name */
#ifdef WIN32
  const char *temp;
#endif

  if (name == NULL)
    return 0;

    /* Set up temporary file name and preprocess.  */
#ifdef WIN32
  strcpy(tmpfile, "scene.tmp");
  sprintf(buffer, "vlabcpp %s %s", name, tmpfile);
  system(buffer);
#else
  strcpy(tmpfile, "/tmp/scene.XXXXXX");
  mkstemp(tmpfile);
  //sprintf(buffer, "vlabcpp %s %s", name, tmpfile);
  //vlabcpp inserts a space between a minus sign and user-defined macro, which breaks the file parsing in arvo
  //so, for now, use preproc, which doesn't insert a space
  sprintf(buffer, "preproc %s > %s", name, tmpfile);
  system(buffer);
#endif

  if ((fp = fopen(tmpfile, "r")) == NULL) {
    fprintf(stderr, "Cannot open preprocessed data file %s.\n", tmpfile);
    return 0;
  }

#ifndef WIN32
  unlink(tmpfile); /* unlink the temp file immediately */
                   /* UNIX will keep the file around until it is closed */
                   /* at which point it is removed */
#endif

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
           array (obstacles or concentrations) */

        switch (ptr->flag) {
        case MATERIAL:
          n = ReadFloats(fp, loc_data, 17);

          if (verbose)
            fprintf(stderr, "Material read (%d parameters).\n", n);

          alpha = 1.0;
          if (n >= 8)
            alpha = loc_data[7];
          break;
        case POLYGON:
          fprintf(stderr, "Warning: mesh ignored.\n");
          break;
        case RECTANGLE:
          if (alpha != 1) /* only for concentration output */
            if (!ReadPrimitive(fp, ptr->flag, 2, &concentrations, alpha))
              fprintf(stderr, "Error reading rectangle.\n");
          break;
        case MESH:
          fprintf(stderr, "Warning: mesh ignored.\n");
          break;
        case SPHERE:
          if (verbose)
            fprintf(stderr, "Sphere:");
          if (!ReadPrimitive(fp, ptr->flag, 1,
                             alpha == 1 ? &obstacles : &concentrations, alpha))
            fprintf(stderr, "Error reading sphere.\n");
          break;
        case CONE:
          if (verbose)
            fprintf(stderr, "Cone:");
          if (!ReadPrimitive(fp, ptr->flag, 3,
                             alpha == 1 ? &obstacles : &concentrations, alpha))
            fprintf(stderr, "Error reading cone.\n");
          break;
        case PRISM:
          if (verbose)
            fprintf(stderr, "Prism:");
          if (!ReadPrimitive(fp, ptr->flag, 3,
                             alpha == 1 ? &obstacles : &concentrations, alpha))
            fprintf(stderr, "Error reading prism.\n");
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

#ifdef WIN32
  remove(tmpfile);
#endif

  if (verbose)
    fprintf(stderr, "3D primitive file read.\n");

  return 1;
}

/*************************************************************************/
int IsInside(primitive_type *prim, float *float_array, float *P) {
  float pt[4];
  float rad;
  int i;

  if (prim == NULL)
    return 0;

  TransformPoint(P, prim->matrix, pt);

  if (pt[3] != 1.0)
    if (pt[3] != 0.0)
      for (i = 0; i < 3; i++)
        pt[i] /= pt[3];

  switch (prim->flag) {
  case SPHERE:
    return ((pt[0] * pt[0] + pt[1] * pt[1] + pt[2] * pt[2]) <=
            float_array[prim->data] * float_array[prim->data]);

  case CONE:
    if ((pt[1] < 0) || (pt[1] > float_array[prim->data + 2]))
      return 0;

    rad = float_array[prim->data] +
          pt[1] / float_array[prim->data + 2] *
              (float_array[prim->data + 1] - float_array[prim->data]);

    return (pt[0] * pt[0] + pt[2] * pt[2] <= rad * rad);

  case PRISM:
    return ((pt[0] >= 0) && (pt[0] <= float_array[prim->data]) &&
            (pt[1] >= 0) && (pt[1] <= float_array[prim->data + 1]) &&
            (pt[2] >= 0) && (pt[2] <= float_array[prim->data + 2]));

  default:
    return 0;
  }
}

/*************************************************************************/
/* returns obstacle index */
int IsInsideO(short *index, float *P) {
  int i;

  for (i = 0; i < MAXOBSTACLESINCELL; i++)
    if (index[i] >= 0)
      if (IsInside(obstacles.primitive_array + index[i], obstacles.float_array,
                   P))
        return index[i];

  return -1;
}

/*************************************************************************/
/* Note: both P and norm must have size of 4 floats.
 */
void GetObstacleNormal(short index, float *P, float *norm) {
  primitive_type *prim;
  int i;
  float pt[4], len, x, d, y, dy, vec[4];
  float zero[4] = {0}, zeroT[4];
  float *float_array;

  if (index < 0)
    return;

  prim = obstacles.primitive_array + index;
  float_array = obstacles.float_array;

  TransformPoint(P, prim->matrix, pt);

  if (pt[3] != 1.0)
    if (pt[3] != 0.0)
      for (i = 0; i < 3; i++)
        pt[i] /= pt[3];

  vec[3] = 1.0;

  switch (prim->flag) {
  case SPHERE:
    for (i = 0; i < 3; i++)
      vec[i] = pt[i];
    break;

  case CONE:
    /* is the point closer to the top, base, or side? */
    len = pt[1];
    i = 0x01;

    if (float_array[prim->data + 2] - pt[1] < len) {
      len = float_array[prim->data + 2] - pt[1];
      i = 0x02;
    }

    d = (float_array[prim->data + 1] - float_array[prim->data]) /
        float_array[prim->data + 2];
    /* radius at height pt[1] minus the distance from the axis y */
    y = pt[1] * d + float_array[prim->data] -
        (dy = sqrt(pt[2] * pt[2] + pt[0] * pt[0]));

    /* distance from the cone side */
    x = y / sqrt(1 + d * d);

    if (x < len) {
      vec[0] = pt[0];
      vec[2] = pt[2];
      vec[1] = dy * d;
    } else {
      vec[0] = vec[2] = 0;
      vec[1] = i == 0x01 ? -1 : 1;
    }

    break;

  case PRISM:
    /* find the closest face assuming pt is in the object */
    len = pt[0];
    i = 0x01;

    if (float_array[prim->data] - pt[0] < len) {
      i = 0x02;
      len = float_array[prim->data] - pt[0];
    }
    if (pt[1] < len) {
      i = 0x04;
      len = pt[1];
    }
    if (float_array[prim->data + 1] - pt[1] < len) {
      i = 0x08;
      len = float_array[prim->data + 1] - pt[1];
    }
    if (pt[2] < len) {
      i = 0x10;
      len = pt[2];
    }
    if (float_array[prim->data + 2] - pt[2] < len) {
      i = 0x20;
      len = float_array[prim->data + 2] - pt[2];
    }

    /* determine the normal */
    vec[0] = vec[1] = vec[2] = 0;

    if (i & 0x03)
      vec[0] = i == 0x01 ? -1 : 1;

    if (i & 0x0c)
      vec[1] = i == 0x04 ? -1 : 1;

    if (i & 0x30)
      vec[2] = i == 0x10 ? -1 : 1;

    break;
  }

  /* transform the normal back to the world coordinates */
  TransformPoint(vec, prim->invmatrix, norm);

  zero[3] = 1.0;
  TransformPoint(zero, prim->invmatrix, zeroT);

  if (zeroT[3] != 1.0)
    if (zeroT[3] != 0.0)
      for (i = 0; i < 3; i++)
        zeroT[i] /= zeroT[3];

  if (norm[3] != 1.0)
    if (norm[3] != 0.0)
      for (i = 0; i < 3; i++)
        norm[i] /= norm[3];

  for (i = 0; i < 3; i++)
    norm[i] -= zeroT[i];

  /* normalize */
  len = sqrt(norm[0] * norm[0] + norm[1] * norm[1] + norm[2] * norm[2]);
  for (i = 0; i < 3; i++)
    norm[i] /= len;
}

/*************************************************************************/
/* fill grid according to the presence of obstacle and concentration
   primitives */
void FillGrid(grid_type *grid) {
  int x, y, z, i, j, a, b, c, num, obs, con;
  float pt[4], P[4], size[3];
  int range[3][2];
  primitive_type *prim;
  CELL_TYPE *cell;

  if (verbose)
    fprintf(stderr, "Start filling the grid.\n");

  pt[3] = P[3] = 1.0; /* always */

  for (i = X; i <= Z; i++)
    size[i] = grid->range[i] / (float)grid->size[i] / 2.0;

  /* initialize all nodes */
  for (z = 0; z < grid->size[Z]; z++) {
    pt[Z] =
        grid->pos[Z] + ((float)z + 0.5) / (float)grid->size[Z] * grid->range[Z];
    for (y = 0; y < grid->size[Y]; y++) {
      pt[Y] = grid->pos[Y] +
              ((float)y + 0.5) / (float)grid->size[Y] * grid->range[Y];
      for (x = 0; x < grid->size[X]; x++) {
        pt[X] = grid->pos[X] +
                ((float)x + 0.5) / (float)grid->size[X] * grid->range[X];

        cell = grid->data + z * grid->size[X] * grid->size[Y] +
               y * grid->size[X] + x;

        cell->flag = DATA;
        cell->val = grid->min;
        for (i = 0; i < MAXOBSTACLESINCELL; i++)
          cell->obstacle[i] = -1;
      }
    }
  }

  /* for all obstacles */
  for (obs = 0; obs < obstacles.num_primitives; obs++) {
    prim = obstacles.primitive_array + obs;

    if (verbose)
      fprintf(stderr, "Obstacle %d bbox: (%g,%g,%g) %gx%gx%g\n", obs,
              prim->bbox[0], prim->bbox[1], prim->bbox[2], prim->bbox[3],
              prim->bbox[4], prim->bbox[5]);

    for (c = X; c <= Z; c++) {
      range[c][0] = floor((prim->bbox[c] - grid->pos[c]) / grid->range[c] *
                          (float)grid->size[c]) -
                    1;
      if (range[c][0] < 0)
        range[c][0] = 0;

      range[c][1] =
          1 + floor((prim->bbox[c] + prim->bbox[c + 3] - grid->pos[c]) /
                    grid->range[c] * (float)grid->size[c]);
      if (range[c][1] >= grid->size[c])
        range[c][1] = grid->size[c] - 1;
    }

    if (verbose)
      fprintf(stderr, "Obstacle %d range: x:%d-%d; y:%d-%d; z:%d-%d\n", obs,
              range[X][0], range[X][1], range[Y][0], range[Y][1], range[Z][0],
              range[Z][1]);

    /* for all nodes in the range */
    for (z = range[Z][0]; z <= range[Z][1]; z++) {
      pt[Z] = grid->pos[Z] +
              ((float)z + 0.5) / (float)grid->size[Z] * grid->range[Z];

      for (y = range[Y][0]; y <= range[Y][1]; y++) {
        pt[Y] = grid->pos[Y] +
                ((float)y + 0.5) / (float)grid->size[Y] * grid->range[Y];

        for (x = range[X][0]; x <= range[X][1]; x++) {
          pt[X] = grid->pos[X] +
                  ((float)x + 0.5) / (float)grid->size[X] * grid->range[X];

          cell = grid->data + z * grid->size[X] * grid->size[Y] +
                 y * grid->size[X] + x;

          num = 0;

          /* check all 8 points - ineffective */
          for (a = -1; a < 2; a += 2)
            for (b = -1; b < 2; b += 2)
              for (c = -1; c < 2; c += 2) {
                P[0] = pt[0] + (float)a * size[0];
                P[1] = pt[1] + (float)b * size[1];
                P[2] = pt[2] + (float)c * size[2];

                if (IsInside(prim, obstacles.float_array, P))
                  num++;
              }

          if (num > 0) {
            /* find first index -1 */
            for (j = 0; j < MAXOBSTACLESINCELL; j++)
              if (cell->obstacle[j] == -1) {
                cell->obstacle[j] = obs;
                break;
              }
            if (j == MAXOBSTACLESINCELL)
              fprintf(stderr, "Warning: too many obstacles in one node.\n");
          }

          if (num == 8)
            cell->flag = OBSTACLE;
        }
      }
    }
  }

  for (con = 0; con < concentrations.num_primitives; con++) {
    prim = concentrations.primitive_array + con;

    if (verbose)
      fprintf(stderr, "Concentration %d bbox: (%g,%g,%g) %gx%gx%g\n", con,
              prim->bbox[0], prim->bbox[1], prim->bbox[2], prim->bbox[3],
              prim->bbox[4], prim->bbox[5]);

    for (c = X; c <= Z; c++) {
      range[c][0] = floor((prim->bbox[c] - grid->pos[c]) / grid->range[c] *
                          (float)grid->size[c]) -
                    1;
      if (range[c][0] < 0)
        range[c][0] = 0;

      range[c][1] =
          1 + floor((prim->bbox[c] + prim->bbox[c + 3] - grid->pos[c]) /
                    grid->range[c] * (float)grid->size[c]);
      if (range[c][1] >= grid->size[c])
        range[c][1] = grid->size[c] - 1;
    }

    if (verbose)
      fprintf(stderr, "Concentration %d range: x:%d-%d; y:%d-%d; z:%d-%d\n",
              con, range[X][0], range[X][1], range[Y][0], range[Y][1],
              range[Z][0], range[Z][1]);

    /* for all nodes in the range */
    for (z = range[Z][0]; z <= range[Z][1]; z++) {
      pt[Z] = grid->pos[Z] +
              ((float)z + 0.5) / (float)grid->size[Z] * grid->range[Z];

      for (y = range[Y][0]; y <= range[Y][1]; y++) {
        pt[Y] = grid->pos[Y] +
                ((float)y + 0.5) / (float)grid->size[Y] * grid->range[Y];

        for (x = range[X][0]; x <= range[X][1]; x++) {
          pt[X] = grid->pos[X] +
                  ((float)x + 0.5) / (float)grid->size[X] * grid->range[X];

          cell = grid->data + z * grid->size[X] * grid->size[Y] +
                 y * grid->size[X] + x;

          if (cell->flag != OBSTACLE)
            /* for all concentrations */
            if (IsInside(prim, concentrations.float_array, pt)) {
              if (prim->alpha == 0.0) {
                /* source */
                cell->flag = SOURCE;
                cell->val = 1.0;
              } else {
                /* data */
                cell->flag = DATA;
                cell->val += 1 - prim->alpha;
              }
            }
        }
      }
    }
  }

  if (verbose)
    fprintf(stderr, "Grid filled.\n");
}

/*************************************************************************/
void OutputRectangle(FILE *fp, float *p1, float *p2, float *p3, float *p4) {
  fprintf(fp, "polygon\n");
  fprintf(fp, "%.3g %.3g %.3g\n", p1[0], p1[1], p1[2]);
  fprintf(fp, "%.3g %.3g %.3g\n", p2[0], p2[1], p2[2]);
  fprintf(fp, "%.3g %.3g %.3g\n", p3[0], p3[1], p3[2]);
  fprintf(fp, "%.3g %.3g %.3g\n\n", p4[0], p4[1], p4[2]);
}

/*************************************************************************/
void OutputContours(FILE *fp, grid_type *grid, int sel_flag,
                    int (*Decide)(CELL_TYPE *, CELL_TYPE *)) {
  int x, y, z;
  int i[3], c;
  float pt[3], node_size[3], P[2][2][2][3];
  CELL_TYPE *which_node;
  char firstx, lastx, firsty, lasty, firstz, lastz;

  for (c = X; c <= Z; c++)
    node_size[c] = grid->range[c] / (float)grid->size[c];

  /* for all nodes */
  firstz = 1;

  for (z = 0; z < grid->size[Z]; z++) {
    pt[Z] = grid->pos[Z] + ((float)z) / (float)grid->size[Z] * grid->range[Z];
    lastz = z == grid->size[Z] - 1;
    firsty = 1;

    for (y = 0; y < grid->size[Y]; y++) {
      pt[Y] = grid->pos[Y] + ((float)y) / (float)grid->size[Y] * grid->range[Y];
      lasty = y == grid->size[Y] - 1;
      firstx = 1;

      for (x = 0; x < grid->size[X]; x++) {
        pt[X] =
            grid->pos[X] + ((float)x) / (float)grid->size[X] * grid->range[X];
        lastx = x == grid->size[X] - 1;

        which_node = grid->data + z * grid->size[X] * grid->size[Y] +
                     y * grid->size[X] + x;

        if (which_node->flag == sel_flag) {
          /* make the cube */
          for (i[0] = 0; i[0] < 2; i[0]++)
            for (i[1] = 0; i[1] < 2; i[1]++)
              for (i[2] = 0; i[2] < 2; i[2]++)
                for (c = 0; c < 3; c++)
                  P[i[0]][i[1]][i[2]][c] = pt[c] + i[c] * node_size[c];

          /* check all 6 neighbors */
          if (!firstz)
            if (Decide(which_node,
                       which_node - grid->size[X] * grid->size[Y])) {
              OutputRectangle(fp, P[0][0][0], P[0][1][0], P[1][1][0],
                              P[1][0][0]);
            }
          if (!lastz)
            if (Decide(which_node,
                       which_node + grid->size[X] * grid->size[Y])) {
              OutputRectangle(fp, P[0][0][1], P[1][0][1], P[1][1][1],
                              P[0][1][1]);
            }

          if (!firsty)
            if (Decide(which_node, which_node - grid->size[X])) {
              OutputRectangle(fp, P[0][0][0], P[0][0][1], P[1][0][1],
                              P[1][0][0]);
            }
          if (!lasty)
            if (Decide(which_node, which_node + grid->size[X])) {
              OutputRectangle(fp, P[0][1][0], P[1][1][0], P[1][1][1],
                              P[0][1][1]);
            }

          if (!firstx)
            if (Decide(which_node, which_node - 1)) {
              OutputRectangle(fp, P[0][0][0], P[0][0][1], P[0][1][1],
                              P[0][1][0]);
            }
          if (!lastx)
            if (Decide(which_node, which_node + 1)) {
              OutputRectangle(fp, P[1][0][0], P[1][1][0], P[1][1][1],
                              P[1][0][1]);
            }
        }
        firstx = 0;
      }
      firsty = 0;
    }
    firstz = 0;
  }
}

/*************************************************************************/
int DecideObstacle(CELL_TYPE *current,
                   CELL_TYPE *neighbor) {
  return neighbor->flag != OBSTACLE;
}

/*************************************************************************/
int DecideSource(CELL_TYPE *current,
                 CELL_TYPE *neighbor) {
  return neighbor->flag != SOURCE;
}

/*************************************************************************/
int DecideConcentration(CELL_TYPE *current, CELL_TYPE *neighbor) {
  if (neighbor->flag == DATA) {
    if (current->val >= concentration_contour_value)
      return neighbor->val < concentration_contour_value;
    else
      return neighbor->val >= concentration_contour_value;
  }
  return 0;
}

/*************************************************************************/
float FindConcentrationCountour(CELL_TYPE *current, CELL_TYPE *neighbor) {
  float ret;

  if (neighbor->flag != OBSTACLE)
    if (current->val != neighbor->val) {
      ret = (concentration_contour_value - current->val) /
            (neighbor->val - current->val);

      if ((ret > 0) && (ret <= 1))
        return ret;

      return -1;
    }

  return -1;
}

/*************************************************************************/
void SaveMatrix(FILE *fp, float *matrix) {
  int row, col;

  fprintf(fp, "multmatrix\n");
  for (row = 0; row < MAT_SIZE; row++) {
    for (col = 0; col < MAT_SIZE; col++)
      fprintf(fp, "%g ", matrix[access(row, col)]);
    fprintf(fp, "\n");
  }
}

/*************************************************************************/
void OutputSources(FILE *fp) {
  primitive_type *prim;
  float *data = concentrations.float_array;
  int i;

  prim = concentrations.primitive_array;

  for (i = 0; i < concentrations.num_primitives; i++) {
    if (prim->alpha == 0)
      switch (prim->flag) {
      case SPHERE:
        fprintf(fp, "pushmatrix\n");
        SaveMatrix(fp, prim->invmatrix);

        fprintf(fp, "sphere %g\n", data[prim->data]);

        fprintf(fp, "popmatrix\n");
        break;
      case CONE:
        fprintf(fp, "pushmatrix\n");
        SaveMatrix(fp, prim->invmatrix);

        fprintf(fp, "cone %g %g %g\n", data[prim->data], data[prim->data + 1],
                data[prim->data + 2]);

        fprintf(fp, "popmatrix\n");
        break;
      case PRISM:
        fprintf(fp, "pushmatrix\n");
        SaveMatrix(fp, prim->invmatrix);

        fprintf(fp, "prism %g %g %g\n", data[prim->data], data[prim->data + 1],
                data[prim->data + 2]);

        fprintf(fp, "popmatrix\n");
        break;
      }
    prim++;
  }
}

/*************************************************************************/
void OutputRectangleContours(FILE *fp, grid_type *grid) {
  primitive_type *prim;
  float *data = concentrations.float_array;
  float pt[4], ptG[4], val;
  int coord[3];
  int i, j, c;
  int x, y;

  prim = concentrations.primitive_array;

  for (i = 0; i < concentrations.num_primitives; i++) {
    switch (prim->flag) {
    case RECTANGLE:
      fprintf(fp, "pushmatrix\n");
      SaveMatrix(fp, prim->invmatrix);

      for (x = 0; x <= (int)data[prim->data] - 1; x++)
        for (y = 0; y <= (int)data[prim->data + 1] - 1; y++) {

          pt[0] = 0.5 + (float)x;
          pt[1] = 0.5 + (float)y;
          pt[2] = 0;
          pt[3] = 1;

          TransformPoint(pt, prim->invmatrix, ptG);

          if (ptG[3] != 1.0)
            if (ptG[3] != 0.0)
              for (j = 0; j < 3; j++)
                ptG[j] /= ptG[3];

          val = -1;
          for (c = X; c <= Z; c++) {
            coord[c] = floor((ptG[c] - grid->pos[c]) / grid->range[c] *
                             (float)grid->size[c]);

            if ((coord[c] < 0) || (coord[c] >= grid->size[c]))
              val = 0;
          }

          if (val == -1) {
            val = grid->data[coord[Z] * grid->size[Y] * grid->size[X] +
                             coord[Y] * grid->size[X] + coord[X]]
                      .val;

            val = (val - grid->min) / (grid->max - grid->min);
          }

          fprintf(fp, "rectangle 1 1 %d %d %.2g\n", x, y, val + 0.05);
        }

      fprintf(fp, "popmatrix\n");
      break;
    }
    prim++;
  }
}

/*************************************************************************/
void OutputPrimitives(grid_type *grid) {
  FILE *fp;
  float alpha;
  extern char *contour_mat;
  extern char *source_mat;
  extern char *section_mat;

  if ((fp = fopen(grid->out3Dname, "w")) == NULL) {
    fprintf(stderr, "Cannot open the primitive output file %s.\n",
            grid->out3Dname);
    return;
  }

  /* output sources */
  if (source_mat == NULL)
    fprintf(fp, "material\n0.3 0.3 0.3 1\n0.9 0.9 0.9 1\n"
                "0.7 0.7 0.7 1\n0 0 0 1\n10\n\n");
  else
    fprintf(fp, "material %s\n", source_mat);

  if (output_type == POLYGONS) {
    OutputContours(fp, grid, SOURCE, DecideSource);
  } else
    OutputSources(fp);

  if (concentration_contour_value > grid->min) {
    alpha = concentration_alpha;
    if (contour_mat == NULL)
      fprintf(fp,
              "material\n0.1 0.1 0.1 %g\n0.1 0.6 0 %g\n0.5 0.5 0.5 %g\n"
              "0 0 0 %g\n25\n\n",
              alpha, alpha, alpha, alpha);
    else
      fprintf(fp, "material %s\n", contour_mat);

    if (output_type == POLYGONS)
      OutputContours(fp, grid, DATA, DecideConcentration);
    else
      OutputTriangulatedContours(fp, grid, FindConcentrationCountour);
  }

  if (section_mat == NULL)
    fprintf(fp, "material\n0.0 0 0 1\n0 0 0 1\n0 0 0 1\n0.5 0.5 0 1\n0\n\n");
  else
    fprintf(fp, "material %s\n", section_mat);

  OutputRectangleContours(fp, grid);

  fclose(fp);
}
