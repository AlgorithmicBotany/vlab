/*
  Environmental process - soil (2D)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "image.h"
#include "targa.h"
#include "soil2d.h"
#include "soil3d.h"
#include "comm_lib.h"
#include "matrix.h"

/**** field specific variables ****/

grid_type grid;

float a, n, h; /* water flow constants */

#define MAXLAYERS 100
struct layer_type {
  float geotropic_angle;
  float thickness;
} layer[MAXLAYERS];

int num_layers;

char verbose;

char depletion;  /* is the grid updated by L-system? */
DATA_TYPE omega; /* controls the speed of convergence (relaxation factor) */
char obs_src;    /* are obstacles and sources simulated? */

char output_type; /* type of 3D output: polygons/triangles */

float concentration_contour_value;
float concentration_alpha;

char *contour_mat;
char *source_mat;
char *section_mat;

char output_normals;
char keep_depl_cells;

/* diffusion parameters */
int diff_steps;
float diff_tolerance;

/* to output files just at some specific points */
struct ANIMINTERVAL {
  int from;
  int to;
  int step;
};
typedef struct ANIMINTERVAL ANIMINTERVAL;

#define MAXANIMINTERVALS 1000

ANIMINTERVAL intervals[MAXANIMINTERVALS];

void vec_to_quat(float v[3], float q[4]) {
  q[0] = 0;
  q[1] = v[0];
  q[2] = v[1];
  q[3] = v[2];
}

void quat_to_vec(float q[4], float v[3]) {
  v[0] = q[1];
  v[1] = q[2];
  v[2] = q[3];
}

void mult_quats(float q1[4], float q2[4], float dest[4]) {
  dest[0] = q1[0] * q2[0] - (q1[1] * q2[1] + q1[2] * q2[2] + q1[3] * q2[3]);

  dest[1] = q1[0] * q2[1] + q2[0] * q1[1] + q1[2] * q2[3] - q1[3] * q2[2];
  dest[2] = q1[0] * q2[2] + q2[0] * q1[2] + q1[3] * q2[1] - q1[1] * q2[3];
  dest[3] = q1[0] * q2[3] + q2[0] * q1[3] + q1[1] * q2[2] - q1[2] * q2[1];
}

void inverse_quat(float q[4], float dest[4]) {
  float size = 1.0 / (q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);

  dest[0] = size * q[0];
  dest[1] = -size * q[1];
  dest[2] = -size * q[2];
  dest[3] = -size * q[3];
}

void rot_by_quat(float v[3], float q[4]) {
  float qv[4];
  float qi[4];
  float qr[4];

  inverse_quat(q, qi);
  vec_to_quat(v, qv);

  mult_quats(qi, qv, qr);
  mult_quats(qr, q, qv);
  quat_to_vec(qv, v);
}

void rot_by_quat2(float src[3], float q[4], float dest[3]) {
  float qv[4];
  float qi[4];
  float qr[4];

  inverse_quat(q, qi);
  vec_to_quat(src, qv);

  mult_quats(qi, qv, qr);
  mult_quats(qr, q, qv);

  quat_to_vec(qv, dest);
}

/****************************************************************************/
void FreeFieldStructures(void) {
  if (grid.data != NULL)
    free(grid.data);

  if (grid.outimagename != NULL)
    free(grid.outimagename);
}

/****************************************************************************/
CELL_TYPE *GetCell(grid_type *grid, int x, int y, int z) {
  if (x < 0)
    x = 0;
  if (x >= grid->size[X])
    x = grid->size[X] - 1;

  if (y < 0)
    y = 0;
  if (y >= grid->size[Y])
    y = grid->size[Y] - 1;

  if (z < 0)
    z = 0;
  if (z >= grid->size[Z])
    z = grid->size[Z] - 1;

  return grid->data + z * grid->size[Y] * grid->size[X] + y * grid->size[X] + x;
}

/****************************************************************************/
int LoadInitialData(char *imagename, grid_type *grid) {
  int x, y;
  IMAGE *image;
  unsigned short *buf;
  unsigned short r, g, b;
  CELL_TYPE *ptr;
  int c;
  targa_params_type TGAspec;

  if ((c = strlen(imagename)) > 3)
    if (strcmp(imagename + c - 3, "tga") == 0) {
      int xsize, ysize;
      unsigned char *rowbuf;

      /* load in targa image */
      if (!loadTGAhead(imagename, &TGAspec)) {
        fprintf(stderr, "soil: cannot open input targa image %s.\n", imagename);
        return 0;
      }

      grid->size[X] = xsize = TGAspec.Xres;
      grid->size[Y] = ysize = TGAspec.Yres;

      /* one line buffer */
      if ((rowbuf = (unsigned char *)malloc(xsize * 3 *
                                            sizeof(unsigned char))) == NULL) {
        fprintf(stderr, "soil: cannot allocate memory for image %s!\n",
                imagename);
        return 0;
      }

      if ((grid->data = (CELL_TYPE *)malloc(grid->size[X] * grid->size[Y] *
                                            sizeof(CELL_TYPE))) == NULL) {
        fprintf(stderr, "Soil - cannot allocate memory for the grid!\n");
        return 0;
      }

      /* read the image in */
      /* image is stored in rows of R, G, and B. R,G,B is converted to b&w
         and stored as a float (0-255) */
      for (y = ysize - 1; y >= 0; y--) {
        loadTGArow(&TGAspec, y, rowbuf);

        ptr = grid->data + y * grid->size[X];

        for (x = 0; x < grid->size[X]; x++) {
          if (obs_src) {
            r = rowbuf[x * 3 + 2];
            g = rowbuf[x * 3 + 1];
            b = rowbuf[x * 3 + 0];

            switch (r + g + b) {
            case 765: /* 3*255 */
              ptr->flag = SOURCE;
              ptr->val = grid->max;
              break;
            case 165: /* 40 +55+70 */
              ptr->flag = OBSTACLE;
              ptr->val = 0;
              if ((r == 40) && (g == 55) && (b == 70))
                break;
            default:
              if ((r == b) && (r > 0)) {
                ptr->flag = OBSTACLE;
                ptr->val = 0;
                break;
              }
              ptr->flag = DATA;
              ptr->val = grid->min +
                         (((DATA_TYPE)g) / 255.0) * (grid->max - grid->min);
            }
          } else {
            ptr->flag = DATA;
            ptr->val = grid->min + ((0.299 * (DATA_TYPE)rowbuf[x * 3 + 2] +
                                     0.587 * (DATA_TYPE)rowbuf[x * 3 + 1] +
                                     0.114 * (DATA_TYPE)rowbuf[x * 3 + 0]) /
                                    255.0) *
                                       (grid->max - grid->min);
          }
          ptr++;
        }
      }

      loadTGAfinish(&TGAspec);
      free(rowbuf);

      return 1;
    }

#ifndef SUN
  if ((image = iopen(imagename, "r", 0, 0, 0, 0, 0)) == NULL) {
    fprintf(stderr, "Soil - cannot open image %s!\n", imagename);
    return 0;
  }

  grid->size[X] = image->xsize;
  grid->size[Y] = image->ysize;

  /* one line buffer */
  if ((buf = (unsigned short *)malloc(3 * image->xsize *
                                      sizeof(unsigned short))) == NULL) {
    fprintf(stderr,
            "Soil - cannot allocate memory for one line of the image %s!\n",
            imagename);
    return 0;
  }

  if ((grid->data = (CELL_TYPE *)malloc(grid->size[X] * grid->size[Y] *
                                        sizeof(CELL_TYPE))) == NULL) {
    fprintf(stderr, "Soil - cannot allocate memory for the grid!\n");
    return 0;
  }

  /* read the image in */
  /* image is stored in rows of R, G, and B. R,G,B is converted to b&w and */
  /* and stored as a float (0-255) */
  for (y = 0; y < grid->size[Y]; y++) {
    /* red */
    getrow(image, buf, y, 0);
    /* green */
    getrow(image, buf + image->xsize, y, 1);
    /* blue */
    getrow(image, buf + 2 * image->xsize, y, 2);

    ptr = grid->data + y * grid->size[X];

    for (x = 0; x < grid->size[X]; x++) {
      if (obs_src) {
        r = buf[x];
        g = buf[x + image->xsize];
        b = buf[x + 2 * image->xsize];

        switch (r + g + b) {
        case 765: /* 3*255 */
          ptr->flag = SOURCE;
          ptr->val = grid->max;
          break;
        case 165: /* 40 +55+70 */
          ptr->flag = OBSTACLE;
          ptr->val = 0;
          if ((r == 40) && (g == 55) && (b == 70))
            break;
        default:
          if ((r == b) && (r > 0)) {
            ptr->flag = OBSTACLE;
            ptr->val = 0;
            break;
          }
          ptr->flag = DATA;
          ptr->val = grid->min + (((DATA_TYPE)buf[x + image->xsize]) / 255.0) *
                                     (grid->max - grid->min);
        }
      } else {
        ptr->flag = DATA;
        ptr->val = grid->min + ((0.299 * (DATA_TYPE)buf[x] +
                                 0.587 * (DATA_TYPE)buf[x + image->xsize] +
                                 0.114 * (DATA_TYPE)buf[x + 2 * image->xsize]) /
                                255.0) *
                                   (grid->max - grid->min);
      }
      ptr++;
    }
  }

  free(buf);
  iclose(image);
#else
  /* SUN */
  fprintf(stderr,
          "Soil - reading of an rgb image not supported in Sun version.\n"
          "       Initialized to uniform 64x32 grid of 0 values");

  if ((grid->data = (CELL_TYPE *)malloc(64 * 32 * sizeof(CELL_TYPE))) == NULL) {
    fprintf(stderr, "Soil - cannot allocate memory for the 64x32 grid.\n");
    return 0;
  }
#endif

  return 1;
}

