#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "soil2d.h"
#include "triangulate.h"

extern char verbose;
extern char output_normals;

struct point_type {
  float pt[3];
  float norm[3];
  char edge;
};

#define MAX_POLS 4

struct polygon_type {
  struct point_type pts[MAX_POLS][6]; /* max 6 points with normals */
  char orient[MAX_POLS][4]; /* orientation of triangles - max 4 of them */
  int num_pts[MAX_POLS];
};

typedef struct polygon_type polygon_type;

struct polygon_type1 {
  struct point_type pts[6]; /* max 6 points with normals */
};

typedef struct polygon_type1 polygon_type1;

/*************************************************************************/
static void Normalize(float *vec) {
  float len;
  int i;

  if ((len = sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2])) > 0)
    for (i = 0; i < 3; i++)
      vec[i] /= len;
}

/*************************************************************************/
int GetTriangleNormal(struct point_type *pt1, struct point_type *pt2,
                      struct point_type *pt3, float *norm) {
  int i;
  float vec1[3], vec2[3];

  for (i = X; i <= Z; i++) {
    vec1[i] = pt2->pt[i] - pt1->pt[i];
    vec2[i] = pt3->pt[i] - pt1->pt[i];
  }
  /* get the normal */
  norm[0] = vec1[1] * vec2[2] - vec1[2] * vec2[1];
  norm[1] = vec1[2] * vec2[0] - vec1[0] * vec2[2];
  norm[2] = vec1[0] * vec2[1] - vec1[1] * vec2[0];

  Normalize(norm);

  for (i = 0; i < 3; i++)
    vec1[i] = pt1->norm[i] + pt2->norm[i] + pt3->norm[i];

  if (vec1[0] * norm[0] + vec1[1] * norm[1] + vec1[2] * norm[2] > 0) {
    for (i = 0; i < 3; i++)
      norm[i] *= -1.0;
    return -1; /* switch orientation when drawing */
  }

  return 1;
}

/*************************************************************************/
void SetVertexNormal(grid_type *grid, int x, int y, int z, float *norm) {
  norm[X] =
      (GetCell(grid, x + 1, y, z)->val - GetCell(grid, x - 1, y, z)->val) /
      2.0 / grid->range[X];
  norm[Y] =
      (GetCell(grid, x, y + 1, z)->val - GetCell(grid, x, y - 1, z)->val) /
      2.0 / grid->range[Y];
  norm[Z] =
      (GetCell(grid, x, y, z + 1)->val - GetCell(grid, x, y, z - 1)->val) /
      2.0 / grid->range[Z];
}

/*************************************************************************/
void FindPointsNormal(polygon_type *ptr, float *pt, float *norm) {
  int p, c, pol;
  float mindist = 0.0001;

  for (pol = 0; pol < MAX_POLS; pol++)
    for (p = 0; p < ptr->num_pts[pol]; p++) {
      if ((fabs(ptr->pts[pol][p].pt[0] - pt[0]) < mindist) &&
          (fabs(ptr->pts[pol][p].pt[1] - pt[1]) < mindist) &&
          (fabs(ptr->pts[pol][p].pt[2] - pt[2]) < mindist)) {
        /* found */
        for (c = 0; c < 3; c++)
          norm[c] = ptr->pts[pol][p].norm[c];

        if (verbose) {
          fprintf(stderr, "neighbor normal: (%g,%g,%g)\n", norm[0], norm[1],
                  norm[2]);
        }
        return;
      }
    }

  /* point is not in the neighbour */
  for (c = 0; c < 3; c++)
    norm[c] = 0;
}

