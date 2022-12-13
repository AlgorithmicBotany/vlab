/*
  Environmental process - Takenaka's light environment
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "takenaka.h"
#include "comm_lib.h"

/**** field specific variables ****/
/* leaf clusters */
#define LEAF_ARRAY_SIZE 1000

struct LEAF_UNIT_TYPE {
  float pos[3];
  float rad;
  float la;           /* leaf area */
  unsigned long dist; /* position in L-system string */
  int master;
  float wm; /* weight of product necessary for maintenance */
  unsigned long ray_signature;
};
typedef struct LEAF_UNIT_TYPE LEAF_UNIT_TYPE;

LEAF_UNIT_TYPE *leaves = NULL;
int num_leaves;
int leaf_array_size;

/* grid */
struct LEAF_LIST_TYPE {
  unsigned int leaf_index;
  struct LEAF_LIST_TYPE *next;
};
typedef struct LEAF_LIST_TYPE LEAF_LIST_TYPE;

struct CELL_TYPE {
  LEAF_LIST_TYPE *list;
};
typedef struct CELL_TYPE CELL_TYPE;

#define X 0
#define Y 1
#define Z 2

struct grid_type {
  int size[3];    /* size of the grid (in nodes) */
  float range[3]; /* size in coordinates */
  float pos[3];   /* position of lower left front corner */
  float maxrad;
  CELL_TYPE *data;
};
typedef struct grid_type grid_type;

/* light sources */
struct SOURCE_TYPE {
  float pos[3];
  float radius;
  float intensity; /* mol/cm^2/yr */
};
typedef struct SOURCE_TYPE SOURCE_TYPE;

#define MAX_NUM_SOURCES 100

SOURCE_TYPE sources[MAX_NUM_SOURCES];

int num_sources;

/* prototypes */
CELL_TYPE *GetCell(grid_type *grid, int x, int y, int z);
void FreeFieldStructures(void);

grid_type grid = {0};

char verbose;
char is_disk_source;

float parameter_s, transmittance, efficiency, beam_radius;

unsigned long ray_signature;

/****************************************************************************/
void Normalize(float *norm) {
  float len;
  int i;

  /* normalize */
  len = sqrt(norm[0] * norm[0] + norm[1] * norm[1] + norm[2] * norm[2]);

  if (len > 0)
    for (i = 0; i < 3; i++)
      norm[i] /= len;
}

/****************************************************************************/
void FreeFieldStructures(void) {
  int i;
  LEAF_LIST_TYPE *ptr, *ptr2;

  if (grid.data != NULL) {
    for (i = 0; i < grid.size[X] * grid.size[Y] * grid.size[Z]; i++) {
      /* free the linked list */
      ptr = grid.data[i].list;

      while (ptr != NULL) {
        ptr2 = ptr->next;
        free(ptr);
        ptr = ptr2;
      }
    }

    free(grid.data);
    grid.data = NULL;
  }

  if (leaves != NULL) {
    free(leaves);
    leaves = NULL;
  }
  num_leaves = 0;
  leaf_array_size = LEAF_ARRAY_SIZE;
}

/****************************************************************************/
void InitializeLeafArray(void) {
  if ((leaves = (LEAF_UNIT_TYPE *)malloc(leaf_array_size *
                                         sizeof(LEAF_UNIT_TYPE))) == NULL) {
    fprintf(stderr, "Takenaka - cannot allocate memory for leaf array!\n");
    exit(0);
  }
}

/****************************************************************************/
float Distance2(float *p1, float *p2) {
  int i;
  float vec[3];

  for (i = 0; i < 3; i++)
    vec[i] = p2[i] - p1[i];

  return vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2];
}

/****************************************************************************/
/* adds a new item to the beginning of a linked list */
void AddToList(LEAF_LIST_TYPE **first, unsigned int item_index) {
  LEAF_LIST_TYPE *ptr;

  if ((ptr = (LEAF_LIST_TYPE *)malloc(sizeof(LEAF_LIST_TYPE))) == NULL) {
    fprintf(stderr, "Takenaka - cannot allocate memory for leaf item!\n");
    exit(0);
  }

  ptr->next = (*first);
  ptr->leaf_index = item_index;
  *first = ptr;
}