/****************************************************************************/
int SaveImage(grid_type *grid) {
  unsigned short *row;
  IMAGE *oimage;
  int y, x;
  DATA_TYPE val;
  int size[2], c;
  unsigned char *rowb;
  targa_params_type TGAspec;

  /* make it closest higher or equal power of two (6->8, 4->4, 7->8)
   to be able to place it as a texture */
  size[X] = 1;
  while (grid->size[X] > size[X])
    size[X] <<= 1;

  size[Y] = 1;
  while (grid->size[Y] > size[Y])
    size[Y] <<= 1;

  if (((c = strlen(grid->outimagename)) > 3) &&
      (strcmp(grid->outimagename + c - 3, "tga") == 0)) {
    /* save TGA image */
    TGAspec.type = TGA_TRUECOLOR_RLE;
    TGAspec.Xres = size[X];
    TGAspec.Yres = size[Y];

    if ((TGAspec.fp = fopen(grid->outimagename, "w")) == NULL) {
      fprintf(stderr, "soil:  cannot open targa file %s\n", grid->outimagename);
      return 0;
    }

    saveTGAhead(&TGAspec);

    if ((rowb = (unsigned char *)malloc(
             (unsigned int)(3 * size[X] * sizeof(unsigned char)))) == NULL)
      return 0;

    /* the bottom rows are always black */
    for (x = 0; x < size[X]; x++)
      rowb[3 * x] = rowb[3 * x + 1] = rowb[3 * x + 2] = 0;

    for (y = grid->size[Y]; y < size[Y]; y++) {
      saveTGArow(&TGAspec, y, rowb);
    }

    for (y = grid->size[Y] - 1; y >= 0; y--) {
      for (x = 0; x < grid->size[X]; x++) {
        val = grid->data[y * grid->size[X] + x].val;

        val = (val - grid->min) / (grid->max - grid->min) * 255.0;
        if (val < 0)
          val = 0;
        if (val > 255)
          val = 255;

        switch (grid->data[y * grid->size[X] + x].flag) {
        case DATA:
          rowb[3 * x + 1] = (unsigned short)val; /* green */
          rowb[3 * x] = rowb[3 * x + 2] = (unsigned short)val;
          break;
        case SOURCE:
          rowb[3 * x] = rowb[3 * x + 1] = rowb[3 * x + 2] = 255;
          break;
        case OBSTACLE:
          rowb[3 * x + 2] = 40;
          rowb[3 * x + 1] = 55;
          rowb[3 * x + 0] = 70;
          break;
        }
      }
      saveTGArow(&TGAspec, y, rowb);
    }

    free(rowb);
    saveTGAfinish(&TGAspec);

    if (verbose)
      fprintf(stderr, "soil - image %s saved.\n", grid->outimagename);
    return 1;
  }

  if ((row = (unsigned short *)malloc(
           (unsigned int)(3 * size[X] * sizeof(short)))) == NULL)
    return 0;

  oimage = iopen(grid->outimagename, "w", RLE(1), 3, size[X], size[Y], 3);
  if (oimage == NULL) {
    perror(grid->outimagename);
    return 0;
  }
  isetname(oimage, grid->outimagename);

  /* the end of a row is always black */
  for (x = grid->size[X]; x < size[X]; x++)
    row[3 * x] = row[3 * x + 1] = row[3 * x + 2] = 0;

  for (y = 0; y < grid->size[Y]; y++) {
    for (x = 0; x < grid->size[X]; x++) {
      val = grid->data[y * grid->size[X] + x].val;

      val = (val - grid->min) / (grid->max - grid->min) * 255.0;
      if (val < 0)
        val = 0;
      if (val > 255)
        val = 255;

      switch (grid->data[y * grid->size[X] + x].flag) {
      case DATA:
        row[size[X] + x] = (unsigned short)val; /* green */
        row[x] = row[2 * size[X] + x] = (unsigned short)val;
        break;
      case SOURCE:
        row[x] = row[size[X] + x] = row[2 * size[X] + x] = 255;
        break;
      case OBSTACLE:
        row[x] = 40;
        row[size[X] + x] = 55;
        row[2 * size[X] + x] = 70;
        break;
      }
    }

    /* output the b&w image as a truecolor image */
    putrow(oimage, row, y, 0);               /* red */
    putrow(oimage, row + size[X], y, 1);     /* green */
    putrow(oimage, row + 2 * size[X], y, 2); /* blue */
  }

  /* fill the remaining rows by zeros */
  for (x = 0; x < grid->size[X]; x++)
    row[x] = 0;

  for (; y < size[Y]; y++) {
    putrow(oimage, row, y, 0); /* red */
    putrow(oimage, row, y, 1); /* green */
    putrow(oimage, row, y, 2); /* blue */
  }

  free(row);
  iclose(oimage);

  if (verbose)
    fprintf(stderr, "Soil - image %s saved.\n", grid->outimagename);

  return 1;
}