/*************************************************************************/
void OutputLayer(polygon_type *three_layers, grid_type *grid, FILE *fp,
                 int lower, int middle, int upper) {
  int x, y, c, p, i, n, pol;
  polygon_type *ptr, *ptr1, *ptr2, *ptr3;
  float sum_norm[6][3], norm[3];
  int edge;
  static int neighbours[12][3] = {
      /* edge neighbors (voxels in x,y,z direction)*/
      {0, -1, -1}, /* 0 */
      {1, 0, -1},  /* 1 */
      {0, 1, -1},  /* 2 */
      {-1, 0, -1}, /* 3 */
      {-1, -1, 0}, /* 4 */
      {1, -1, 0},  /* 5 */
      {1, 1, 0},   /* 6 */
      {-1, 1, 0},  /* 7 */
      {0, -1, 1},  /* 8 */
      {1, 0, 1},   /* 9 */
      {0, 1, 1},   /*10 */
      {-1, 0, 1}   /*11 */
  };

  for (y = 0; y < grid->size[Y] - 1; y++)
    for (x = 0; x < grid->size[X] - 1; x++) {

      ptr = three_layers + (middle * grid->size[Y] + y) * grid->size[X] + x;

      for (pol = 0; pol < MAX_POLS; pol++) {
        if (ptr->num_pts[pol] == 0)
          break;

        /* for all points in the voxel */
        for (p = 0; p < ptr->num_pts[pol]; p++) {
          /* sum the normals from the same point stored in neigboring voxels
             (up to 4 neighbors). Store them in 'sum_norm' */

          /* start with the current voxel */
          for (c = 0; c < 3; c++)
            sum_norm[p][c] = ptr->pts[pol][p].norm[c];

          if (verbose) {
            fprintf(stderr, "point normal: (%g,%g,%g)\n", sum_norm[p][0],
                    sum_norm[p][1], sum_norm[p][2]);
          }

          edge = ptr->pts[pol][p].edge; /* points edge */

          /* get the proper neighbours */

          if (neighbours[edge][Z] == 0) {
            /* [Y] and [X] must be nonzero */

            ptr1 = ptr + neighbours[edge][Y] * grid->size[X];

            /* check whether ptr1 points inside the grid */
            if ((y + neighbours[edge][Y] < 0) ||
                (y + neighbours[edge][Y] >= grid->size[Y] - 1))
              ptr1 = NULL;

            ptr2 = ptr + neighbours[edge][X];

            /* check whether ptr2 points inside the grid */
            if ((x + neighbours[edge][X] < 0) ||
                (x + neighbours[edge][X] >= grid->size[X] - 1))
              ptr2 = NULL;
          } else {
            ptr1 = NULL;
            /* check whether the upper or lower layer exists */
            if (neighbours[edge][Z] == 1) {
              if (upper != -1)
                ptr1 = three_layers +
                       (upper * grid->size[Y] + y) * grid->size[X] + x;
            } else
                /* neigbours[edge][Z] is -1 */
                if (lower != -1)
              ptr1 = three_layers +
                     (lower * grid->size[Y] + y) * grid->size[X] + x;

            if (neighbours[edge][Y] == 0) {
              /* [X] must be nonzero */
              ptr2 = ptr + neighbours[edge][X];

              /* check whether ptr2 points inside the grid */
              if ((x + neighbours[edge][X] < 0) ||
                  (x + neighbours[edge][X] >= grid->size[X] - 1))
                ptr2 = NULL;
            } else {
              /* [Y] is nonzero */
              ptr2 = ptr + neighbours[edge][Y] * grid->size[X];

              /* check whether ptr2 points inside the grid */
              if ((y + neighbours[edge][Y] < 0) ||
                  (y + neighbours[edge][Y] >= grid->size[Y] - 1))
                ptr2 = NULL;
            }
          }

          if ((ptr1 != NULL) && (ptr2 != NULL))
            ptr3 = ptr2 - ptr + ptr1;
          else
            ptr3 = NULL;

          n = 0;

          if (ptr1 != NULL) {
            FindPointsNormal(ptr1, ptr->pts[pol][p].pt, norm);
            for (c = 0; c < 3; c++)
              sum_norm[p][c] += norm[c];

            if ((norm[0] != 0) || (norm[1] != 0) || (norm[2] != 0))
              n++;
          }

          if (ptr2 != NULL) {
            FindPointsNormal(ptr2, ptr->pts[pol][p].pt, norm);
            for (c = 0; c < 3; c++)
              sum_norm[p][c] += norm[c];
            if ((norm[0] != 0) || (norm[1] != 0) || (norm[2] != 0))
              n++;
          }
          if (ptr3 != NULL) {
            FindPointsNormal(ptr3, ptr->pts[pol][p].pt, norm);
            for (c = 0; c < 3; c++)
              sum_norm[p][c] += norm[c];
            if ((norm[0] != 0) || (norm[1] != 0) || (norm[2] != 0))
              n++;
          }
          Normalize(sum_norm[p]);

          c = ptr1 != NULL ? 1 : 0;
          if (ptr2 != NULL)
            c++;
          if (ptr3 != NULL)
            c++;

          if (verbose) {
            fprintf(stderr,
                    "polygonuv %.5g %.5g %.5g %.5g %.5g %.5g "
                    "(found in %d neighbors)\n",
                    ptr->pts[pol][p].pt[0], ptr->pts[pol][p].pt[1],
                    ptr->pts[pol][p].pt[2], -sum_norm[p][0], -sum_norm[p][1],
                    -sum_norm[p][2], n);
          }
        }

        /* output the points */
        for (c = 2; c < ptr->num_pts[pol]; c++) {
          fprintf(fp, "polygonuv\n");
          fprintf(fp, "%.5g %.5g %.5g %.5g %.5g %.5g\n", ptr->pts[pol][0].pt[0],
                  ptr->pts[pol][0].pt[1], ptr->pts[pol][0].pt[2],
                  -sum_norm[0][0], -sum_norm[0][1], -sum_norm[0][2]);

          if (ptr->orient[pol][c - 2] == 1)
            for (i = -1; i <= 0; i++)
              fprintf(fp, "%.5g %.5g %.5g %.5g %.5g %.5g\n",
                      ptr->pts[pol][c + i].pt[0], ptr->pts[pol][c + i].pt[1],
                      ptr->pts[pol][c + i].pt[2], -sum_norm[c + i][0],
                      -sum_norm[c + i][1], -sum_norm[c + i][2]);
          else
            for (i = 0; i >= -1; i--)
              fprintf(fp, "%.5g %.5g %.5g %.5g %.5g %.5g\n",
                      ptr->pts[pol][c + i].pt[0], ptr->pts[pol][c + i].pt[1],
                      ptr->pts[pol][c + i].pt[2], -sum_norm[c + i][0],
                      -sum_norm[c + i][1], -sum_norm[c + i][2]);
        }
      }
    }
}

