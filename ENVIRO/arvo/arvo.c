/*
  Environmental process - Arvo and Kirk's environmentally sensitive automata
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include "image.h"
#include "arvo.h"
#include "comm_lib.h"
#include "scene3d.h"
#include "matrix.h"

/**** field specific variables ****/
#define QUERY_ARRAY_SIZE 1000

struct item_type {
  float position[3];
  float heading[3];
  float up[3];
  float length;
  int tries;
  int id; /* in case it represents an object added to the grid */
  unsigned char num_params;
  unsigned long dist;
  int master;
};
typedef struct item_type item_type;

item_type *queries;
int num_queries;
int query_array_size;

grid_type grid;

char verbose, add_objects, remove_objects;

float surf_dist, max_surf_dist;
int maxtriesQ, maxtries_surface;

unsigned long ray_signature;

/****************************************************************************/
void FreeFieldStructures(void) {
  if (grid.data != NULL)
    free(grid.data);
  grid.data = NULL;

  if (queries != NULL)
    free(queries);
  queries = NULL;
}

/****************************************************************************/
void InitializeFieldStructures(void) {
  FreeFieldStructures();
  num_queries = 0;
  query_array_size = QUERY_ARRAY_SIZE;
  if ((queries = (item_type *)malloc(query_array_size * sizeof(item_type))) ==
      NULL) {
    fprintf(stderr, "arvo - cannot allocate memory for querries.\n");
    exit(0);
  }
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
void StoreInput(grid_type *grid, int master, unsigned long dist,
                Cmodule_type *two_modules, CTURTLE *tu) {
  int i;

  if ((two_modules[0].num_params != 7) && (two_modules[0].num_params != 0) &&
      (two_modules[0].num_params != 1) && (two_modules[0].num_params != 4)) {
    fprintf(stderr,
            "arvo - communication symbol must have 0,1,4 or 7 parameters.\n");
    return;
  }

  if (tu->positionC < 3) {
    fprintf(stderr, "arvo - turtle position wasn't sent to the environment.\n");
    return;
  }

  if (tu->headingC < 3) {
    fprintf(stderr, "arvo - turtle heading wasn't sent to the environment.\n");
    return;
  }

  if (two_modules[0].num_params == 7) {
    if (tu->upC < 3) {
      fprintf(stderr,
              "arvo - turtle up vector wasn't sent to the environment.\n");
      return;
    }
  }

  if (two_modules[0].num_params > 0) {
    /* store the query */
    if (num_queries >= query_array_size) {
      /* reallocate */
      query_array_size *= 2;

      if ((queries = (item_type *)realloc(
               queries, query_array_size * sizeof(item_type))) == NULL) {
        fprintf(stderr, "arvo - cannot reallocate memory for querries.\n");
        exit(0);
      }
      if (verbose)
        fprintf(stderr, "arvo - queries reallocated. New size is %d.\n",
                query_array_size);
    }

    for (i = X; i <= Z; i++) {
      queries[num_queries].position[i] = tu->position[i];
      queries[num_queries].heading[i] = tu->heading[i];
    }
    Normalize(queries[num_queries].heading);

    queries[num_queries].id = -1;
    queries[num_queries].dist = dist;
    queries[num_queries].master = master;
    queries[num_queries].num_params = two_modules[0].num_params;

    if (two_modules[0].num_params == 7) {
      for (i = X; i <= Z; i++) {
        queries[num_queries].up[i] = tu->up[i];
      }
      Normalize(queries[num_queries].up);

      queries[num_queries].length = two_modules[0].params[0].value;
      queries[num_queries].tries = two_modules[0].params[1].value;
    } else {
      /* one or four parameters */
      queries[num_queries].length = two_modules[0].params[0].value;
    }

    num_queries++;
  }

  if (add_objects) {
    i = AddObject(grid, tu, &two_modules[1]);

    if (two_modules[0].num_params > 0)
      queries[num_queries - 1].id = i;
  }
}

/****************************************************************************/
int cell_step[3];
float node_size[3];

void SetGlobalParameters(void) {
  int c;

  cell_step[X] = 1;
  cell_step[Y] = grid.size[X];
  cell_step[Z] = grid.size[X] * grid.size[Y];

  for (c = X; c <= Z; c++) {
    /* half size of a node */
    node_size[c] = 0.5 * grid.range[c] / (float)grid.size[c];
  }
}

/****************************************************************************/
/* assuming that dir is of unit length - necessary for IsIntersection() !!! */
float Intersect(float *pt, float *dir, int id, float mindist,
                float surface_dist, grid_type *grid, float *norm) {
  float middle[3], nsize[3], rnorm[3] = {0, 0, 0};
  float aux, smallest, len;
  OBJECT_LIST_TYPE *ptr;
  int c, node[3], ind, cstep[3];
  CELL_TYPE *cell;
  char intersects = 0;

  for (c = X; c <= Z; c++) {
    /* adjusted according to the direction */
    if (dir[c] < 0) {
      nsize[c] = -node_size[c];
      cstep[c] = -cell_step[c];
    } else {
      nsize[c] = node_size[c];
      cstep[c] = cell_step[c];
    }
  }

  for (c = X; c <= Z; c++) {
    /* initial voxel */
    node[c] =
        floor((pt[c] - grid->pos[c]) / grid->range[c] * (float)grid->size[c]);

    if ((node[c] < 0) || (node[c] >= grid->size[c]))
      return -1;

    /* voxel center */
    middle[c] = ((float)node[c] + 0.5) / (float)grid->size[c] * grid->range[c];
  }

  cell = grid->data + node[Z] * grid->size[Y] * grid->size[X] +
         node[Y] * grid->size[X] + node[X];

  /* increase the current ray signature */
  if (++ray_signature == 0) {
    ray_signature = 1;

    /* all signatures of obstacles and objects should be set to 0 */
  }

  if (mindist < 0)
    mindist = 2 * (grid->range[X] + grid->range[Y] + grid->range[Z]);

  for (;;) {
    /* go through the list associated with the node and check for
       intersection with each object */
    ptr = cell->list;

    while (ptr != NULL) {
      if ((ptr->prim->ray_signature != ray_signature) &&
          (ptr->prim->id != id)) {
        /* make sure that the direction is of unit lenght! */
        if ((len = IsIntersection(pt, dir, mindist, surface_dist, ptr->prim,
                                  rnorm)) >= 0) {
          ptr->prim->ray_signature = ray_signature;

          if (len < mindist) {
            intersects = 1;

            mindist = len;

            for (c = X; c <= Z; c++)
              norm[c] = rnorm[c];
          }
        }
      }
      ptr = ptr->next;
    }

    /* determine the next node intersected by the ray */

    /* find the intersected face */
    smallest = fabs(middle[X] + nsize[X] - pt[X]);
    ind = X;

    for (c = Y; c <= Z; c++)
      if (fabs((aux = (middle[c] + nsize[c] - pt[c])) * dir[ind]) <
          fabs(smallest * dir[c])) {
        smallest = aux;
        ind = c;
      }

    /* compare the distance for the intersection with the voxel and */
    /* mindist */
    if (smallest / dir[ind] > mindist) {
      /* doesn't have to go further */
      return intersects ? mindist : -1;
    }

    /* get the next middle point */
    middle[ind] += 2 * nsize[ind];

    cell += cstep[ind];

    if (dir[ind] > 0)
      node[ind]++;
    else
      node[ind]--;

    /* if the ray leaves the grid, go out of the loop */
    if ((node[ind] < 0) || (node[ind] >= grid->size[ind])) {
      return intersects ? mindist : -1;
    }
  }
}

/****************************************************************************/
int IsInside(float *pt, int id, float surf_dist, grid_type *grid, float *norm) {
  OBJECT_LIST_TYPE *ptr;
  int c, node[3];
  CELL_TYPE *cell;

  for (c = X; c <= Z; c++) {
    /* initial voxel */
    node[c] =
        floor((pt[c] - grid->pos[c]) / grid->range[c] * (float)grid->size[c]);

    if ((node[c] < 0) || (node[c] >= grid->size[c]))
      return 0;
  }

  cell = grid->data + node[Z] * grid->size[Y] * grid->size[X] +
         node[Y] * grid->size[X] + node[X];

  ptr = cell->list;

  while (ptr != NULL) {
    if (ptr->prim->id != id)
      if (IsInObject(pt, surf_dist, ptr->prim, norm))
        return 1;

    ptr = ptr->next;
  }

  return 0;
}

/****************************************************************************/
/* vector perpendicular to vec having angle alpha with desdir */
void PickOrthogonal(float *vec, float *desdir, float alpha, float *res) {
  int i;
  float y[3], sina, cosa;

  /* get y perpendicular to both */
  CrossProduct(desdir, vec, y);

  sina = sin(alpha);
  cosa = cos(alpha);

  for (i = X; i <= Z; i++)
    res[i] = desdir[i] * cosa + y[i] * sina;
}

/****************************************************************************/
/* should be improved because this routine will generate more vectors
   around poles. */
void PickRandomVector(float *vec) {
  float alpha, beta, sinb;

  alpha = drand48() * 2 * M_PI;
  beta = drand48() * M_PI;

  vec[0] = cos(alpha) * (sinb = sin(beta));
  vec[2] = sin(alpha) * sinb;

  vec[1] = cos(beta);
}

/****************************************************************************/
void DetermineResponse(grid_type *grid) {
  int i, q, c;
  float mind, d, angle;
  float Q[3], T[3], norm[3], vec[3];
  int tries;
  Cmodule_type comm_symbol;
  unsigned long dist;
  int master;

  if (verbose) {
    fprintf(stderr, "arvo - starting answering %d querie(s).\n", num_queries);
  }

  /* for all queries */
  for (q = 0; q < num_queries; q++) {
    dist = queries[q].dist;
    master = queries[q].master;

    comm_symbol.num_params = queries[q].num_params;

    for (i = 0; i < comm_symbol.num_params; i++)
      comm_symbol.params[i].set = 0;

    if (queries[q].num_params != 7) {

      /* end point */
      for (c = X; c <= Z; c++)
        T[c] =
            queries[q].position[c] + queries[q].heading[c] * queries[q].length;

      if (queries[q].length == 0)
        c = IsInside(queries[q].position, queries[q].id, surf_dist, grid, norm);
      else if (queries[q].length < 0)
        c = (Intersect(T, queries[q].heading, queries[q].id, -queries[q].length,
                       surf_dist, grid, norm) >= 0) ||
            IsInside(queries[q].position, queries[q].id, surf_dist, grid,
                     norm) ||
            IsInside(T, queries[q].id, surf_dist, grid, norm);
      else
        c = (Intersect(queries[q].position, queries[q].heading, queries[q].id,
                       queries[q].length, surf_dist, grid, norm) >= 0) ||
            IsInside(T, queries[q].id, surf_dist, grid, norm) ||
            IsInside(queries[q].position, queries[q].id, surf_dist, grid, norm);

      if (c) {
        comm_symbol.params[0].value = 1;
        comm_symbol.params[0].set = 1;

        if (queries[q].num_params == 4) {
          for (c = X; c <= Z; c++) {
            comm_symbol.params[1 + c].value = norm[c];
            comm_symbol.params[1 + c].set = 1;
          }
        }
      } else {
        comm_symbol.params[0].set = 1;
        comm_symbol.params[0].value = 0;

        if (queries[q].num_params == 4) {
          for (c = X; c <= Z; c++) {
            comm_symbol.params[1 + c].value = 0;
            comm_symbol.params[1 + c].set = 1;
          }
        }
      }
      if (verbose)
        fprintf(stderr, "Arvo - returned value: %g\n",
                comm_symbol.params[0].value);
    } else {
      /* default */
      comm_symbol.params[3].value = 0;
      comm_symbol.params[3].set = 1;

      /* follow the algorithm on page 4 of the paper */
      angle = 0;
      tries = 0; /* tries for Q */

      do {
        /* find Q */
        do {
          if (++tries == maxtriesQ) {
            fprintf(stderr, "Arvo - point Q not found.\n");
            goto send;
          }

          /* pick T orthogonal to querries[q].up - the previous normal */
          PickOrthogonal(queries[q].up, queries[q].heading, angle, T);

          angle = -angle;
          if ((tries % 2) == 1)
            angle += 2 * M_PI / (float)maxtriesQ;

          for (c = X; c <= Z; c++)
            Q[c] = queries[q].position[c] + queries[q].length * T[c];

        } while (Intersect(queries[q].position, T, queries[q].id,
                           queries[q].length, 0, grid, norm) >= 0);

        if (verbose)
          fprintf(stderr, "Arvo - point Q: (%g,%g,%g)\n", Q[0], Q[1], Q[2]);

        mind = max_surf_dist;

        for (i = 0; i < maxtries_surface; i++) {
          if (i == 0)
            for (c = X; c <= Z; c++)
              vec[c] = -queries[q].up[c];
          else
            PickRandomVector(vec);

          if ((d = Intersect(Q, vec, queries[q].id, mind, 0, grid, norm)) >=
              0) {
            mind = d;

            for (c = 0; c < 3; c++) {
              /* normal */
              comm_symbol.params[4 + c].value = norm[c];
              comm_symbol.params[4 + c].set = 1;

              /* new heading */
              comm_symbol.params[c].value =
                  (Q[c] + d * vec[c] + surf_dist * norm[c] -
                   queries[q].position[c]);
              comm_symbol.params[c].set = 1;
            }
            /* get the length of the heading */
            comm_symbol.params[3].value =
                sqrt(comm_symbol.params[0].value * comm_symbol.params[0].value +
                     comm_symbol.params[1].value * comm_symbol.params[1].value +
                     comm_symbol.params[2].value * comm_symbol.params[2].value);

            /* normalize the heading */
            if (comm_symbol.params[3].value > 0)
              for (c = X; c <= Z; c++)
                comm_symbol.params[c].value /= comm_symbol.params[3].value;
          }
        }

        for (c = X; c <= Z; c++)
          vec[c] = comm_symbol.params[c].value;

      } while ((mind == max_surf_dist) ||
               (Intersect(queries[q].position, vec, queries[q].id,
                          comm_symbol.params[3].value, 0, grid, norm) >= 0));

      if (verbose) {
        fprintf(stderr, "arvo - new heading: (%f,%f,%f), length: %g\n",
                comm_symbol.params[0].value, comm_symbol.params[1].value,
                comm_symbol.params[2].value, comm_symbol.params[3].value);
        fprintf(stderr, "       normal: (%f,%f,%f).\n",
                comm_symbol.params[4].value, comm_symbol.params[5].value,
                comm_symbol.params[6].value);
      }
    }

  send:
    CSSendData(master, dist, &comm_symbol);
  }
}

/****************************************************************************/
void ProcessArguments(int argc, char **argv) {
  FILE *fp;
  int i;
  char *keywords[] = {
      "domain size",          /*  0 */
      "position",             /*  1 */
      "verbose",              /*  2 */
      "grid size",            /*  3 */
      "seed",                 /*  4 */
      "surface distance",     /*  5 */
      "max surface distance", /*  6 */
      "obstacles",            /*  7 */
      "tries for Q",          /*  8 */
      "tries for surface",    /*  9 */
      "add objects",          /* 10 */
      "remove objects",       /* 11 */
      NULL                    /* the last item must be NULL! */
  };
  char *token, input_line[255];

  /* defaults */
  verbose = 0;
  surf_dist = 0.5;
  max_surf_dist = 2 * surf_dist;
  maxtriesQ = 64;
  maxtries_surface = 64;
  add_objects = 0;
  remove_objects = 1;

  if ((grid.data = (CELL_TYPE *)malloc(sizeof(CELL_TYPE))) == NULL)
    exit(0);
  grid.size[X] = 1;
  grid.size[Y] = 1;
  grid.size[Z] = 1;
  grid.data->list = NULL;

  grid.range[X] = 2.0;
  grid.range[Y] = 2.0;
  grid.range[Z] = 2.0;
  grid.pos[X] = -1.0;
  grid.pos[Y] = -1.0;
  grid.pos[Z] = -1.0;

  if (argc == 1) {
    printf("arvo - not enough arguments!\n"
           "USAGE: arvo -e environment_file specification_file\n");

    exit(0);
  }

  InitializeFieldStructures();

  /* read in environment file */
  if ((fp = fopen(argv[1], "r")) == NULL)
    fprintf(stderr, "arvo - cannot open specification file %s.\n", argv[1]);
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
                "arvo - unknown directive %s in the specification file.\n",
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

      case 2: /* verbose */
        token = strtok(NULL, ",; \t:\n");
        if (token == NULL)
          break;
        if (strcmp(token, "on") == 0)
          verbose = 1;
        break;

      case 3: /* number of nodes - size */
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
          fprintf(stderr, "arvo - cannot allocate memory for the grid!\n");
          break;
        }
        break;

      case 4: /* seed */
        token = strtok(NULL, ",; \t:\n");
        if (token == NULL)
          break;
        srand48(atol(token));
        break;

      case 5: /* distance from a surface */
        token = strtok(NULL, ",; \t:\n");
        if (token == NULL)
          break;
        surf_dist = atof(token);
        break;

      case 6: /* maximal distance from a surface */
        token = strtok(NULL, ",; \t:\n");
        if (token == NULL)
          break;
        max_surf_dist = atof(token);
        break;

      case 7: /* obstacles */
        /* file input */
        if ((token = strtok(NULL, ",; \t:\n")) == NULL)
          break;
        ReadPrimitiveFile(token, 1);
        break;

      case 8: /* number of tries for point Q */
        token = strtok(NULL, ",; \t:\n");
        if (token == NULL)
          break;
        maxtriesQ = atoi(token);
        break;

      case 9: /* number of tries for closest surface */
        token = strtok(NULL, ",; \t:\n");
        if (token == NULL)
          break;
        maxtries_surface = atoi(token);
        break;

      case 10: /* add objects */
        token = strtok(NULL, ",; \t:\n");
        if (token == NULL)
          break;
        if (strcmp(token, "on") == 0)
          add_objects = 1;
        break;

      case 11: /* remove objects */
        token = strtok(NULL, ",; \t:\n");
        if (token == NULL)
          break;
        if (strcmp(token, "off") == 0)
          remove_objects = 0;
        break;
      }
    }
  }

  SetGlobalParameters();

  FillGrid(&grid);

  if (remove_objects == 0)
    InitializeObjects();

  if (verbose) {
    fprintf(stderr, "arvo - domain size:     %gx%gx%g\n", grid.range[X],
            grid.range[Y], grid.range[Z]);
    fprintf(stderr, "arvo - position:       (%g,%g,%g)\n", grid.pos[X],
            grid.pos[Y], grid.pos[Z]);
    fprintf(stderr, "arvo - number of nodes: %dx%dx%d\n", grid.size[X],
            grid.size[Y], grid.size[Z]);

    fprintf(stderr, "\narvo - specification file processed.\n\n");
  }
}