/****************************************************************************/
/* returns 0/1 */
int IsInObstacle2d(CELL_TYPE *cell) { return cell->flag == OBSTACLE; }

/****************************************************************************/
/* returns the obstacle index */
int IsInObstacle3d(CELL_TYPE *cell, float *pt) {
  if (cell->flag == OBSTACLE)
    return 1;

  if (cell->obstacle[0] >= 0)
    return IsInsideO(cell->obstacle, pt);

  return -1;
}

/****************************************************************************/
/* 2d obstacle avoiding
 */
#define AVOID_STEP 2 /* degres */

void SolveAvoiding2D(grid_type *grid, int x, int y, Cmodule_type *comm_symbol,
                     CTURTLE *tu) {
  float len, sP[3], eP[4], rot[2][2], up[2], down[2], vec[2];
  float angle, div_angle;
  int i, ix, j;

  if (comm_symbol->num_params < 6)
    return;

  eP[3] = 1; /* for IsInsideO */

  for (i = 0; i < 3; i++) /* end point */
    eP[i] = tu->position[i];

  /* get the starting point of the internode from the current position
     and the internode length sent as the 5-th parameter of ?E */
  len = comm_symbol->params[4].value;

  if ((!IsInObstacle2d(grid->data + y * grid->size[X] + x)) || (len <= 0)) {
    comm_symbol->params[4].value = 0;
    comm_symbol->params[4].set = 1;
    return;
  }

  if (verbose)
    fprintf(stderr, "2d avoiding.\n");

  if (tu->headingC < 2) {
    fprintf(stderr,
            "2d avoiding - turtle heading wasn't sent to the environment.\n");
    return;
  }

  /* get an angle by which is the current heading diverted from the previous
     one (sent as the 6-th parameter of ?E) */
  div_angle = comm_symbol->params[5].value;

  if (verbose)
    fprintf(stderr, "2d avoiding - internode lenght is %lf.\n", len);

  for (i = 0; i < 3; i++) /* startpoint */
    sP[i] = tu->position[i] - len * tu->heading[i];

  /* create the rotation matrix */
  rot[0][0] = rot[1][1] = cos((double)AVOID_STEP / 180.0 * M_PI);
  rot[0][1] = -(rot[1][0] = sin((double)AVOID_STEP / 180.0 * M_PI));

  for (i = 0; i < 2; i++)
    up[i] = down[i] = len * tu->heading[i];

  angle = 0;

  /* first seek in the interval 0,div_angle */
  for (j = 1; j <= fabs((double)div_angle) / AVOID_STEP; j++) {
    vec[0] = up[0] * rot[0][0] + (div_angle >= 0 ? 1 : -1) * up[1] * rot[1][0];
    vec[1] = (div_angle >= 0 ? 1 : -1) * up[0] * rot[0][1] + up[1] * rot[1][1];

    eP[0] = sP[0] + vec[0];
    eP[1] = sP[1] + vec[1];

    x = floor((eP[0] - grid->pos[X]) / grid->range[X] *
              (double)(grid->size[X]));
    y = floor((eP[1] - grid->pos[Y]) / grid->range[Y] *
              (double)(grid->size[Y]));

    if ((x >= 0) && (x < grid->size[X]) && (y >= 0) && (y < grid->size[Y]))
      if (!IsInObstacle2d(grid->data + y * grid->size[X] + x)) {
        angle = AVOID_STEP * j * (div_angle >= 0 ? 1 : -1);
        break;
      }
    up[0] = vec[0];
    up[1] = vec[1];
  }

  if (angle == 0) {
    /* rotate the internode +- n*AVOID_STEP and find the smallest avoiding
       angle */
    for (i = 1; i <= 180 / AVOID_STEP; i++) {
      vec[0] =
          up[0] * rot[0][0] + (div_angle >= 0 ? 1 : -1) * up[1] * rot[1][0];
      vec[1] =
          (div_angle >= 0 ? 1 : -1) * up[0] * rot[0][1] + up[1] * rot[1][1];
      for (ix = 0; ix < 2; ix++) {
        eP[ix] = sP[ix] + vec[ix];
      }

      x = floor((eP[0] - grid->pos[X]) / grid->range[X] *
                (double)(grid->size[X]));
      y = floor((eP[1] - grid->pos[Y]) / grid->range[Y] *
                (double)(grid->size[Y]));

      if ((x >= 0) && (x < grid->size[X]) && (y >= 0) && (y < grid->size[Y]))
        if (!IsInObstacle2d(grid->data + y * grid->size[X] + x)) {
          angle = AVOID_STEP * (i + j - 1) * (div_angle >= 0 ? 1 : -1);
          break;
        }
      up[0] = vec[0];
      up[1] = vec[1];

      /* the same the other direction */
      vec[0] = down[0] * rot[0][0] +
               -(div_angle >= 0 ? 1 : -1) * down[1] * rot[1][0];
      vec[1] = -(div_angle >= 0 ? 1 : -1) * down[0] * rot[0][1] +
               down[1] * rot[1][1];
      for (ix = 0; ix < 2; ix++) {
        eP[ix] = sP[ix] + vec[ix];
      }

      x = floor((eP[0] - grid->pos[X]) / grid->range[X] *
                (double)(grid->size[X]));
      y = floor((eP[1] - grid->pos[Y]) / grid->range[Y] *
                (double)(grid->size[Y]));

      if ((x >= 0) && (x < grid->size[X]) && (y >= 0) && (y < grid->size[Y]))
        if (!IsInObstacle2d(grid->data + y * grid->size[X] + x)) {
          angle = -AVOID_STEP * i * (div_angle >= 0 ? 1 : -1);
          break;
        }
      down[0] = vec[0];
      down[1] = vec[1];
    }
  }

  if (angle == 0)
    fprintf(stderr,
            "2d avoiding - didn't find a free position for the internode.\n");

  /* return the angle as the 5-th parameter of ?E */
  comm_symbol->params[4].value = angle;
  comm_symbol->params[4].set = 1;

  if (verbose)
    fprintf(stderr, "2d avoiding - returned angle is %lf degrees.\n", angle);
}

/****************************************************************************/
/* 3d obstacle avoiding
 */