/****************************************************************************/
void InitializeGrid(grid_type *grid) {
  CELL_TYPE *ptr;
  int x, y, z, c;
  unsigned int lv;
  float range[3][2];
  float pt[3], half, aux;

  if ((grid->data = (CELL_TYPE *)malloc(grid->size[X] * grid->size[Y] *
                                        grid->size[Z] * sizeof(CELL_TYPE))) ==
      NULL) {
    fprintf(stderr, "Takenaka - cannot allocate memory for grid!\n");
    exit(0);
  }

  half = 0;
  for (c = X; c <= Z; c++) {
    aux = grid->range[c] / (float)grid->size[c];
    half += aux * aux;
  }
  half = 0.5 * sqrt(half);

  if (verbose)
    fprintf(stderr, "Initializing grid at (%g,%g,%g) of range %g x %g x %g.\n",
            grid->pos[X], grid->pos[Y], grid->pos[Z], grid->range[X],
            grid->range[Y], grid->range[Z]);

  /* set all lists to NULL */
  ptr = grid->data;
  for (z = 0; z < grid->size[Z]; z++)
    for (y = 0; y < grid->size[Y]; y++)
      for (x = 0; x < grid->size[X]; x++)
        (ptr++)->list = NULL;

  /* for all leaves */
  for (lv = 0; lv < (size_t)num_leaves; lv++) {

    /* get the range of voxels possibly intersecting the cluster sphere */
    for (c = X; c <= Z; c++) {
      range[c][0] = floor((leaves[lv].pos[c] - leaves[lv].rad - grid->maxrad -
                           grid->pos[c]) /
                          grid->range[c] * (float)grid->size[c]) -
                    1;
      if (range[c][0] < 0)
        range[c][0] = 0;

      range[c][1] = 1 + floor((leaves[lv].pos[c] + leaves[lv].rad +
                               grid->maxrad - grid->pos[c]) /
                              grid->range[c] * (float)grid->size[c]);
      if (range[c][1] >= grid->size[c])
        range[c][1] = grid->size[c] - 1;
    }

    /* for all nodes in the range */
    for (z = range[Z][0]; z <= range[Z][1]; z++) {
      pt[Z] = grid->pos[Z] +
              ((float)z + 0.5) / (float)grid->size[Z] * grid->range[Z];

      for (y = range[Y][0]; y <= range[Y][1]; y++) {
        pt[Y] = grid->pos[Y] +
                ((float)y + 0.5) / (float)grid->size[Y] * grid->range[Y];

        for (x = range[X][0]; x <= range[X][1]; x++) {
          /* pt is the voxel's centre */
          pt[X] = grid->pos[X] +
                  ((float)x + 0.5) / (float)grid->size[X] * grid->range[X];

          if (Distance2(pt, leaves[lv].pos) <=
              (leaves[lv].rad + grid->maxrad + half) *
                  (leaves[lv].rad + grid->maxrad + half))
            AddToList(&(GetCell(grid, x, y, z)->list), lv);
        }
      }
    }
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
void StoreLeafCluster(grid_type *grid, int master, unsigned long module_id,
                      Cmodule_type *comm_symbol, CTURTLE *tu) {
  int i;
  float rad, aux;

  if (comm_symbol->num_params < 2) {
    if (verbose)
      fprintf(stderr, "Takenaka - two parameters for ?E required!\n");
    return; /* nothing required + no room for the response */
  }

  if (tu->positionC < 3) {
    fprintf(stderr,
            "Takenaka - turtle position wasn't sent to the environment.\n");
    return;
  }

  if (num_leaves >= leaf_array_size) {
    /* rellocate the array */
    leaf_array_size *= 2;
    if ((leaves = (LEAF_UNIT_TYPE *)realloc(
             leaves, leaf_array_size * sizeof(LEAF_UNIT_TYPE))) == NULL) {
      fprintf(stderr, "Takenaka - cannot reallocate memory for leaf array!\n");
      exit(0);
    }
    if (verbose)
      fprintf(stderr, "Takenaka - leaf array reallocated to size %d.\n",
              leaf_array_size);
  }

  leaves[num_leaves].la = comm_symbol->params[0].value;

  leaves[num_leaves].rad = rad =
      parameter_s * 0.5 * sqrt(comm_symbol->params[0].value / M_PI);

  if (rad > grid->maxrad)
    grid->maxrad = rad;

  for (i = X; i <= Z; i++) {
    leaves[num_leaves].pos[i] = tu->position[i];
  }

  /* update the grid so it encloses all leaf clusters */
  if (num_leaves == 0)
    /* first leaf cluster */
    for (i = X; i <= Z; i++) {
      grid->pos[i] = tu->position[i] - rad;
      grid->range[i] = 2 * rad;
    }
  else
    for (i = X; i <= Z; i++) {
      if ((aux = grid->pos[i] - (tu->position[i] - rad)) > 0) {
        grid->pos[i] -= aux;
        grid->range[i] += aux;
      }

      if ((aux = (tu->position[i] + rad) - (grid->pos[i] + grid->range[i])) >
          0) {
        grid->range[i] += aux;
      }
    }

  leaves[num_leaves].wm = comm_symbol->params[1].value;
  leaves[num_leaves].dist = module_id;
  leaves[num_leaves].master = master;

  num_leaves++;
}

/****************************************************************************/
int IsClusterIntersection(float *pt, float *dir, float rad,
                          LEAF_UNIT_TYPE *leaf) {
  float vec[3], aux, d;
  int c;

  for (c = X; c <= Z; c++)
    vec[c] = pt[c] - leaf->pos[c];

  aux = dir[0] * vec[0] + dir[1] * vec[1] + dir[2] * vec[2]; /* dir.(pt-C) */

  /* discriminant/4 = aux^2 - (pt-C).(pt-C) + rad^2 */
  if ((d = aux * aux - (vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]) +
           (leaf->rad + rad * beam_radius) * (leaf->rad + rad * beam_radius)) >=
      0)
    /* there is an intersection */
    if (-aux + sqrt(d) > 0)
      /* and is behind pt */
      return 1;

  /* otherwise return 0 */
  return 0;
}

/****************************************************************************/
float ProductFromSource(LEAF_UNIT_TYPE *leaf, SOURCE_TYPE *source,
                        grid_type *grid) {
  float dir[3], pt[3], node_size[3], middle[3];
  float reduction, red, aux, smallest;
  LEAF_LIST_TYPE *ptr;
  int i, c, node[3], ind, cell_step[3];
  CELL_TYPE *cell;

  cell_step[X] = 1;
  cell_step[Y] = grid->size[X];
  cell_step[Z] = grid->size[X] * grid->size[Y];

  if (is_disk_source) {

    if (leaf->pos[Y] > sources->pos[Y]) {
      if (verbose)
        fprintf(stderr, "Takenaka - leaf above disk source!\n");
      return 0;
    }

    if (leaf->pos[X] * leaf->pos[X] + leaf->pos[Z] * leaf->pos[Z] >
        sources->radius * sources->radius) {
      if (verbose)
        fprintf(stderr, "Takenaka - leaf center outside the disk!\n");
      return 0;
    }

    /* direction towards the source */
    dir[X] = dir[Z] = 0;
    dir[Y] = source->pos[Y] - leaf->pos[Y];
  } else
    for (c = X; c <= Z; c++) {
      /* direction towards the source */
      dir[c] = source->pos[c] - leaf->pos[c];
    }

  for (c = X; c <= Z; c++) {
    /* current point in a voxel */
    pt[c] = leaf->pos[c];

    /* half size of a node */
    node_size[c] = 0.5 * grid->range[c] / (float)grid->size[c];

    /* adjusted according to the source position */
    if (dir[c] < 0) {
      node_size[c] = -node_size[c];
      cell_step[c] = -cell_step[c];
    }
  }

  Normalize(dir); /* necessary for IsClusterIntersection() */

  for (c = X; c <= Z; c++) {
    /* initial voxel */
    node[c] =
        floor((pt[c] - grid->pos[c]) / grid->range[c] * (float)grid->size[c]);

    /* voxel center */
    middle[c] = ((float)node[c] + 0.5) / (float)grid->size[c] * grid->range[c];
  }

  cell = GetCell(grid, node[X], node[Y], node[Z]);

  reduction = 1;
  red = 1 - (1 - transmittance) / (parameter_s * parameter_s);
  red *= red;

  /* increase the current ray signature */
  if (++ray_signature == 0) {
    ray_signature = 1;

    for (i = 0; i < num_leaves; i++)
      leaves[i].ray_signature = 0;
  }

  /* to prevent intersection with itself */
  leaf->ray_signature = ray_signature;

  for (;;) {
    /* go through the list associated with the node and check for
       intersection with each sphere */
    ptr = cell->list;

    while (ptr != NULL) {
      if (leaves[ptr->leaf_index].ray_signature != ray_signature) {
        if (IsClusterIntersection(pt, dir, leaf->rad,
                                  leaves + ptr->leaf_index)) {
          leaves[ptr->leaf_index].ray_signature = ray_signature;
          /* if intersects, multiply by the reduction factor */
          reduction *= red;
        }
      }
      ptr = ptr->next;
    }

    /* determine the next node intersected by the ray */

    /* find the intersected face */
    smallest = fabs(middle[X] + node_size[X] - pt[X]);
    ind = X;

    for (c = Y; c <= Z; c++)
      if (fabs((aux = (middle[c] + node_size[c] - pt[c])) * dir[ind]) <
          fabs(smallest * dir[c])) {
        smallest = aux;
        ind = c;
      }

    /* get the next middle point */
    middle[ind] += 2 * node_size[ind];

    cell += cell_step[ind];

    if (dir[ind] > 0)
      node[ind]++;
    else
      node[ind]--;

    /* if the ray leaves the grid, go out of the loop */
    if ((node[ind] < 0) || (node[ind] >= grid->size[ind])) {
      return source->intensity * reduction;
    }
  }
}

/****************************************************************************/
void DetermineResponse(grid_type *grid) {
  int lv, src, i;
  float sumTi, a;
  float sumarea = 0;
  long numvis = 0;
  Cmodule_type comm_symbol;

  comm_symbol.num_params = 2;
  for (i = 0; i < comm_symbol.num_params; i++)
    comm_symbol.params[i].set = 1;

  if (verbose) {
    fprintf(stderr, "Takenaka - maximum leaf cluster radius %g.\n",
            grid->maxrad);

    fprintf(stderr,
            "Takenaka - start determining response for each out of %d"
            " leaves.\n",
            num_leaves);
  }

  ray_signature = 0;
  for (lv = 0; lv < num_leaves; lv++)
    leaves[lv].ray_signature = 0;

  /* for all leaves */
  for (lv = 0; lv < num_leaves; lv++) {
    sumarea += leaves[lv].la;

    sumTi = 0;

    /* for all sources */
    for (src = 0; src < num_sources; src++)
      sumTi += ProductFromSource(leaves + lv, sources + src, grid);

    if (sumTi >= 8.60)
      numvis++;

    comm_symbol.params[0].value = leaves[lv].la;

    /* compute value a */
    a = 1 - (1 - transmittance) / (parameter_s * parameter_s);
    a = 1 - a * a;

    /* sumTi contains sum of Ti. We need Sum Ti.Ai.a
       Ai is s^2*PI*rad^2 for all i */
    comm_symbol.params[1].value =
        (sumTi * leaves[lv].la / 4.0 * parameter_s * parameter_s * a);

    /* times photosynthetic efficiency minus maintenance cost */
    comm_symbol.params[1].value =
        (comm_symbol.params[1].value * efficiency - leaves[lv].wm);

    CSSendData(leaves[lv].master, leaves[lv].dist, &comm_symbol);

    if (verbose) {
      fprintf(stderr, "Takenaka - returned value: %g.\n",
              comm_symbol.params[1].value);
    }
  }

  if (verbose) {
    fprintf(stderr, "Takenaka - %ld leaves unobstructed.\n", numvis);
    fprintf(stderr, "Takenaka - total leaf area: %g cm^2.\n", sumarea);
  }
}

/****************************************************************************/
void SetDefaultSources(void) {
  num_sources = 1;

  sources[0].pos[0] = 10000;
  sources[0].pos[1] = 0;
  sources[0].pos[2] = 0;

  /* 1000micromol/m^2/s converted to mol/cm^2/yr considering
     duration of daylight 12h/day and growth period 200 days/year */
  sources[0].intensity = 10e-7 * 3600 * 12 * 200; /* 8.64 */
}

/****************************************************************************/
void NormalizeSources(void) {
  int i;
  float total, r;

  if (num_sources == 0)
    SetDefaultSources();

  total = 0;
  for (i = 0; i < num_sources; i++)
    total += sources[i].intensity;

  /* 1000micromol/m^2/s converted to mol/cm^2/yr considering
     duration of daylight 12h/day and growth period 200 days/year */
  r = 10e-7 * 3600 * 12 * 200 / total;

  for (i = 0; i < num_sources; i++)
    sources[i].intensity *= r;

  if (verbose) {
    fprintf(stderr, "Takenaka - %d light sources.\n", num_sources);

    for (i = 0; i < num_sources; i++)
      fprintf(stderr, "Takenaka - %d. at (%g,%g,%g) with intensity %g.\n",
              i + 1, sources[i].pos[0], sources[i].pos[1], sources[i].pos[2],
              sources[i].intensity);
  }
}

/****************************************************************************/
void ProcessArguments(int argc, char **argv) {
  FILE *fp;
  int i;
  char *keywords[] = {
      "grid size",     /*  0 */
      "verbose",       /*  1 */
      "parameter s",   /*  2 */
      "transmittance", /*  3 */
      "source",        /*  4 */
      "efficiency",    /*  5 */
      "beam radius",   /*  6 */
      "disk source",   /*  7 */
      NULL             /* the last item must be NULL! */
  };
  char *token, input_line[255];

  /* defaults */
  verbose = 0;

  grid.size[X] = 1;
  grid.size[Y] = 1;
  grid.size[Z] = 1;

  parameter_s = 1.5;
  transmittance = 0.1;
  efficiency = 0.015;
  beam_radius = 0;

  num_sources = 0;
  is_disk_source = 0;

  if (argc == 1) {
    printf("Takenaka - not enough arguments!\n"
           "USAGE: takenaka -e environment_file light_spec_file\n");

    exit(0);
  }

  /* read in environment file */
  if ((fp = fopen(argv[1], "r")) == NULL)
    fprintf(stderr, "Takenaka - cannot open specification file %s.\n", argv[1]);
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
                "Takenaka - unknown directive %s in the specification file.\n",
                token);
        continue;
      }

      switch (i) {
      case 0: /* grid size */
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        grid.size[X] = atof(token);
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        grid.size[Y] = atof(token);
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        grid.size[Z] = atof(token);
        break;

      case 1: /* verbose */
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        if (strcmp(token, "on") == 0)
          verbose = 1;
        break;

      case 2: /* parameter s */
        if ((token = strtok(NULL, ",; \t:\n")) == NULL)
          break;
        parameter_s = atof(token);
        break;

      case 3: /* transmittance */
        if ((token = strtok(NULL, ",; \t:\n")) == NULL)
          break;
        transmittance = atof(token);
        break;

      case 4: /* source */
        if (is_disk_source) {
          fprintf(stderr,
                  "Takenaka - disk source used, point source igored!\n");
          break;
        }
        if (num_sources >= MAX_NUM_SOURCES) {
          fprintf(stderr, "Takenaka - too many light sources. Ignored.\n");
          break;
        }

        /* default */
        for (i = 0; i < 3; i++)
          sources[num_sources].pos[i] = 0;
        sources[num_sources].intensity = 1;

        for (i = 0; i < 3; i++) {
          if ((token = strtok(NULL, ",; \t:\n")) == NULL)
            break;
          sources[num_sources].pos[i] = atof(token);
        }

        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        sources[num_sources].intensity = atof(token);

        num_sources++;
        break;

      case 5: /* efficiency */
        if ((token = strtok(NULL, ",; \t:\n")) == NULL)
          break;
        efficiency = atof(token);
        break;

      case 6: /* beam_radius */
        if ((token = strtok(NULL, ",; \t:\n")) == NULL)
          break;
        beam_radius = atof(token);
        break;

      case 7: /* disk source */
        if ((token = strtok(NULL, ",; \t:\n")) == NULL)
          break;
        sources[0].radius = atof(token);
        sources[0].intensity = 1;
        is_disk_source = 1;

        for (i = 0; i < 3; i++) {
          if ((token = strtok(NULL, ",; \t:\n")) == NULL)
            break;
          sources[0].pos[i] = atof(token);
        }
        if (i != 3)
          /* use default */
          for (i = 0; i < 3; i++)
            sources[0].pos[i] = 0;
        break;
      }
    }
  }

  if (is_disk_source)
    num_sources = 1;

  if (verbose) {
    fprintf(stderr, "Takenaka - grid size: %dx%dx%d\n", grid.size[X],
            grid.size[Y], grid.size[Z]);

    fprintf(stderr, "Takenaka - parameter s: %g\n", parameter_s);

    fprintf(stderr, "Takenaka - efficiency: %g\n", efficiency);

    fprintf(stderr, "Takenaka - transmittance: %g\n", transmittance);

    fprintf(stderr, "\nTakenaka - specification file processed.\n\n");
  }

  /* normalize light sources */
  NormalizeSources();
}

/****************************************************************************/
void MainLoop(void) {
  Cmodule_type two_modules[2];
  unsigned long module_id;
  int master;
  CTURTLE turtle;
  int in;

  /* infinite loop - until signal 'exit' comes */
  for (;;) {
    CSBeginTransmission();

    in = 0;

    if (verbose)
      fprintf(stderr, "Takenaka - start processing data.\n");

    FreeFieldStructures();
    InitializeLeafArray();
    grid.maxrad = 0;

    /* process the data */
    while (CSGetData(&master, &module_id, two_modules, &turtle)) {
      in = 1;
      StoreLeafCluster(&grid, master, module_id, &two_modules[0], &turtle);
    }

    if (in) {
      InitializeGrid(&grid);

      DetermineResponse(&grid);
    }

    if (verbose)
      fprintf(stderr, "Takenaka - data processed.\n");

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