/*************************************************************************/
/* determine startpoint of an edge */
int getstart(int i, grid_type *grid) {
  int c;
  int ret;

  ret = 0;
  c = i % 4;
  if ((c == 1) || (c == 2))
    ret += 1;
  if ((c == 2) || (c == 3))
    ret += grid->size[X];
  if (i >= 8)
    ret += grid->size[X] * grid->size[Y];

  return ret;
}

/*************************************************************************/
/* determine end-point of an edge */
int getend(int i, grid_type *grid) {
  int c;
  int ret;

  ret = 0;

  if ((i >= 4) && (i < 8))
    c = i % 4;
  else
    c = (i + 1) % 4;

  if ((c == 1) || (c == 2))
    ret += 1;
  if ((c == 2) || (c == 3))
    ret += grid->size[X];
  if (i >= 4)
    ret += grid->size[X] * grid->size[Y];

  return ret;
}

/*************************************************************************/
float pt[3], node_size[3];
float P[12];         /* to store position (0-1)of intersections on each edge */
int dir[12];         /* to store direction of search for neighbouring
                        intersection */
float normals[8][3]; /* normals in 8 cube verices */

/*************************************************************************/
void output_point(int i, struct point_type *outpt) {
  float p1[3], p2[3];
  int c, from, to;

  for (c = X; c <= Z; c++)
    p1[c] = p2[c] = pt[c];

  /* determine p1 - start point if the edge 'i' (the cube is positioned in
     (0,0,0)) */
  c = i % 4;
  if ((c == 1) || (c == 2))
    p1[X] += node_size[X];
  if ((c == 2) || (c == 3))
    p1[Y] += node_size[Y];
  if (i >= 8)
    p1[Z] += node_size[Z];

  if ((i >= 4) && (i < 8))
    c = i % 4;
  else
    c = (i + 1) % 4;

  /* determine p2 - end point if the edge 'i' (the cube is positioned in
     (0,0,0)) */
  if ((c == 1) || (c == 2))
    p2[X] += node_size[X];
  if ((c == 2) || (c == 3))
    p2[Y] += node_size[Y];
  if (i >= 4)
    p2[Z] += node_size[Z];

  /* edge-contour intersection */
  outpt->pt[X] = p1[X] + P[i] * (p2[X] - p1[X]);
  outpt->pt[Y] = p1[Y] + P[i] * (p2[Y] - p1[Y]);
  outpt->pt[Z] = p1[Z] + P[i] * (p2[Z] - p1[Z]);

  /* store edge number - used to get to neighbors for setting normal */
  outpt->edge = i;

  if (output_normals) {
    /* index of the edge start point */
    from = i >= 4 ? i - 4 : i;
    /* index of the edge end point */
    to = (i + 1) % 4;
    if (i >= 8)
      to += 4;
    else if (i >= 4)
      to = 4 + (i % 4);

    /* interpolate between cube's vertices normals */
    for (c = 0; c < 3; c++)
      outpt->norm[c] = normals[from][c] * P[i] + (1 - P[i]) * normals[to][c];

    Normalize(outpt->norm);
  }
}