void SolveAvoiding3D(grid_type *grid, int x, int y, int z,
                     Cmodule_type *comm_symbol, CTURTLE *tu) {
  float sP[3], eP[4], vec[3], heading[3];
  float norm[4], rot[3], len, q[4], bendsin;
  float angle, astep;
  int i, ix, j, oind;
  CELL_TYPE *cell;

  if (comm_symbol->num_params < 7)
    return;

  for (i = 0; i < 3; i++) /* end point */
    eP[i] = tu->position[i];
  eP[3] = 1; /* for IsInsideO */

  cell = grid->data + z * grid->size[X] * grid->size[Y] + y * grid->size[X] + x;

  /* get the starting point of the internode from the current position
     and the internode length sent as the 5-th parameter of ?E */
  len = comm_symbol->params[4].value;

  if (((oind = IsInObstacle3d(cell, eP)) == -1) || (len <= 0)) {
    comm_symbol->params[4].value = 0;
    comm_symbol->params[5].value = 0;
    comm_symbol->params[6].value = 0;
    comm_symbol->params[4].set = 1;
    comm_symbol->params[5].set = 1;
    comm_symbol->params[6].set = 1;
    return;
  }

  if (verbose)
    fprintf(stderr, "3d avoiding.\n");

  if (tu->headingC < 3) {
    fprintf(stderr,
            "3d avoiding - turtle heading wasn't sent to the environment.\n");
    return;
  }

  if (verbose)
    fprintf(stderr, "3d avoiding - internode lenght is %lf.\n", len);

  for (i = 0; i < 3; i++) /* startpoint */
    sP[i] = tu->position[i] - len * tu->heading[i];

  /* get the normal */
  GetObstacleNormal(oind, eP, norm);

  if (verbose)
    fprintf(stderr, "3d avoiding - normal is (%f,%f,%f).\n", norm[0], norm[1],
            norm[2]);

  /* get the rotation vector */
  CrossProduct(tu->heading, norm, rot);

  /* normalize the rotation vector */
  if ((len = sqrt(rot[0] * rot[0] + rot[1] * rot[1] + rot[2] * rot[2])) ==
      0.0) {
    if ((tu->heading[0] == 0) && (tu->heading[1] == 0)) {
      norm[1] = norm[2] = 0;
      norm[0] = -tu->heading[2];
    } else {
      norm[0] = -tu->heading[1];
      norm[1] = tu->heading[0];
      norm[2] = 0;
    }

    return;
  }

  for (i = 0; i < 3; i++)
    rot[i] /= len;

  if (verbose)
    fprintf(stderr, "3d avoiding - rotation vector is (%f,%f,%f).\n", rot[0],
            rot[1], rot[2]);

  astep = (float)AVOID_STEP / 180 * M_PI / 2;

  for (i = 0; i < 3; i++)
    heading[i] = comm_symbol->params[4].value * tu->heading[i];

  /* rotate the internode +- n*AVOID_STEP and find the smallest avoiding
     angle */
  for (angle = astep; angle <= M_PI / 2; angle += astep)
    for (i = -1; i <= 1; i += 2) {
      /* create a quaternion expressing the desired rotation */
      /* note that the rotation is by angle 2*angle */
      q[0] = cos((double)angle * (double)i);
      q[1] = (bendsin = sin((double)angle * (double)i)) * rot[0];
      q[2] = bendsin * rot[1];
      q[3] = bendsin * rot[2];

      rot_by_quat2(heading, q, vec);

      for (ix = 0; ix < 3; ix++) {
        eP[ix] = sP[ix] + vec[ix];
      }

      x = floor((eP[0] - grid->pos[X]) / grid->range[X] *
                (double)(grid->size[X]));
      y = floor((eP[1] - grid->pos[Y]) / grid->range[Y] *
                (double)(grid->size[Y]));
      z = floor((eP[2] - grid->pos[Z]) / grid->range[Z] *
                (double)(grid->size[Z]));

      if ((x >= 0) && (x < grid->size[X]) && (y >= 0) && (y < grid->size[Y]) &&
          (z >= 0) && (z < grid->size[Z]))
        if (IsInObstacle3d(grid->data + z * grid->size[X] * grid->size[Y] +
                               y * grid->size[X] + x,
                           eP) == -1) {
          /* normalize vec */
          /*	  len = sqrt(vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2]);
          for(j=0;j<3;i++)
            vec[j] /= len;
            */

          /* return vec as the 5-th to 7-th parameter of ?E */
          for (j = 0; j < 3; j++) {
            comm_symbol->params[4 + j].value = vec[j];
            comm_symbol->params[4 + j].set = 1;
          }

          if (verbose)
            fprintf(stderr, "3d avoiding - returned vector is (%f,%f,%f).\n",
                    vec[0], vec[1], vec[2]);
          return;
        }
    }

  fprintf(stderr,
          "3d avoiding - didn't find a free position for the internode.\n");
}

