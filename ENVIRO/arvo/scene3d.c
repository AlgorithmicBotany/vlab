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

#include "arvo.h"
#include "comm_lib.h"
#include "scene3d.h"
#include "matrix.h"

struct syntax_item {
  int flag;
  char *keyword;
};

struct syntax_item syn[] = {
    {POLYGON, "polygon"},
    {RECTANGLE, "rectangle"},
    {MESH, "mesh"},
    {PRISM, "prism"},
    {CONE, "cone"},
    {CYLINDER, "cylinder"},
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
    fprintf(stderr, "arvo - objects initialized.\n");
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
void DetermineBoundingBox(primitive_type *prim) {
  float pt[4][3]; /* bounding box in primitive space */
  float size[3];
  float ptG[4][3]; /* bounding box in grid space */
  float coord;
  int i, j, x, y, z;
  float rad, aux;

  /* set 4 points specifying a bounding box in primitive space */
  switch (prim->flag) {
  case SPHERE:
    for (i = 0; i < 3; i++) {
      pt[0][i] = -prim->data[0];
      size[i] = 2 * prim->data[0];
    }
    break;

  case CONE:
    rad = prim->data[0] >= prim->data[1] ? prim->data[0] : prim->data[1];

    for (i = 0; i <= 2; i += 2)
      pt[0][i] = -rad;
    pt[0][1] = 0;

    size[0] = size[2] = 2 * rad;
    size[1] = prim->data[2];
    break;

  case CYLINDER:
    rad = prim->data[0];

    for (i = 0; i <= 2; i += 2)
      pt[0][i] = -rad;
    pt[0][1] = 0;

    size[0] = size[2] = 2 * rad;
    size[1] = prim->data[1];
    break;

  case PRISM:
    for (i = 0; i < 3; i++) {
      pt[0][i] = 0;
      size[i] = prim->data[i];
    }
    break;

  case POLYGON:
    /* no more than 3 points */

    /* for each coordinate */
    for (i = X; i <= Z; i++) {
      pt[0][i] = prim->data[0 * 3 + i];
      size[i] = fabs(prim->data[1 * 3 + i] - prim->data[0 * 3 + i]);

      for (j = 1; j <= 2; j++) {
        if (pt[0][i] > prim->data[j * 3 + i]) {
          pt[0][i] = prim->data[j * 3 + i];
        }

        if ((aux = fabs(prim->data[((j + 1) % 3) * 3 + i] -
                        prim->data[j * 3 + i])) > size[i])
          size[i] = aux;
      }
    }
    break;

  case RECTANGLE:
    for (i = 0; i < 3; i++)
      pt[0][i] = 0;

    size[X] = prim->data[X];
    size[Y] = prim->data[Y];
    size[Z] = 0;
    break;
  }


  /* make those 4 points */
  for (j = 0; j < 3; j++)
    for (i = 0; i < 3; i++)
      pt[1 + j][i] = pt[0][i] + (i == j ? size[j] : 0);

  /* transform those 4 points to the grid space */
  for (i = 0; i < 4; i++)
    Transform3Point(pt[i], prim->invmatrix, ptG[i]);

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
int SetPrimitive(int flag, int float_num, array_item **last_array, float *data,
                 float *invmatrix) {
  int i;
  primitive_type *ptr;
  array_item *prim;

  prim = *last_array;

  /* check if enough room for new primitive */
  if (prim->num_primitives == ARRAY_SIZE) {
    /* add new array */
    AllocatePrimitives(&(prim->next), prim->index + 1);

    *last_array = prim->next;
    prim = prim->next;
  }

  if (float_num > 9) {
    fprintf(stderr, "Not enough memory for %d floats!\n", float_num);
    exit(0);
  }

  ptr = prim->primitive_array + prim->num_primitives;

  for (i = 0; i < float_num; i++)
    ptr->data[i] = data[i];

  for (i = 0; i < MAT_SIZE * MAT_SIZE; i++)
    ptr->invmatrix[i] = invmatrix[i];

  InverseMatrix(ptr->invmatrix, ptr->matrix);

  ptr->flag = flag;
  ptr->ray_signature = 0;

  if (last_array == &last_object_array)
    ptr->id = prim->index * ARRAY_SIZE + prim->num_primitives + 1;
  else
    ptr->id = 0;

  if (verbose) {
    fprintf(stderr, " %d parameter(s) read:", float_num);
    for (i = 0; i < float_num; i++)
      fprintf(stderr, " %g", ptr->data[i]);
    fprintf(stderr, "\n");
  }

  DetermineBoundingBox(ptr);

  prim->num_primitives++;

  return 1;
}

/*************************************************************************/
int ReadPrimitive(FILE *fp, int flag, int float_num, array_item **prim) {
  float local_data[20];
  float invmatrix[16];

  if (ReadFloats(fp, local_data, float_num) != float_num)
    return 0;

  GetMatrix(invmatrix);

  return SetPrimitive(flag, float_num, prim, local_data, invmatrix);
}

/*************************************************************************/
void MatrixFromHeader(CTURTLE *tu, float *invmatrix) {
  float up[3], left[3];   /* any possible turtle left and up vector */
  float y[3] = {0, 1, 0}; /* y vector */
  float len;
  int i;

  MakeUnitMatrix(invmatrix);

  CrossProduct(tu->heading, y, up);
  if ((len = sqrt(up[0] * up[0] + up[1] * up[1] + up[2] * up[2])) == 0) {
    invmatrix[access(1, 1)] = tu->heading[1]; /* just proper scaling */
  } else {
    /* normalize up */
    for (i = 0; i < 3; i++)
      up[i] /= len;

    CrossProduct(tu->heading, up, left);
    for (i = 0; i < 3; i++) {
      invmatrix[access(0, i)] = up[i];
      invmatrix[access(1, i)] = tu->heading[i];
      invmatrix[access(2, i)] = left[i];
    }
  }

  for (i = 0; i < 3; i++)
    invmatrix[access(3, i)] = tu->position[i];
}

/*************************************************************************/
int AddObject(grid_type *grid, CTURTLE *tu, Cmodule_type *next_symbol) {
  float invmatrix[16];
  int c;
  float pars[4];

  switch (next_symbol->symbol[0]) {
  case 'S': /* sphere */
    if (next_symbol->num_params == 1) {
      MakeUnitMatrix(invmatrix);

      for (c = X; c <= Z; c++)
        invmatrix[access(3, c)] = tu->position[c];

      if (verbose)
        fprintf(stderr, "Field - adding sphere (radius %g)\n",
                next_symbol->params[0].value);

      SetPrimitive(SPHERE, 1, &last_object_array, &next_symbol->params[0].value,
                   invmatrix);
      break;
    }
    return -1;

  case 'C': /* cylinder and cone */
    if (next_symbol->num_params == 2) {
      /* cylinder */
      MatrixFromHeader(tu, invmatrix);

      if (verbose)
        fprintf(stderr, "Field - adding cylinder (radius %g, height %g)\n",
                next_symbol->params[0].value, next_symbol->params[1].value);

      for (c = 0; c < 2; c++)
        pars[c] = next_symbol->params[c].value;

      SetPrimitive(CYLINDER, 2, &last_object_array, pars, invmatrix);
      break;
    }

    if (next_symbol->num_params == 3) {
      /* cone */
      MatrixFromHeader(tu, invmatrix);

      if (verbose)
        fprintf(stderr,
                "Field - adding cone (radius1 %g, radius2 %g, "
                "height %g)\n",
                next_symbol->params[0].value, next_symbol->params[1].value,
                next_symbol->params[1].value);

      for (c = 0; c < 3; c++)
        pars[c] = next_symbol->params[c].value;

      SetPrimitive(CONE, 3, &last_object_array, pars, invmatrix);
      break;
    }
    return -1;

  default:
    return -1;
  }

  SetPrimitiveCells(grid, last_object_array->primitive_array +
                              last_object_array->num_primitives - 1);

  return (last_object_array->index * ARRAY_SIZE +
          last_object_array->num_primitives);
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
  //sprintf(buffer, "cc -E %s > %s", name, tmpfile);
  sprintf(buffer, "preproc %s > %s", name, tmpfile);
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
            fprintf(stderr, "Polygon (triangle):");
          if (!ReadPrimitive(fp, ptr->flag, 9, &last_obstacle_array))
            fprintf(stderr, "Error reading polygon.\n");
          break;
        case RECTANGLE:
          if (verbose)
            fprintf(stderr, "Rectangle:");
          if (!ReadPrimitive(fp, ptr->flag, 2, &last_obstacle_array))
            fprintf(stderr, "Error reading rectangle.\n");
          break;
        case MESH:
          fprintf(stderr, "Warning: mesh ignored.\n");
          break;
        case SPHERE:
          if (verbose)
            fprintf(stderr, "Sphere:");
          if (!ReadPrimitive(fp, ptr->flag, 1, &last_obstacle_array))
            fprintf(stderr, "Error reading sphere.\n");
          break;
        case CYLINDER:
          if (verbose)
            fprintf(stderr, "Cylinder:");
          if (!ReadPrimitive(fp, ptr->flag, 2, &last_obstacle_array))
            fprintf(stderr, "Error reading cylinder.\n");
          break;
        case CONE:
          if (verbose)
            fprintf(stderr, "Cone:");
          if (!ReadPrimitive(fp, ptr->flag, 3, &last_obstacle_array))
            fprintf(stderr, "Error reading cone.\n");
          break;
        case PRISM:
          if (verbose)
            fprintf(stderr, "Prism:");
          if (!ReadPrimitive(fp, ptr->flag, 3, &last_obstacle_array))
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

  if (verbose)
    fprintf(stderr, "3D primitive file read.\n");

  return 1;
}

/*************************************************************************/
float GetFaceIntersection(float *ptO, float *dirO, float *data, int ind,
                          float mindist, float surf_dist, float *norm) {
  float c, aux;
  int i1, i2;

  if (fabs(dirO[ind]) < EPSILON)
    return -1;

  if (dirO[ind] > 0)
    aux = (-surf_dist - ptO[ind]) / dirO[ind];
  else
    aux = (data[ind] + surf_dist - ptO[ind]) / dirO[ind];

  if ((aux > 0) && (aux < mindist)) {
    i1 = (ind + 1) % 3;
    c = ptO[i1] + aux * dirO[i1];

    if ((c >= -surf_dist) && (c <= data[i1] + surf_dist)) {
      i2 = (ind + 2) % 3;
      c = ptO[i2] + aux * dirO[i2];

      if ((c >= -surf_dist) && (c <= data[i2] + surf_dist)) {
        norm[ind] = dirO[ind] > 0 ? -1 : 1;
        norm[i1] = norm[i2] = 0;

        return aux;
      }
    }
  }

  return -1;
}

/*************************************************************************/
int Inside2DTriangle(float pt[2], float vert[3][2]) {
  int i, i2, sign, prev_sign;

  /* test whether pt is inside the 2d triangle */
  for (i = 0; i < 3; i++) {
    /* On what side of line segment  vert1,vert2 is pt lying?
       Sign of
       (vert2[Y]-vert1[Y])*x + (vert1[X]-vert2[X])*y +
       (vert2[X]*vert1[Y] - vert2[Y]*vert1[X])
       determines that. */
    i2 = (i + 1) % 3;

    sign = ((vert[i2][Y] - vert[i][Y]) * pt[X] +
            (vert[i][X] - vert[i2][X]) * pt[Y] +
            (vert[i2][X] * vert[i][Y] - vert[i2][Y] * vert[i][X])) >= 0
               ? 1
               : -1;

    if (i > 0)
      if (sign != prev_sign) {
        /* isn't inside */
        if (verbose)
          fprintf(stderr, "Arvo - point is outside the 2d triangle \n");
        return 0;
      }

    prev_sign = sign;
  }
  return 1;
}

/*************************************************************************/
/* returns <0, when the object specified by *prim and corresponding array
   of floats *float_array, is not intersected by the ray.
   Otherwise returns the distance to the intersection and when this
   distance is less that the given mindist, normal in the intersection is
   computed as well */
float IsIntersection(float *pt, float *dir, float mindist, float surf_dist,
                     primitive_type *prim, float *norm) {
  float ptO[3], dirO[3], normO[3];
  float radius, aux, x, y, y1, y2, len, d, lenD, lenA;
  int c, i;

  if (prim == NULL)
    return -1;

  /* trasform the ray into object space */
  Transform3Point(pt, prim->matrix, ptO);
  Transform3Vector(dir, prim->matrix, dirO);

  lenA = lenD = sqrt(dirO[X] * dirO[X] + dirO[Y] * dirO[Y] + dirO[Z] * dirO[Z]);

  /* adjust mindist and surface distance */
  mindist *= lenA;
  surf_dist *= lenA;

  len = -1;

  switch (prim->flag) {
  case SPHERE:
    if (verbose)
      fprintf(stderr, "Arvo - checking sphere intersection.\n");

    for (c = X; c <= Z; c++)
      dirO[c] /= lenD;

    radius = prim->data[0] + surf_dist;

    aux =
        dirO[0] * ptO[0] + dirO[1] * ptO[1] + dirO[2] * ptO[2]; /* dir.(pt-C) */

    /* discriminant/4 = aux^2 - (pt-C).(pt-C) + rad^2 */
    if ((d = aux * aux - (ptO[0] * ptO[0] + ptO[1] * ptO[1] + ptO[2] * ptO[2]) +
             radius * radius) >= 0)
      /* there is an intersection */
      /* is it behind ptO ?*/
      if ((len = (-aux - sqrt(d))) >= 0)
        if (len < mindist) {
          /* get normal */
          for (c = X; c <= Z; c++)
            normO[c] = pt[c] + len * dirO[c];
          break;
        }

    if (verbose)
      fprintf(stderr, "Arvo - doesn't intersect the sphere.\n");

    return -1;

  case CYLINDER:
    radius = prim->data[0] + surf_dist;

    if ((aux = dirO[X] * dirO[X] + dirO[Z] * dirO[Z]) < EPSILON * EPSILON) {
      if (ptO[X] * ptO[X] + ptO[Z] * ptO[Z] <= radius * radius) {
        if ((dirO[Y] > 0) && (ptO[Y] <= -surf_dist)) {
          if ((len = -ptO[Y] - surf_dist) < mindist) {
            normO[X] = normO[Z] = 0;
            normO[Y] = -1;
            break;
          }
        }
        if ((dirO[Y] < 0) &&
            ((len = ptO[Y] - (prim->data[1] + surf_dist)) > 0)) {
          if (len < mindist) {
            normO[X] = normO[Z] = 0;
            normO[Y] = 1;
            break;
          }
        }
      }
      return -1;
    }

    lenD = aux; /* squared length of (dirO[0],dirO[2]) */

    aux = dirO[0] * ptO[0] + dirO[2] * ptO[2]; /* dir.(pt-C) */

    /* discriminant/4 = aux^2 - (pt-C).(pt-C) + rad^2 */
    if ((d = aux * aux - lenD * ((ptO[0] * ptO[0] + ptO[2] * ptO[2]) -
                                 radius * radius)) >= 0)
      /* there is an intersection */
      /* is it behind ptO ?*/
      if ((len = (-aux - (d = sqrt(d))) / lenD) >= 0)
        if (len < mindist) {
          y1 =
              ptO[1] + len * dirO[1]; /* y coordinate of the two intersections*/
          y2 = ptO[1] + (-aux + d) / lenD * dirO[1];

          if (y2 > y1) {
            aux = y2;
            y2 = y1;
            y1 = aux;
          }

          if (y1 < -surf_dist) {
            if (y2 >= -surf_dist) {
              normO[0] = normO[2] = 0;
              normO[1] = -1;
              break;
            }
          } else if (y1 < prim->data[1] + surf_dist) {
            if (y2 > prim->data[1] + surf_dist) {
              normO[0] = normO[2] = 0;
              normO[1] = 1;
              break;
            } else {
              /* y1 is on the cylinder side */
              normO[1] = 0;
              normO[0] = ptO[0] + len * dirO[0];
              normO[2] = ptO[2] + len * dirO[2];
              break;
            }
          }
        }

    return -1;

  case RECTANGLE:
    if (fabs(dirO[2]) < EPSILON)
      return -1;

    if (dirO[Z] > 0)
      len = (-surf_dist - ptO[Z]) / dirO[Z];
    else
      len = (surf_dist - ptO[Z]) / dirO[Z];

    x = ptO[X] + len * dirO[X];
    if ((x < 0) || (x > prim->data[0]))
      return -1;

    y = ptO[Y] + len * dirO[Y];
    if ((y < 0) || (y > prim->data[1]))
      return -1;

    normO[X] = 0;
    normO[Y] = 0;
    normO[Z] = dirO[Z] > 0 ? -1 : 1;
    break;

  case CONE:
#ifdef FINISH_IT
    aux = (prim->data[1] - prim->data[0]) / prim->data[2]; /* (rad2-rad1)/h */
    dirO[Y] *= aux;
    ptO[Y] *= aux;

    ca = dirO[X] * dirO[X] + dirO[Y] * dirO[Y] - dirO[Z] * dirO[Z];

    fprintf(stderr, "Arvo - cone intersection not implemented.\n");
#endif
    return -1;

  case PRISM:
 
    if ((aux = GetFaceIntersection(ptO, dirO, prim->data, 0, mindist, surf_dist,
                                   normO)) >= 0) {
      len = mindist = aux;
    }
    if ((aux = GetFaceIntersection(ptO, dirO, prim->data, 1, mindist, surf_dist,
                                   normO)) >= 0) {
      len = mindist = aux;
    }
    if ((aux = GetFaceIntersection(ptO, dirO, prim->data, 2, mindist, surf_dist,
                                   normO)) >= 0) {
      len = mindist = aux;
    }
    break;

    /******************************************************/
  case POLYGON: {
    /* following code is not one of the most effective but it is Sunday 21:46pm
       and Siggraph deadline is approaching */
    float Tnorm[3];
    float vec1[3], vec2[3];
    float I[2][3];                /* intersections with the
                                     triangle+-surf_dist */
    float intr[2][2], vert[3][2]; /* projected intersections and
                                     triangle vertices */
    float tpt[2];
    int ci[2]; /* what coordinate indexes to take - as the projection */
    int i2;
    float dir_dot_T, dI[2], t, t1, t2, d, dt;

    /* normalize dirO */
    for (c = X; c <= Z; c++)
      dirO[c] /= lenD;

    /* get the triangle normal */
    for (c = X; c <= Z; c++) {
      vec2[c] = prim->data[2 * 3 + c] - prim->data[0 * 3 + c];
      vec1[c] = prim->data[1 * 3 + c] - prim->data[0 * 3 + c];
    }
    Tnorm[0] = vec1[1] * vec2[2] - vec1[2] * vec2[1];
    Tnorm[1] = vec1[2] * vec2[0] - vec1[0] * vec2[2];
    Tnorm[2] = vec1[0] * vec2[1] - vec1[1] * vec2[0];

    Normalize(Tnorm);

    if ((dir_dot_T = DotProduct(dirO, Tnorm)) == 0)
      return -1;

    /* if the plane equation is nx*x+ny*y+nz*z+d=0, get d for the plane
       of the triangle (use the first triangle vertex). */
    d = -DotProduct(Tnorm, prim->data);

    /* if the line equation is ptO+t*dir0, get t for the intersection:
       (ptO+t*dir0)Tnorm + d = 0 */
    t = (-d - DotProduct(ptO, Tnorm)) / dir_dot_T;

    /* two intersections  t+dt and t-dt */
    dt = surf_dist / dir_dot_T;

    if (dt < 0) {
      dt = -dt;
    }
    /* t-dt will be the closer one */

    /* make sure that both intersections are within (0,mindist) */
    t1 = t - dt;
    if (t1 > mindist)
      /* first intersection is too far */
      return -1;

    if (t1 < 0)
      t1 = 0;

    t2 = t + dt;
    if (t2 < 0)
      /* the second intersection is before ptO */
      return -1;

    if (t2 > mindist)
      t2 = mindist;

    /* the two intersections */
    for (c = X; c <= Z; c++) {
      I[0][c] = ptO[c] + t1 * dirO[c];
      I[1][c] = ptO[c] + t2 * dirO[c];
    }

    /* get their d (to determine the distance from the triangle plane) */
    for (i = 0; i < 2; i++) {
      dI[i] = -DotProduct(I[i], Tnorm);
      if ((dI[i] < d - surf_dist - 0.00001) ||
          (dI[i] > d + surf_dist + 0.00001)) {
        fprintf(stderr, "Arvo - dI[%d]=%g out of range %g+-%g\n", i, dI[i], d,
                surf_dist);
        fprintf(stderr, "Arvo - t=%g, dt=%g, t1=%g, t2=%g, mindist=%g\n", t, dt,
                t1, t2, mindist);
      }
    }

    /* find a plane into which the triangle and the two intersections may
       be projected without reducing the triangle to a line segment */
    for (c = Z; c >= X; c--)
      if (fabs(Tnorm[c]) > 0.5) {
        /* project to plane [c]=0 */
        ci[0] = (c + 1) % 3;
        ci[1] = (c + 2) % 3;
        break;
      }

    /* intersections must be brought to the triangle plane */
    for (c = 0; c < 2; c++) {
      intr[0][c] = I[0][ci[c]] - (d - dI[0]) * Tnorm[ci[c]];
      intr[1][c] = I[1][ci[c]] - (d - dI[1]) * Tnorm[ci[c]];

      /* triangle vertices */
      for (i = 0; i < 3; i++)
        vert[i][c] = prim->data[i * 3 + ci[c]];
    }

    /* check whether one of the projected intersections is inside the triangle
     */
    for (i = 0; i < 2; i++)
      if (Inside2DTriangle(intr[0], vert))
        break;

    /* if i<2 there is an intersection */
    if (i == 2) {
      return -1;
      /* both points outside */
      /* test for intersection between line segment intr1,intr2 and each
         triangle edge */
      for (i = 0; i < 3; i++) {
        /* line segment  intr1 + t2*(intr2-intr1) and segment with equation:
           (vert2[Y]-vert1[Y])*x + (vert1[X]-vert2[X])*y +
           (vert2[X]*vert1[Y] - vert2[Y]*vert1[X]) = 0; */
        i2 = (i + 1) % 3;
        /* get t2 */
        t2 = (-(vert[i2][X] * vert[i][Y] - vert[i2][Y] * vert[i][X]) -
              (vert[i2][Y] - vert[i][Y]) * intr[0][X] -
              (vert[i][X] - vert[i2][X]) * intr[0][Y]) /
             ((vert[i2][Y] - vert[i][Y]) * (intr[1][X] - intr[0][X]) +
              (vert[i][X] - vert[i2][X]) * (intr[1][Y] - intr[0][Y]));

        if ((t2 >= 0) && (t2 <= 1)) {
          /* now we know that segment intr1,intr2 does intersect the line */
          /* does it intersect the line segment though? */

          tpt[X] = intr[0][X] + t2 * (intr[1][X] - intr[0][X]);
          tpt[Y] = intr[0][Y] + t2 * (intr[1][Y] - intr[0][Y]);

          /* product of vectors (vert1,tpt)(vect2,tpt) should be <=0 */
          if ((tpt[X] - vert[i][X]) * (tpt[X] - vert[i2][X]) +
                  (tpt[Y] - vert[i][Y]) * (tpt[X] - vert[i2][X]) <=
              0) {
            /* intersection indeed */
            break; /* for(i=0;i<3;i++) */
          }
        }
      }

      if (i == 3)
        /* no intersection */
        return -1;
    }

    if (t - dt == t1)
      len = t1;
    else
      len = t2;

    for (c = 0; c < 3; c++)
      normO[c] = Tnorm[c];

    if (dirO[0] * normO[0] + dirO[1] * normO[1] + dirO[2] * normO[2] < 0)
      for (c = 0; c < 3; c++)
        normO[c] *= -1;

    break;
  }
  default:
    return -1;
  }

  if (len <= 0)
    return -1;

  if (verbose) {
    fprintf(stderr, "Arvo - intersection found.\n");
    fprintf(stderr, "Arvo - returned len = %g, mindist=%g\n", len / lenA,
            mindist);
  }

  /* transform normal */
  Transform3Vector(normO, prim->invmatrix, norm);

  Normalize(norm);

  return len / lenA;
}

/*************************************************************************/
int IsInObject(float *pt, float surf_dist, primitive_type *prim, float *norm) {
  float ptO[3], radius, len0, len1, len2, len3, aux, normO[3] = {0, 1, 0};
  float red;
  int c;

  if (prim == NULL)
    return 0;

  /* trasform the point into object space */
  Transform3Point(pt, prim->matrix, ptO);

  switch (prim->flag) {
  case SPHERE:
    if (verbose)
      fprintf(stderr, "Arvo - is inside a sphere?\n");

    radius = prim->data[0] + surf_dist + EPSILON;

    if (ptO[0] * ptO[0] + ptO[1] * ptO[1] + ptO[2] * ptO[2] > radius * radius)
      return 0;

    for (c = X; c <= Z; c++)
      normO[c] = ptO[c];

    if (verbose)
      fprintf(stderr, "Arvo - yes, is inside the sphere.\n");

    break;

  case CYLINDER:
    radius = prim->data[0] + surf_dist + EPSILON;

    if ((len1 = ptO[0] * ptO[0] + ptO[2] * ptO[2]) > radius * radius)
      return 0;

    len1 = radius - sqrt(len1);

    if (((len2 = surf_dist + ptO[Y]) < 0) ||
        ((len3 = prim->data[1] + surf_dist - ptO[Y]) < 0))
      return 0;

    if (len2 < len3) /* closer to the bottom */
      normO[Y] = -1;

    if ((len1 < len2) && (len1 < len3)) {
      normO[Y] = 0;
      normO[X] = ptO[X];
      normO[Z] = ptO[Z];
    }

    break;

  case RECTANGLE:
    return 0;

  case CONE:
    aux = (prim->data[1] - prim->data[0]) / prim->data[2];
    red = sqrt(1 + aux * aux);

    radius = prim->data[0] + aux * ptO[Y] + surf_dist * red;
    /* even the surface distance is adjusted */

    if ((len0 = ptO[X] * ptO[X] + ptO[Z] * ptO[Z]) > radius * radius)
      return 0;

    len1 = radius - (len0 = sqrt(len0));

    /* the distance from the side is */
    len1 /= red;

    if (((len2 = surf_dist + ptO[Y]) < 0) ||
        ((len3 = prim->data[2] + surf_dist - ptO[Y]) < 0))
      return 0;

    if (len2 < len3) /* closer to the bottom */
      normO[Y] = -1;

    if ((len1 < len2) && (len1 < len3)) {
      normO[Y] = -aux * len0;
      normO[X] = ptO[X];
      normO[Z] = ptO[Z];
    }

    break;

  case PRISM:
    len1 = prim->data[0] + prim->data[1] + prim->data[2];

    for (c = X; c <= Z; c++) {
      if ((len2 = ptO[c] + surf_dist) < 0)
        return 0;
      if (len2 < len1) {
        len1 = len2;
        normO[c] = -1;
        normO[(c + 1) % 3] = normO[(c + 2) % 3] = 0;
      }

      if ((len2 = prim->data[c] + surf_dist - ptO[c]) < 0)
        return 0;
      if (len2 < len1) {
        len1 = len2;
        normO[c] = 1;
        normO[(c + 1) % 3] = normO[(c + 2) % 3] = 0;
      }
    }

    break;
    /******************************************************/
  case POLYGON: {
    float Tnorm[3];
    float vec1[3], vec2[3];
    float Tpt[2], vert[3][2]; /* projected point and
                                 triangle vertices */
    int ci[2]; /* what coordinate indexes to take - as the projection */
    int i;
    float d, dp;

    /* get the triangle normal */
    for (c = 0; c < 3; c++) {
      vec2[c] = prim->data[2 * 3 + c] - prim->data[0 * 3 + c];
      vec1[c] = prim->data[1 * 3 + c] - prim->data[0 * 3 + c];
    }
    Tnorm[0] = vec1[1] * vec2[2] - vec1[2] * vec2[1];
    Tnorm[1] = vec1[2] * vec2[0] - vec1[0] * vec2[2];
    Tnorm[2] = vec1[0] * vec2[1] - vec1[1] * vec2[0];

    Normalize(Tnorm);

    if (verbose) {
      fprintf(stderr, "Arvo ---------------------------------------------\n");
      fprintf(stderr, "Arvo - triangle normal (%g,%g,%g)\n", Tnorm[0], Tnorm[1],
              Tnorm[2]);
      fprintf(stderr, "Arvo - ptO=(%g,%g,%g)\n", ptO[X], ptO[Y], ptO[Z]);
      fprintf(stderr, "Arvo - first vertex (%g,%g,%g)\n", prim->data[X],
              prim->data[Y], prim->data[Z]);
    }

    /* if the plane equation is nx*x+ny*y+nz*z+d=0, get d for the plane
       of the triangle (use the first triangle vertex). */
    d = -DotProduct(Tnorm, prim->data);

    /* get d of the point */
    dp = -DotProduct(Tnorm, ptO);

    if (verbose)
      fprintf(stderr, "Arvo - d=%g, dp=%g\n", d, dp);

    /* is dp within the range  d-surf_dist, d+surf_dist ? */
    if ((dp < d - surf_dist) || (dp > d + surf_dist)) {
      if (verbose)
        fprintf(stderr, "Arvo - point is too far from the triangle plane\n");

      return 0;
    }

    /* find a plane into which the triangle and the two intersections may
       be projected without reducing the triangle to a line segment */
    for (c = Z; c >= X; c--)
      if (fabs(Tnorm[c]) > 0.5) {
        /* project to plane [c]=0 */
        ci[0] = (c + 1) % 3;
        ci[1] = (c + 2) % 3;
        break;
      }

    for (c = 0; c < 2; c++) {
      /* move the point into the triangle plane before projecting */
      Tpt[c] = ptO[ci[c]] + (d - dp) * Tnorm[ci[c]];

      /* triangle vertices */
      for (i = 0; i < 3; i++)
        vert[i][c] = prim->data[i * 3 + ci[c]];
    }

    if (verbose) {
      fprintf(stderr, "Arvo - 2d point (%g,%g)\n", Tpt[X], Tpt[Y]);
      fprintf(stderr, "Arvo - 2d triangle (%g,%g), (%g,%g), (%g,%g)\n",
              vert[0][X], vert[0][Y], vert[1][X], vert[1][Y], vert[2][X],
              vert[2][Y]);
    }

    if (Inside2DTriangle(Tpt, vert) == 0)
      return 0;

    /* normal - not on the sides */
    for (c = 0; c < 3; c++)
      normO[c] = Tnorm[c];

    if (dp > d)
      for (c = 0; c < 3; c++)
        normO[c] *= -1;

    break;
  }
  default:
    return 0;
  }

  if (verbose)
    fprintf(stderr, "Arvo - point is inside the object.\n");

  /* transform normal */
  Transform3Vector(normO, prim->invmatrix, norm);

  Normalize(norm);

  return 1;
}

/****************************************************************************/
/* adds a new item to the beginning of a linked list */
void AddToList(OBJECT_LIST_TYPE **first, primitive_type *prim) {
  OBJECT_LIST_TYPE *ptr;

  if ((ptr = (OBJECT_LIST_TYPE *)malloc(sizeof(OBJECT_LIST_TYPE))) == NULL) {
    fprintf(stderr, "Arvo - cannot allocate memory for leaf item!\n");
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
  float pt[4];
  int range[3][2];
  CELL_TYPE *cell;

  pt[3] = 1.0; /* always */

  if (verbose)
    fprintf(stderr, "Primitive bbox: (%g,%g,%g) %gx%gx%g\n", prim->bbox[0],
            prim->bbox[1], prim->bbox[2], prim->bbox[3], prim->bbox[4],
            prim->bbox[5]);

  for (c = X; c <= Z; c++) {
    range[c][0] = floor((prim->bbox[c] - grid->pos[c]) / grid->range[c] *
                        (float)grid->size[c]) -
                  1;
    if (range[c][0] < 0)
      range[c][0] = 0;

    range[c][1] = 1 + floor((prim->bbox[c] + prim->bbox[c + 3] - grid->pos[c]) /
                            grid->range[c] * (float)grid->size[c]);
    if (range[c][1] >= grid->size[c])
      range[c][1] = grid->size[c] - 1;
  }

  if (verbose)
    fprintf(stderr, "Primitive range: x:%d-%d; y:%d-%d; z:%d-%d\n", range[X][0],
            range[X][1], range[Y][0], range[Y][1], range[Z][0], range[Z][1]);

  /* for all nodes in the range */
  for (z = range[Z][0]; z <= range[Z][1]; z++) {
    pt[Z] =
        grid->pos[Z] + ((float)z + 0.5) / (float)grid->size[Z] * grid->range[Z];

    for (y = range[Y][0]; y <= range[Y][1]; y++) {
      pt[Y] = grid->pos[Y] +
              ((float)y + 0.5) / (float)grid->size[Y] * grid->range[Y];

      for (x = range[X][0]; x <= range[X][1]; x++) {
        pt[X] = grid->pos[X] +
                ((float)x + 0.5) / (float)grid->size[X] * grid->range[X];

        cell = grid->data + z * grid->size[X] * grid->size[Y] +
               y * grid->size[X] + x;

        AddToList(&(cell->list), prim);
      }
    }
  }
}

/*************************************************************************/
/* fill grid according to the presence of obstacles  */
void FillGrid(grid_type *grid) {
  int x, y, z, obs;
  float pt[4];
  CELL_TYPE *cell;
  array_item *ptr;

  if (verbose)
    fprintf(stderr, "Start filling the grid.\n");

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

        cell->list = NULL;
      }
    }
  }

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