/****************************************************************************/
void MainLoop(void) {
  Cmodule_type two_modules[2];
  unsigned long module_id;
  int master;
  CTURTLE turtle;
  int i;
  int in;

  /* infinite loop - until signal 'exit' comes */
  for (;;) {
    in = 0;

    if (verbose)
      fprintf(stderr, "arvo - start processing data.\n");

    num_queries = 0;

    if (remove_objects)
      InitializeObjects();

    CSBeginTransmission();

    /* process the data */
    while (CSGetData(&master, &module_id, two_modules, &turtle)) {
      in = 1;
      if (verbose) {
        fprintf(stderr, "arvo - comm. symbol has %d parameters:\n      ",
                two_modules[0].num_params);
        for (i = 0; i < two_modules[0].num_params; i++)
          fprintf(stderr, " %g", two_modules[0].params[i].value);
        fprintf(stderr, "\n");

        fprintf(stderr, "arvo - next symbol '%s' has %d parameters:\n      ",
                two_modules[1].symbol, two_modules[1].num_params);
        for (i = 0; i < two_modules[1].num_params; i++)
          fprintf(stderr, " %g", two_modules[1].params[i].value);
        fprintf(stderr, "\n");
      }

      StoreInput(&grid, master, module_id, two_modules, &turtle);
    }

    if (in)
      DetermineResponse(&grid);

    if (verbose)
      fprintf(stderr, "arvo - data processed.\n");

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