/****************************************************************************/
void DetermineResponse(grid_type *grid, Cmodule_type *two_modules,
                       CTURTLE *tu) {
  int x, y, z, i, l;
  float val, gw, sgw, v;
  float vec[3] = {0.0, -1.0, 0.0}, grvec[3];
  float len, rot[3], q[4], bendsin;
  CELL_TYPE *cell;

  if (two_modules[0].num_params == 0) {
    if (verbose)
      fprintf(stderr, "Soil - no room for the response (no parameter).\n");
    return; /* nothing required + no room for the response */
  }

  if (tu->positionC < 2) {
    fprintf(stderr, "Soil - turtle position wasn't sent to the environment.\n");
    return;
  }

  val = two_modules[0].params[0].value; /* how much nutrients is desired? */

  two_modules[0].params[0].value = 0; /* no growth */
  two_modules[0].params[0].set = 1;

  x = floor((tu->position[0] - grid->pos[X]) / grid->range[X] *
            (double)(grid->size[X]));
  y = floor((tu->position[1] - grid->pos[Y]) / grid->range[Y] *
            (double)(grid->size[Y]));
  z = floor((tu->position[2] - grid->pos[Z]) / grid->range[Z] *
            (double)(grid->size[Z]));

  if ((x < 0) || (x >= grid->size[X]) || (y < 0) || (y >= grid->size[Y]) ||
      (z < 0) || (z >= grid->size[Z])) {
    if (verbose)
      fprintf(stderr, "Soil - node [%d,%d,%d] is out of the range.\n", x, y, z);

    /* set obstacle avoiding vector to 0 */
    for (i = 4; i < two_modules[0].num_params; i++) {
      two_modules[0].params[i].value = 0;
      two_modules[0].params[i].set = 1;
    }
    return;
  }

  cell = grid->data + z * grid->size[X] * grid->size[Y] + y * grid->size[X] + x;
  v = cell->val;

  switch (cell->flag) {
  case OBSTACLE:
    val = v = 0;
    break;
  case SOURCE:
    /* even source can be depleted in one step */
  case DATA:
    if (depletion == 0)
      val = v;
    else {
      if (val >= v)
        val = v;

      /* update the grid value */
      cell->val -= val;

      if (keep_depl_cells)
        /* mark so diffusion won't change the value in this cell */
        cell->flag |= CONSUMER;
    }
  }

  two_modules[0].params[0].value = val;
  two_modules[0].params[0].set = 1;

  if (verbose) {
    fprintf(stderr, "Soil - value in node [%d,%d,%d] was %f, %d, %d.", x, y, z,
            v, cell->flag, cell->obstacle[0]);
    fprintf(stderr, " Returned value is %f.\n", val);
  }

  if (grid->size[Z] == 1)
    SolveAvoiding2D(grid, x, y, &two_modules[0], tu);
  else
    SolveAvoiding3D(grid, x, y, z, &two_modules[0], tu);

  if (two_modules[0].num_params <= 1) {
    if (verbose)
      fprintf(stderr,
              "Soil - only one parameter, no room for the direction.\n");
    return;
  }

  /* get the geotropic weight */
  gw = two_modules[0].params[1].value;

  if (two_modules[0].num_params <= 2) {
    if (verbose)
      fprintf(stderr,
              "Soil - only two parameters, no room for the direction.\n");
    return;
  }

  /* get the soil gradient vector */
  sgw = two_modules[0].params[2].value;

  grvec[0] =
      (GetCell(grid, x + 1, y, z)->val - GetCell(grid, x - 1, y, z)->val) /
      (2.0 * (grid->max - grid->min));
  grvec[1] =
      (GetCell(grid, x, y + 1, z)->val - GetCell(grid, x, y - 1, z)->val) /
      (2.0 * (grid->max - grid->min));
  grvec[2] =
      (GetCell(grid, x, y, z + 1)->val - GetCell(grid, x, y, z - 1)->val) /
      (2.0 * (grid->max - grid->min));

  if (verbose) {
    fprintf(stderr, "Soil - obtained soil gradient weight: %g.\n", sgw);
    fprintf(stderr, "Soil - soil gradient vector: (%g,%g,%g).\n", grvec[0],
            grvec[1], grvec[2]);
  }

  /* get the geotropism vector */
  /* get the appropriate layer */
  l = 0;
  val = grid->range[Y] - (tu->position[1] - grid->pos[Y]);

  while ((val -= layer[l].thickness) >= 0.0)
    if (++l >= num_layers)
      break;

  if (l >= num_layers)
    vec[0] = vec[1] = vec[2] = 0.0;
  else if (layer[l].geotropic_angle != 0.0) {
    CrossProduct(tu->heading, vec, rot);

    if ((len = rot[0] * rot[0] + rot[1] * rot[1] + rot[2] * rot[2]) == 0.0) {
      rot[0] = rot[1] = 0.0;
      rot[2] = 1.0;
    } else {
      len = 1.0 / sqrt(len);
      rot[0] *= len;
      rot[1] *= len;
      rot[2] *= len;
    }
    /* rot - normalized vector of rotation */

    q[0] = cos(layer[l].geotropic_angle);
    q[1] = (bendsin = sin(layer[l].geotropic_angle)) * rot[0];
    q[2] = bendsin * rot[1];
    q[3] = bendsin * rot[2];

    rot_by_quat(vec, q);
  }
  if (verbose) {
    fprintf(stderr, "Soil - obtained geotropic weight: %g.\n", gw);
    fprintf(stderr, "Soil - geotropic vector: (%g,%g,%g).\n", vec[0], vec[1],
            vec[2]);
  }

  for (i = 0; i < 3; i++) {
    two_modules[0].params[i + 1].value = gw * vec[i] + sgw * grvec[i];
    two_modules[0].params[i + 1].set = 1;
  }

  if (verbose)
    fprintf(stderr, "Soil - returned vector: (%g,%g,%g).\n",
            two_modules[0].params[1].value, two_modules[0].params[2].value,
            two_modules[0].params[3].value);
}

/****************************************************************************/
/* Sets all sources to a predefined value. */
void SetSourcesToValue(grid_type *grid, DATA_TYPE val) {
  int x, y, z;
  CELL_TYPE *ptr = grid->data;

  /* for all cels */
  for (z = 0; z < grid->size[Z]; z++)
    for (y = 0; y < grid->size[Y]; y++)
      for (x = 0; x < grid->size[X]; x++) {
        if (ptr->flag == SOURCE)
          ptr->val = val;
        ptr++;
      }
}

/****************************************************************************/
/* when steps != -1, the simulation is carried for 'steps' steps. Otherwise
   it runs as long as any of the new values is more than the 'tolerance' from
   the previous value in a cell.
   */
void SimulateDiffusion(grid_type *grid, int steps, DATA_TYPE tolerance) {
  CELL_TYPE *new_data, *temp_ptr;
  char first_z, last_z, first_y, last_y, first_x, last_x;
  DATA_TYPE updated_val, old_val, new_val;
  DATA_TYPE max_tolerance;
  CELL_TYPE *ptr, *act_cell;
  int x, y, z, cnt, i;
  char completed;
  int zstep = grid->size[X] * grid->size[Y];
  int ystep = grid->size[X];
  int step = 0;

  if (grid->size[Z] * grid->size[Y] * grid->size[X] <= 1) {
    fprintf(stderr,
            "Soil difusion - grid is too small to simulate diffusion.\n");
    return;
  }

  if (steps == 0)
    return;

  /* another grid is used to store resulting values */
  if ((new_data = (CELL_TYPE *)malloc(grid->size[X] * grid->size[Y] *
                                      grid->size[Z] * sizeof(CELL_TYPE))) ==
      NULL) {
    fprintf(stderr,
            "Soil diffusion - cannot allocate memory for the new grid!\n");
    return;
  }

  if (verbose)
    fprintf(stderr, "Soil - starting diffusion: %d steps, tolerance %f\n",
            steps, tolerance);

  first_z = 1;
  max_tolerance = tolerance;

  do {
    completed = 1;
    max_tolerance = 0;

    /* for all cells */
    for (z = 0; z < grid->size[Z]; z++) {
      last_z = z == (grid->size[Z] - 1);

      first_y = 1;
      for (y = 0; y < grid->size[Y]; y++) {
        last_y = y == (grid->size[Y] - 1);

        first_x = 1;

        for (x = 0; x < grid->size[X]; x++) {
          last_x = x == (grid->size[X] - 1);

          act_cell = GetCell(grid, x, y, z);

          /* check up to 6 possible face neighbors */
          if (act_cell->flag == DATA) {
            updated_val = 0.0;
            cnt = 0;
            if (!first_z) {
              ptr = GetCell(grid, x, y, z - 1);
              if (ptr->flag != OBSTACLE) {
                updated_val += ptr->val;
                cnt++;
              }
            }
            if (!last_z) {
              ptr = GetCell(grid, x, y, z + 1);
              if (ptr->flag != OBSTACLE) {
                updated_val += ptr->val;
                cnt++;
              }
            }
            if (!first_y) {
              ptr = GetCell(grid, x, y - 1, z);
              if (ptr->flag != OBSTACLE) {
                updated_val += ptr->val;
                cnt++;
              }
            }
            if (!last_y) {
              ptr = GetCell(grid, x, y + 1, z);
              if (ptr->flag != OBSTACLE) {
                updated_val += ptr->val;
                cnt++;
              }
            }
            if (!first_x) {
              ptr = GetCell(grid, x - 1, y, z);
              if (ptr->flag != OBSTACLE) {
                updated_val += ptr->val;
                cnt++;
              }
            }
            if (!last_x) {
              ptr = GetCell(grid, x + 1, y, z);
              if (ptr->flag != OBSTACLE) {
                updated_val += ptr->val;
                cnt++;
              }
            }

            /* average values in the neighboring cells */
            updated_val /= (DATA_TYPE)cnt;

            /* omega controlls the speed of convergence */
            updated_val =
                (1 - omega) * (old_val = GetCell(grid, x, y, z)->val) +
                omega * updated_val;

            if (steps == -1) {
              new_val = fabs((double)(updated_val - old_val));
              if (new_val > tolerance) {
                if (new_val > max_tolerance)
                  max_tolerance = new_val;

                new_val = updated_val;
                completed = 0;
              } else
                new_val = old_val;
            } else
              new_val = updated_val;

            /* keep the value within the range */
            /*	  if(new_val > grid->max) new_val = grid->max;
                  if(new_val < grid->min) new_val = grid->min; */
          } else
            new_val = act_cell->val;

          new_data[z * zstep + y * ystep + x].val = new_val;
          new_data[z * zstep + y * ystep + x].flag = act_cell->flag;
          for (i = 0; i < MAXOBSTACLESINCELL; i++)
            new_data[z * zstep + y * ystep + x].obstacle[i] =
                act_cell->obstacle[i];

          first_x = 0;
        }
        first_y = 0;
      }
      first_z = 0;
    }

    if (steps != -1)
      completed = --steps == 0;

    /* switch the grids */
    temp_ptr = grid->data;
    grid->data = new_data;
    new_data = temp_ptr;

    if (verbose)
      fprintf(stderr, "Finished diffusion step %d. Max tolerance: %f.\n",
              ++step, max_tolerance);
  } while (!completed);

  /* remove CONSUMER bit from each cell flag */
  ptr = grid->data;

  /* for all cels */
  for (z = 0; z < grid->size[Z]; z++)
    for (y = 0; y < grid->size[Y]; y++)
      for (x = 0; x < grid->size[X]; x++) {
        if (ptr->flag | (CONSUMER == ptr->flag))
          ptr->flag ^= CONSUMER;
        ptr++;
      }

  free(new_data);
}

