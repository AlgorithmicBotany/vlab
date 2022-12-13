/*
  Environmental process - density (3D)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "density3d.h"
#include "comm_lib.h"
#include "octree.h"

/**** field specific variables ****/

#define X 0
#define Y 1
#define Z 2

struct grid_type {
  int size[3];    /* size of the grid (in nodes) */
  float range[3]; /* size in coordinates */
  float pos[3];   /* position of lower left front corner */
  CELL_TYPE *data;
};
typedef struct grid_type grid_type;

grid_type grid;

char verbose, use_octree;

int current_step;

/***** prototypes *****/

/****************************************************************************/
void FreeFieldStructures(void) {
  if (!use_octree) {
    if (grid.data != NULL)
      free(grid.data);
    grid.data = NULL;
  } else
    FreeOctreeStructures();
}

/****************************************************************************/
void InitializeFieldStructures(void) {
  int i;

  if (!use_octree) {
    if ((grid.data = (CELL_TYPE *)malloc(grid.size[X] * grid.size[Y] *
                                         grid.size[Z] * sizeof(CELL_TYPE))) ==
        NULL) {
      fprintf(stderr, "density3d - cannot allocate memory for the grid!\n");
      exit(0);
    }

    for (i = 0; i < grid.size[X] * grid.size[Y] * grid.size[Z]; i++)
      grid.data[i] = 0;
  }
}

/****************************************************************************/
void DetermineResponse(grid_type *grid, Cmodule_type *comm_symbol,
                       CTURTLE *tu) {
  int x, y, z, c;
  CELL_TYPE *cell;
  double pt[3];

  if (tu->positionC < 3) {
    fprintf(stderr,
            "density3d - turtle position wasn't sent to the environment.\n");
    return;
  }

  if (comm_symbol->num_params != 1) {
    fprintf(stderr, "density3d - comm. symbol must have one parameter\n");
    return;
  }

  if (use_octree) {
    for (c = 0; c < 3; c++)
      pt[c] = (tu->position[c] - grid->pos[c]) / (double)grid->range[c];

    cell = GetLeafPt(pt);
  } else {
    x = floor((tu->position[0] - grid->pos[X]) / grid->range[X] *
              (double)(grid->size[X]));
    y = floor((tu->position[1] - grid->pos[Y]) / grid->range[Y] *
              (double)(grid->size[Y]));
    z = floor((tu->position[2] - grid->pos[Z]) / grid->range[Z] *
              (double)(grid->size[Z]));

    if ((x < 0) || (x >= grid->size[X]) || (y < 0) || (y >= grid->size[Y]) ||
        (z < 0) || (z >= grid->size[Z])) {
      if (verbose)
        fprintf(stderr, "density3d - node [%d,%d,%d] is out of the range.\n", x,
                y, z);

      cell = NULL;
    } else
      cell = grid->data + z * grid->size[X] * grid->size[Y] +
             y * grid->size[X] + x;
  }

  if (cell != NULL) {
    (*cell) += comm_symbol->params[0].value;

    comm_symbol->params[0].value = *cell;
  } else
    comm_symbol->params[0].value = 0;

  comm_symbol->params[0].set = 1;
}

/****************************************************************************/
void ProcessArguments(int argc, char **argv) {
  FILE *fp;
  int i;
  char *keywords[] = {
      "verbose",      /*  0 */
      "range",        /*  1 */
      "position",     /*  2 */
      "grid size",    /*  3 */
      "octree level", /*  4 */
      NULL            /* the last item must be NULL! */
  };
  char *token, input_line[255];

  if (argc == 1) {
    printf("density3d - not enough arguments!\n"
           "USAGE: density3d -e environment_file density3d_spec_file\n");
    exit(0);
  }

  /* defaults */
  verbose = 0;
  use_octree = 0;

  grid.size[X] = 1;
  grid.size[Y] = 1;
  grid.size[Z] = 1;
  grid.range[X] = 2.0;
  grid.range[Y] = 2.0;
  grid.range[Z] = 2.0;
  grid.pos[X] = -1.0;
  grid.pos[Y] = -1.0;
  grid.pos[Z] = -1.0;

  /* read in environment file */
  if ((fp = fopen(argv[1], "r")) == NULL)
    fprintf(stderr, "density3d - cannot open specification file %s.\n",
            argv[1]);
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
                "density - unknown directive %s in the specification file.\n",
                token);
        continue;
      }

      switch (i) {
      case 0: /* verbose */
        token = strtok(NULL, "x,; \t:\n");
        if (token == NULL)
          break;
        if (strcmp(token, "on") == 0)
          verbose = 1;
        break;

      case 1: /* domain size - range */
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        grid.range[X] = atof(token);
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        grid.range[Y] = atof(token);
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        grid.range[Z] = atof(token);
        break;

      case 2: /* position */
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        grid.pos[X] = atof(token);
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        grid.pos[Y] = atof(token);
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        grid.pos[Z] = atof(token);
        break;

      case 3: /* grid size */
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

      case 4: /* octree level - overrides command 'grid size' */
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        InitializeOctree(atoi(token));
        use_octree = 1;
        break;
      }
    }
  }
  if (verbose) {
    fprintf(stderr, "density3d - domain size:     %gx%gx%g\n", grid.range[X],
            grid.range[Y], grid.range[Z]);
    fprintf(stderr, "density3d - position:       (%g,%g,%g)\n", grid.pos[X],
            grid.pos[Y], grid.pos[Z]);

    if (use_octree)
      fprintf(stderr, "density3d - octree used.\n");
    else
      fprintf(stderr, "density3d - number of nodes: %dx%dx%d\n", grid.size[X],
              grid.size[Y], grid.size[Z]);
  }

  InitializeFieldStructures();
}

/****************************************************************************/
void MainLoop(void) {
  Cmodule_type two_modules[2];
  unsigned long module_id;
  int master;
  CTURTLE turtle;
  int i;

  /* infinite loop - until signal 'exit' comes */
  for (;;) {
    CSBeginTransmission();

    if (verbose)
      fprintf(stderr, "density3d - start processing data.\n");

    /* process the data */
    while (CSGetData(&master, &module_id, two_modules, &turtle)) {

      for (i = 0; i < two_modules[0].num_params; i++)
        two_modules[0].params[i].set = 0;

      DetermineResponse(&grid, &two_modules[0], &turtle);

      CSSendData(master, module_id, &two_modules[0]);
    }

    if (verbose)
      fprintf(stderr, "density3d - data processed.\n");

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