/*************************************************************************/
/* Stores polygon vertex 'ind' with its normal */
void StorePoint(polygon_type *ptr, int pol, int ind, float *pt, float *norm) {
  int i;

  for (i = 0; i < 3; i++) {
    ptr->pts[pol][ind].pt[i] = pt[i];
    ptr->pts[pol][ind].norm[i] = norm[i];
  }
}

/*************************************************************************/
/*
  The way edges are numbered:

      +---10----+
     /|        /|
   11 7       9 |
   /  |      /  6
  +----8----+   |
  |   |     |   |
  4   |     5   |
  |   |     |   |
  |   +-----|-2-+
  |  /      |  /
  | 3       | 1
  |/        |/
  +----0----+

 */

int face[6][4][2] = {/* 6 faces times 4 edges with edge number and direction
                         with respect to dir[] */
                     {{0, 1}, {1, 1}, {2, 1}, {3, 1}},
                     {{4, -1}, {0, 1}, {5, 1}, {8, -1}},
                     {{5, -1}, {1, 1}, {6, 1}, {9, -1}},
                     {{6, -1}, {2, 1}, {7, 1}, {10, -1}},
                     {{4, 1}, {11, -1}, {7, -1}, {3, 1}},
                     {{8, 1}, {9, 1}, {10, 1}, {11, 1}}};

int faces_of_edge[12][2] = {{0, 1}, {0, 2}, {0, 3}, {0, 4}, {4, 1}, {1, 2},
                            {2, 3}, {3, 4}, {1, 5}, {2, 5}, {3, 5}, {4, 5}};