/************************************************************************/
/*
   according to the current derivation step and given set of intervals
   determines whether output file(s) should be saved.
   */

static int SaveFiles(int current_step) {
  int ind;
  printf("Current step: %d - from: %d - step: %d - end %d\n",current_step, intervals[ind].from, intervals[ind].step, intervals[ind].to);
  if ((intervals[0].from <= 0) || (current_step == 0))
    return 1; /* no itervals, always save */

  ind = 0;

  for (;;) {
    
    if (current_step <= intervals[ind].to) {
      if (current_step < intervals[ind].from)
        return 0;

      if (intervals[ind].step <= 1)
        return 1;

      return ((current_step - intervals[ind].from) % intervals[ind].step) == 0;
    }

    if (++ind >= MAXANIMINTERVALS)
      return 0;

    if (intervals[ind].from <= 0)
      return 0;
  }
}

/****************************************************************************/
void ProcessArguments(int argc, char **argv) {
  FILE *fp;
  int i, x, y, z;
  char *keywords[] = {
      "domain size",                 /*  0 */
      "position",                    /*  1 */
      "image",                       /*  2 */
      "array",                       /*  3 */
      "verbose",                     /*  4 */
      "layer thicknesses",           /*  5 */
      "geotropic angles",            /*  6 */
      "diffusion",                   /*  7 */
      "depletion",                   /*  8 */
      "relaxation factor",           /*  9 */
      "obstacles and sources",       /* 10 */
      "3D primitives",               /* 11 */
      "frame intervals",             /* 12 */
      "3D output",                   /* 13 */
      "concentration contour value", /* 14 */
      "contour material",            /* 15 */
      "source material",             /* 16 */
      "section material",            /* 17 */
      "output normals",              /* 18 */
      "keep depleted cells",         /* 19 */
      NULL                           /* the last item must be NULL! */
  };
  char *token, input_line[255];
  float val;
  int ind;

  /* defaults */
  verbose = 0;
  depletion = 1;
  omega = 0.5;
  obs_src = 0;
  output_type = POLYGONS;
  keep_depl_cells = 0;

  if ((grid.data = (CELL_TYPE *)malloc(sizeof(CELL_TYPE))) == NULL)
    exit(0);
  grid.size[X] = 1;
  grid.size[Y] = 1;
  grid.size[Z] = 1;
  grid.data->val = 6.0;
  grid.data->flag = DATA;

  grid.range[X] = 2.0;
  grid.range[Y] = 2.0;
  grid.range[Z] = 2.0;
  grid.pos[X] = -1.0;
  grid.pos[Y] = -1.0;
  grid.pos[Z] = -1.0;

  grid.min = 0;
  grid.max = 1;

  grid.outimagename = NULL;
  grid.out3Dname = NULL;

  num_layers = 1;
  layer[0].geotropic_angle = 0;
  layer[0].thickness = grid.range[Y];

  diff_steps = 0; /* no diffusion */
  diff_tolerance = 0.001;

  concentration_contour_value = 0.5;
  concentration_alpha = 0.2;

  contour_mat = NULL;
  source_mat = NULL;
  section_mat = NULL;

  output_normals = 0;

  a = 0.03;
  n = 1.5;
  h = -490;

  if (argc == 1) {
    printf("Soil - not enough arguments!\n"
           "USAGE: soil -e environment_file soil_spec_file\n");

    exit(0);
  }

  /* read in environment file */
  if ((fp = fopen(argv[1], "r")) == NULL)
    fprintf(stderr, "Soil - cannot open specification file %s.\n", argv[1]);
  else {
    /* process the file line by line */
    while (!feof(fp)) {
      /* get the whole line */
      if (fgets(input_line, 255, fp) == NULL)
        break;

      /* get the keyword */
      token = strtok(input_line, "\t:");

      /* look for a keyword in the table */
      i = 0;
      while (keywords[i] != NULL) {
        if (strcmp(keywords[i], token) == 0)
          break;
        i++;
      }

      if (keywords[i] == NULL) {
        fprintf(stderr,
                "Soil - unknown directive %s in the specification file.\n",
                token);
        continue;
      }

      switch (i) {
      case 0: /* domain size - range */
        token = strtok(NULL, "x,; \t:\n");
        if (token == NULL)
          break;
        grid.range[X] = atof(token);
        token = strtok(NULL, "x,; \t:\n");
        if (token == NULL)
          break;
        grid.range[Y] = atof(token);
        token = strtok(NULL, "x,; \t:\n");
        if (token == NULL)
          break;
        grid.range[Z] = atof(token);
        break;

      case 1: /* position */
        token = strtok(NULL, "x,; \t:\n");
        if (token == NULL)
          break;
        grid.pos[X] = atof(token);
        token = strtok(NULL, "x,; \t:\n");
        if (token == NULL)
          break;
        grid.pos[Y] = atof(token);
        token = strtok(NULL, "x,; \t:\n");
        if (token == NULL)
          break;
        grid.pos[Z] = atof(token);
        break;

      case 2: /* input from an image - only 2d */
        /* min and max */
        token = strtok(NULL, "x,; \t:\n");
        if (token == NULL)
          break;
        grid.min = atof(token);
        token = strtok(NULL, "x,; \t:\n");
        if (token == NULL)
          break;
        grid.max = atof(token);

        /* image input */
        token = strtok(NULL, ",; \t:\n");
        if (token == NULL)
          break;
        LoadInitialData(token, &grid);

        /* image output */
        token = strtok(NULL, ",; \t:\n");
        if (token == NULL)
          break;
        grid.outimagename = strdup(token);
        break;

      case 3: /* input from an array */
              /* number of nodes - size */
        token = strtok(NULL, "x,; \t:\n");
        if (token == NULL)
          break;
        grid.size[X] = atoi(token);
        token = strtok(NULL, "x,; \t:\n");
        if (token == NULL)
          break;
        grid.size[Y] = atoi(token);
        token = strtok(NULL, "x,; \t:\n");
        if (token == NULL)
          break;
        grid.size[Z] = atoi(token);

        /* min and max */
        token = strtok(NULL, "x,; \t:\n");
        if (token != NULL)
          grid.min = atof(token);
        token = strtok(NULL, "x,; \t:\n");
        if (token != NULL)
          grid.max = atof(token);

        /* image output */
        token = strtok(NULL, ",; \t:\n");
        if (token != NULL)
          grid.outimagename = strdup(token);

        /* allocate the grid */
        if (grid.size[X] * grid.size[Y] * grid.size[Z] == 0)
          break;
        if ((grid.data = (CELL_TYPE *)malloc(grid.size[X] * grid.size[Y] *
                                             grid.size[Z] *
                                             sizeof(CELL_TYPE))) == NULL) {
          fprintf(stderr, "Soil - cannot allocate memory for the grid!\n");
          break;
        }

        for (z = 0; z < grid.size[Z]; z++)
          for (y = 0; y < grid.size[Y]; y++)
            for (x = 0; x < grid.size[X]; x++) {
              token = strtok(NULL, " \t:\n,x");
              if (token == NULL) {
                if (fgets(input_line, 255, fp) == NULL)
                  break;
                token = strtok(input_line, " \t:\n,x");
              }
              val = atof(token);

              grid.data[z * grid.size[X] * grid.size[Y] +
                        (grid.size[Y] - 1 - y) * grid.size[X] + x]
                  .val = val;

              if (obs_src)
                grid.data[z * grid.size[X] * grid.size[Y] +
                          (grid.size[Y] - 1 - y) * grid.size[X] + x]
                    .flag = (val <= grid.min ? OBSTACLE
                                             : val >= grid.max ? SOURCE : DATA);
              else
                grid.data[z * grid.size[X] * grid.size[Y] +
                          (grid.size[Y] - 1 - y) * grid.size[X] + x]
                    .flag = DATA;
            }
        break;

      case 4: /* verbose */
        token = strtok(NULL, "x,; \t:\n");
        if (token == NULL)
          break;
        if (strcmp(token, "on") == 0)
          verbose = 1;
        break;

      case 5: /* layer thicknesses */
        num_layers = 0;
        for (;;) {
          token = strtok(NULL, "+,; \t:\n");
          if (token == NULL)
            break;

          layer[num_layers++].thickness = atof(token);
        }
        break;

      case 6: /* geotropic angle */
        for (i = 0; i < num_layers; i++) {
          token = strtok(NULL, ",; \t:\n");
          if (token == NULL)
            break;
          layer[i].geotropic_angle = atof(token);
        }
        for (; i < num_layers; i++)
          layer[i].geotropic_angle = 0;
        break;

      case 7: /* diffusion */
        token = strtok(NULL, "x,; \t:\n");
        if (token == NULL)
          break;
        diff_steps = atoi(token);
        token = strtok(NULL, "x,; \t:\n");
        if (token == NULL)
          break;
        diff_tolerance = atof(token);
        break;

      case 8: /* depletion */
        token = strtok(NULL, "x,; \t:\n");
        if (token == NULL)
          break;
        if (strcmp(token, "off") == 0)
          depletion = 0;
        break;

      case 9: /* relaxation factor */
        token = strtok(NULL, "x,; \t:\n");
        if (token == NULL)
          break;
        omega = atof(token);
        break;

      case 10: /* obstacles and sources */
        token = strtok(NULL, "x,; \t:\n");
        if (token == NULL)
          break;
        if (strcmp(token, "on") == 0)
          obs_src = 1;
        break;

      case 11: /* input from an primitive file */
               /* number of nodes - size */
        token = strtok(NULL, "x,; \t:\n");
        if (token == NULL)
          break;
        grid.size[X] = atoi(token);
        token = strtok(NULL, "x,; \t:\n");
        if (token == NULL)
          break;
        grid.size[Y] = atoi(token);
        token = strtok(NULL, "x,; \t:\n");
        if (token == NULL)
          break;
        grid.size[Z] = atoi(token);

        /* allocate the grid */
        if (grid.size[X] * grid.size[Y] * grid.size[Z] == 0)
          break;
        if ((grid.data = (CELL_TYPE *)malloc(grid.size[X] * grid.size[Y] *
                                             grid.size[Z] *
                                             sizeof(CELL_TYPE))) == NULL) {
          fprintf(stderr, "Soil - cannot allocate memory for the grid!\n");
          break;
        }

        /* min and max */
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        grid.min = atof(token);
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        grid.max = atof(token);

        /* file input */
        if ((token = strtok(NULL, ",; \t:\n")) == NULL)
          break;
        ReadPrimitiveFile(token, 1);

        while ((token = strtok(NULL, ",; \t:\n")) != NULL)
          if (strcmp(token, "O") != 0)
            ReadPrimitiveFile(token, 0);
          else
            break;

        FillGrid(&grid);

        /* file output */
        if ((token = strtok(NULL, ",; \t:\n")) == NULL)
          break;
        grid.out3Dname = strdup(token);
        break;
      case 12: /* frame intervals */
        intervals[0].from = 0;

        if ((token = strtok(NULL, ",\n")) == NULL)
          break;

        ind = 0;
        for (;;) {
          intervals[ind].from = intervals[ind].to = intervals[ind].step = 0;

          sscanf(token, "%d-%d step %d", &intervals[ind].from,
                 &intervals[ind].to, &intervals[ind].step);

          if (intervals[ind].to == 0)
            intervals[ind].to = intervals[ind].from;

          if (++ind >= MAXANIMINTERVALS) {
            fprintf(stderr,
                    "Warning: too many frame intervals, the rest ignored.\n");
            break;
          }
          if ((token = strtok(NULL, ",\n")) == NULL)
            break;
        }
      case 13: /* 3Doutput */
        token = strtok(NULL, ",; \t:\n");
        if (token == NULL)
          break;
        if (strcmp(token, "triangles") == 0) {
          if (verbose)
            fprintf(stderr, "Soil - output type: triangles\n");

          output_type = TRIANGLES;
        }
        break;

      case 14: /* concentration contour value */
        token = strtok(NULL, "x,; \t:\n");
        if (token == NULL)
          break;
        concentration_contour_value = atof(token);
        token = strtok(NULL, "x,; \t:\n");
        if (token == NULL)
          break;
        concentration_alpha = atof(token);
        break;

      case 15: /* countour material */
        token = strtok(NULL, "\n");
        if (token == NULL)
          break;
        contour_mat = strdup(token);
        break;

      case 16: /* source material */
        token = strtok(NULL, "\n");
        if (token == NULL)
          break;
        source_mat = strdup(token);
        break;

      case 17: /* section material */
        token = strtok(NULL, "\n");
        if (token == NULL)
          break;
        section_mat = strdup(token);
        break;

      case 18: /* outputnormals */
        token = strtok(NULL, "x,; \t:\n");
        if (token == NULL)
          break;
        if (strcmp(token, "yes") == 0)
          output_normals = 1;
        if (strcmp(token, "old") == 0)
          output_normals = 2;
        break;

      case 19: /* keep depleted cells */
        token = strtok(NULL, "x,; \t:\n");
        if (token == NULL)
          break;
        if (strcmp(token, "on") == 0)
          keep_depl_cells = 1;
        break;
      }
    }
  }

  if (verbose) {
    fprintf(stderr, "Soil - domain size:     %gx%gx%g\n", grid.range[X],
            grid.range[Y], grid.range[Z]);
    fprintf(stderr, "Soil - position:       (%g,%g,%g)\n", grid.pos[X],
            grid.pos[Y], grid.pos[Z]);
    fprintf(stderr, "Soil - number of nodes: %dx%dx%d\n", grid.size[X],
            grid.size[Y], grid.size[Z]);

#ifdef XXX
    if (grid.data != NULL) {
      fprintf(stderr, "Soil - grid data:\n");
      for (z = 0; z < grid.size[Z]; z++)
        for (y = 0; y < grid.size[Y]; y++)
          for (x = 0; x < grid.size[X]; x++)
            fprintf(stderr, " %.2g", grid.data[y * grid.size[X] + x]);
      fprintf(stderr, "\n");
    }
#endif

    fprintf(stderr, "Soil - min: %f, max: %f.\n", grid.min, grid.max);

    fprintf(stderr, "Soil - diffusion: steps %d, tolerance %g\n", diff_steps,
            diff_tolerance);

    fprintf(stderr, "Soil - %d layers:\n       thicknesses:", num_layers);
    for (i = 0; i < num_layers; i++)
      fprintf(stderr, " %g", layer[i].thickness);
    fprintf(stderr, "\n       geotropic angles:");
    for (i = 0; i < num_layers; i++)
      fprintf(stderr, " %g", layer[i].geotropic_angle);
    fprintf(stderr, "\n");

    if (grid.outimagename != NULL)
      fprintf(stderr, "Output image name: %s\n.", grid.outimagename);

    if (grid.out3Dname != NULL)
      fprintf(stderr, "Output 3D name: %s.\n", grid.out3Dname);

    if (intervals[0].from > 0) {
      ind = 0;
      fprintf(stderr, "Frame intervals: ");

      while (intervals[ind].from > 0) {
        fprintf(stderr, "%d", intervals[ind].from);
        if (intervals[ind].to > intervals[ind].from) {
          fprintf(stderr, " - %d step ", intervals[ind].to);
          if (intervals[ind].step > 0)
            fprintf(stderr, "%d", intervals[ind].step);
          else
            fprintf(stderr, "1");
        }
        fprintf(stderr, ", ");
        if (++ind >= MAXANIMINTERVALS)
          break;
      }
      fprintf(stderr, "\n");
    }

    fprintf(stderr, "\nSoil - specification file processed.\n\n");
  }

  /* convert angles to radians */
  for (i = 0; i < num_layers; i++)
    layer[i].geotropic_angle *= M_PI / 180.0;
}

/****************************************************************************/
void MainLoop(void) {
  Cmodule_type two_modules[2];
  unsigned long module_id;
  int master;
  CTURTLE turtle;
  int current_step;
  int in;
  int i;

  /* infinite loop - until signal 'exit' comes */
  for (;;) {
    in = 0;

    CSBeginTransmission();

    if (verbose)
      fprintf(stderr, "soil - start processing data.\n");

    /* process the data */
    while (CSGetData(&master, &module_id, two_modules, &turtle)) {
      in = 1;

      if (verbose) {
        fprintf(stderr, "Soil - comm. symbol has %d parameters:\n      ",
                two_modules[0].num_params);
        for (i = 0; i < two_modules[0].num_params; i++)
          fprintf(stderr, " %g", two_modules[0].params[i].value);
        fprintf(stderr, "\n");

        fprintf(stderr, "Soil - next symbol '%s' has %d parameters:\n      ",
                two_modules[1].symbol, two_modules[1].num_params);
        for (i = 0; i < two_modules[1].num_params; i++)
          fprintf(stderr, " %g", two_modules[1].params[i].value);
        fprintf(stderr, "\n");
      }

      for (i = 0; i < two_modules[0].num_params; i++)
        two_modules[0].params[i].set = 0;

      DetermineResponse(&grid, two_modules, &turtle);

      CSSendData(master, module_id, &two_modules[0]);
    }
    /* doesn't send anything to cpfg */

    current_step = module_id;

    if (in) {
      if (obs_src)
        SetSourcesToValue(&grid, grid.max);

      SimulateDiffusion(&grid, diff_steps, diff_tolerance);

      if (SaveFiles(current_step + 1)) {
        if (grid.outimagename != NULL) {
          if (verbose)
            fprintf(stderr, "Soil - saving image %s.\n", grid.outimagename);
          SaveImage(&grid);
        }

        if (grid.out3Dname != NULL) {
          OutputPrimitives(&grid);
          if (verbose)
            fprintf(stderr, "Soil - saving 3D file %s.\n", grid.out3Dname);
        }
      }

      if (verbose)
        fprintf(stderr, "Soil - data processed.\n");
    }

    /* End transmission returns 1 when the process is requested to exit */
    if (CSEndTransmission())
      break;
  }
}

/****************************************************************************/
int main(int argc, char **argv) {
  char *process_name = strdup(argv[0]);

  /* initialize the communication as the very first thing */
  CSInitialize(&argc, &argv);

  ProcessArguments(argc, argv);

  fprintf(stderr, "Field process %s initialized.\n", process_name);

  MainLoop();

  FreeFieldStructures();

  fprintf(stderr, "Field process %s exiting.\n", process_name);

  /* should be the last function called */
  CTerminate();

  return 1;
}