/*************************************************************************/
void OutputTriangulatedContours(FILE *fp, grid_type *grid,
                                float (*FindContour)(CELL_TYPE *,
                                                     CELL_TYPE *)) {
  int x, y, z;
  int x2, z2;
  int i, c, ind, num, f, aux, p, nums;
  CELL_TYPE *which_node, *start_node, *end_node;
  int last;
  char taken[12];
  polygon_type1 outpt; /* max six points with normals */
  float vec1[3], vec2[3], norm[3];
  int num_pols;

  polygon_type *three_layers; /* last three layers - to determine proper
                                 normals */
  polygon_type *ptr;
  int act_layer;
  float sum_norm[3];
  float tnormals[4][3]; /* triangle normals.
                           Max 6 points -> max 4 triangles */

  if (output_normals) {
    if (output_normals == 1) {
      /* allocate the array for three layers of polygons */
      if ((three_layers = (polygon_type *)malloc(
               3 * grid->size[Y] * grid->size[X] * sizeof(polygon_type))) ==
          NULL) {
        fprintf(stderr, "Soil - cannot allocate memory! "
                        "Less precise normals result.\n");
      }
    } else
      three_layers = NULL;

    act_layer = 0;
  }

  for (c = X; c <= Z; c++)
    node_size[c] = grid->range[c] / (float)grid->size[c];

  /* for all nodes */
  for (z = 0; z < grid->size[Z] - 1; z++) {
    pt[Z] = grid->pos[Z] + ((float)z) * grid->range[Z] / (float)grid->size[Z];

    for (y = 0; y < grid->size[Y] - 1; y++) {
      pt[Y] = grid->pos[Y] + ((float)y) * grid->range[Y] / (float)grid->size[Y];

      for (x = 0; x < grid->size[X] - 1; x++) {
        pt[X] =
            grid->pos[X] + ((float)x) * grid->range[X] / (float)grid->size[X];

        which_node = grid->data + z * grid->size[X] * grid->size[Y] +
                     y * grid->size[X] + x;

        {
          /* find all edges with an intersection */
          ind = -1; /* the first intersection  */
          num = 0;  /* number of intersections */
          for (i = 0; i < 12; i++) {
            taken[i] = 0;
            if ((P[i] = FindContour(start_node = which_node + getstart(i, grid),
                                    end_node = which_node + getend(i, grid))) !=
                -1)
            /* if((i<8)||(P[i]!=1)) */
            { /* ignore case when i>=8 and P[i]==1
                                    because such intersection would be
                                    considered twice */
              dir[i] = (start_node->val < end_node->val) ? 1 : -1;
              /* direction of search for a neighbouring intersection */
              num++;
              if (ind == -1)
                /* first intersection found */
                ind = i;
            }
          }

          if (output_normals) {
            /* get normals in all 8 cube vertices */
            i = 0;
            for (z2 = 0; z2 < 2; z2++) {
              for (x2 = 0; x2 < 2; x2++)
                SetVertexNormal(grid, x + x2, y, z + z2, normals[i++]);
              for (x2 = 1; x2 >= 0; x2--)
                SetVertexNormal(grid, x + x2, y + 1, z + z2, normals[i++]);
            }
          }

          num_pols = 0;

          if ((num > 0) && (num < 3)) {
            fprintf(stderr, "Soil - only %d edges intersect the contour!\n",
                    num);
            fprintf(stderr, "Soil - ind=%d\n", ind);
          }
          nums = num; /* store num - for debugging */

          for (;;) {
            if ((num >= 3) && (ind < 8)) {
              num = 0;

              /* output the first point indexed by ind */
              output_point(ind, outpt.pts + num++);
              last = ind;
              f = faces_of_edge[last][0];
              taken[ind] = 1;

              do {
                /* find edge 'last' in the current face */
                for (c = 0; c < 4; c++)
                  if (face[f][c][0] == last)
                    break;

                if (c == 4)
                  fprintf(stderr, "Soil - fatal: edge not found on a face.\n");

                /* try to find a neighbor on the same face in the given
                   direction dir[last]*/
                for (i = 1; i < 4; i++) {
                  aux = face[f][(c + i * dir[last] * face[f][c][1] + 4) % 4][0];

                  if (last != ind)
                    /* did it come to the starting point ? */
                    if (aux == ind) {
                      last = aux;
                      break;
                    }

                  if ((!taken[aux]) && (P[aux] != -1)) {
                    if (num > 6) {
                      fprintf(stderr,
                              "Error while outputing triangles! "
                              "More then 6 intersections with cube's edges!\n");
                      num = 6;
                      goto out; /* to get out of the while loop */
                    }
                    output_point(aux, outpt.pts + num++);

                    last = aux;
                    taken[last] = 1;
                    /* go to the neighbouring face */
                    if (f != faces_of_edge[last][0])
                      f = faces_of_edge[last][0];
                    else
                      f = faces_of_edge[last][1];
                    break;
                  }
                }
                if (i == 4) {
                  /*
                    fprintf(stderr,"Error while outputing triangles! "
                    "Unpredicted situation - contact the authors!\n");
                    */
                  break;
                }
              } while (last != ind);
            out:

              if ((num > 0) && (num < 3)) {
                fprintf(stderr, "Soil - only %d edges found!\n", num);
                fprintf(stderr, "Soil - ind=%d\n", ind);
              }

              if (output_normals) {
                if (three_layers == NULL) {
                  /* the old not so bad way */
                  /* output directly into the file */
                  for (c = 0; c <= num - 3; c++) {
                    for (i = X; i <= Z; i++) {
                      vec1[i] = outpt.pts[c + 1].pt[i] - outpt.pts[0].pt[i];
                      vec2[i] = outpt.pts[c + 2].pt[i] - outpt.pts[0].pt[i];
                    }
                    /* get the normal */
                    norm[0] = vec1[1] * vec2[2] - vec1[2] * vec2[1];
                    norm[1] = vec1[2] * vec2[0] - vec1[0] * vec2[2];
                    norm[2] = vec1[0] * vec2[1] - vec1[1] * vec2[0];

                    if (norm[0] * outpt.pts[0].norm[0] +
                            norm[1] * outpt.pts[0].norm[1] +
                            norm[2] * outpt.pts[0].norm[2] <=
                        0) {
                      fprintf(fp, "polygonuv\n");
                      fprintf(fp, "%.5g %.5g %.5g %.5g %.5g %.5g\n",
                              outpt.pts[0].pt[0], outpt.pts[0].pt[1],
                              outpt.pts[0].pt[2], outpt.pts[0].norm[0],
                              outpt.pts[0].norm[1], outpt.pts[0].norm[2]);

                      for (i = 1; i < 3; i++)
                        fprintf(
                            fp, "%.5g %.5g %.5g %.5g %.5g %.5g\n",
                            outpt.pts[c + i].pt[0], outpt.pts[c + i].pt[1],
                            outpt.pts[c + i].pt[2], outpt.pts[c + i].norm[0],
                            outpt.pts[c + i].norm[1], outpt.pts[c + i].norm[2]);
                    } else {
                      fprintf(fp, "polygonuv\n");
                      fprintf(fp, "%.5g %.5g %.5g %.5g %.5g %.5g\n",
                              outpt.pts[0].pt[0], outpt.pts[0].pt[1],
                              outpt.pts[0].pt[2], outpt.pts[0].norm[0],
                              outpt.pts[0].norm[1], outpt.pts[0].norm[2]);

                      for (i = 2; i >= 1; i--)
                        fprintf(
                            fp, "%.5g %.5g %.5g %.5g %.5g %.5g\n",
                            outpt.pts[c + i].pt[0], outpt.pts[c + i].pt[1],
                            outpt.pts[c + i].pt[2], outpt.pts[c + i].norm[0],
                            outpt.pts[c + i].norm[1], outpt.pts[c + i].norm[2]);
                    }
                  }
                } else {
                  /* store the polygon into the actual layer */
                  /* average normal in each vertex */

                  /* get the pointer to the voxel */
                  ptr = three_layers +
                        (act_layer * grid->size[Y] + y) * grid->size[X] + x;

                  /* get all triangle normals (up to 4) */
                  for (p = 0; p <= num - 3; p++)
                    ptr->orient[num_pols][p] =
                        GetTriangleNormal(outpt.pts + 0, outpt.pts + p + 1,
                                          outpt.pts + p + 2, tnormals[p]);

                  /* first point is in all triangles */
                  for (i = 0; i < 3; i++)
                    sum_norm[i] = tnormals[0][i];

                  for (p = 1; p <= num - 3; p++) {
                    for (i = 0; i < 3; i++)
                      sum_norm[i] += tnormals[p][i];
                  }
                  Normalize(sum_norm);

                  StorePoint(ptr, num_pols, 0, outpt.pts[0].pt, sum_norm);
                  ptr->pts[num_pols][0].edge = outpt.pts[0].edge;

                  /* second point is only in the first triangle */
                  StorePoint(ptr, num_pols, 1, outpt.pts[1].pt, tnormals[0]);
                  ptr->pts[num_pols][1].edge = outpt.pts[1].edge;

                  /* following points are in two triangles */
                  for (p = 2; p <= num - 2; p++) {
                    for (i = 0; i < 3; i++)
                      sum_norm[i] += tnormals[p - 2][i] + tnormals[p - 1][i];
                    Normalize(sum_norm);

                    StorePoint(ptr, num_pols, p, outpt.pts[p].pt, sum_norm);
                    ptr->pts[num_pols][p].edge = outpt.pts[p].edge;
                  }

                  /* last point is only in the last triangle */
                  StorePoint(ptr, num_pols, num - 1, outpt.pts[num - 1].pt,
                             tnormals[num - 3]);
                  ptr->pts[num_pols][num - 1].edge = outpt.pts[num - 1].edge;

                  /* set the number of points */
                  ptr->num_pts[num_pols] = num;
                }
              } else {
                /* without normals - directly into the file */
                fprintf(fp, "polygon\n");
                for (c = 0; c < num; c++) {
                  fprintf(fp, "%.5g %.5g %.5g\n", outpt.pts[c].pt[0],
                          outpt.pts[c].pt[1], outpt.pts[c].pt[2]);
                }
              }
              /* check if any points left */
              ind = -1;
              num = 0;
              for (i = 0; i < 12; i++)
                if ((P[i] != -1) && (!taken[i])) {
                  num++;
                  if (ind == -1)
                    /* again set the first intersection */
                    ind = i;
                }

              if (num >= 3) {
                /*
                  if(output_normals)
                  if(three_layers != NULL) {
                  fprintf(stderr,
                  "Soil - second polygon in a voxel. Ignored.\n");
                  break;
                  }
                  */
                if (++num_pols == MAX_POLS) {
                  fprintf(stderr, "Soil - more than 4 polygons in a voxel!"
                                  "Ignored.\n");
                  break;
                }

                if (verbose)
                  fprintf(stderr, "Soil - second polygon in a voxel.\n");
              } else {
                /* no more polygons for this cube */
                if (output_normals)
                  if (three_layers != NULL)
                    for (c = num_pols + 1; c < 3; c++)
                      three_layers[(act_layer * grid->size[Y] + y) *
                                       grid->size[X] +
                                   x]
                          .num_pts[c] = 0;
                break;
              }
            } else {
              /* no polygon for this cube */
              if (output_normals)
                if (three_layers != NULL)
                  for (c = 0; c < 3; c++)
                    three_layers[(act_layer * grid->size[Y] + y) *
                                     grid->size[X] +
                                 x]
                        .num_pts[c] = 0;

              break;
            }
          }
        }
      }
    }

    if (output_normals)
      if (three_layers != NULL) {
        if (z == 1)
          /* output the first layer */
          OutputLayer(three_layers, grid, fp, -1, 0, 1);

        if (z >= 2)
          /* go through the middle: ('act_layer'+2)%3  layer */
          OutputLayer(three_layers, grid, fp, (act_layer + 1) % 3,
                      (act_layer + 2) % 3, act_layer);

        act_layer = (act_layer + 1) % 3;
      }
  }

  if (output_normals)
    if (three_layers != NULL) {
      /* finish the layers */
      if (grid->size[Z] == 1)
        /* output the only layer */
        OutputLayer(three_layers, grid, fp, -1, 0, -1);
      else
        /* output the last layer */
        OutputLayer(three_layers, grid, fp, -(act_layer + 2) % 3, act_layer,
                    -1);

      free(three_layers);
    }
}
