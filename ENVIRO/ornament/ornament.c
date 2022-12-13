/*
  Environmental process - ornament
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "image.h"
#include "targa.h"
#include "ornament.h"
#include "comm_lib.h"

/**** field specific variables ****/
/* cell type */
typedef struct {
  unsigned char objects, light;
} CELL_TYPE;

/* quaries */
#define QUERY_ARRAY_SIZE 1000
struct QUERY_UNIT_TYPE {
  float pos[2];
  unsigned long dist; /* position in L-system string */
  int master;
};
typedef struct QUERY_UNIT_TYPE QUERY_UNIT_TYPE;

QUERY_UNIT_TYPE *queries = NULL;
int num_queries;
int query_array_size;

#define X 0
#define Y 1

struct grid_type {
  int size[2];    /* size of the grid (in nodes) */
  float range[2]; /* size in coordinates */
  float pos[2];   /* position of lower left front corner */
  CELL_TYPE *data;
  char *input_image_name;
};
typedef struct grid_type grid_type;

grid_type grid = {0};

char verbose;

/****************************************************************************/
void FreeFieldStructures(void) {
  if (grid.data != NULL) {
    free(grid.data);
    grid.data = NULL;
  }

  if (grid.input_image_name != NULL) {
    free(grid.input_image_name);
    grid.input_image_name = NULL;
  }

  if (queries != NULL) {
    free(queries);
    queries = NULL;
  }
  num_queries = 0;
}

/****************************************************************************/
void InitializeQueryArray(void) {
  if (queries != NULL) {
    free(queries);
    queries = NULL;
  }

  query_array_size = QUERY_ARRAY_SIZE;

  if ((queries = (QUERY_UNIT_TYPE *)malloc(query_array_size *
                                           sizeof(QUERY_UNIT_TYPE))) == NULL) {
    fprintf(stderr, "ornament - cannot allocate memory for queries!\n");
    exit(0);
  }
  num_queries = 0;
}

/****************************************************************************/
int LoadInitialData(grid_type *grid) {
  int x, y;
  float xratio, yratio;
  IMAGE *image;
  unsigned short *buf;
  CELL_TYPE *ptr;
  targa_params_type TGAspec;
  int c;

  if ((c = strlen(grid->input_image_name)) > 3)
    if (strcmp(grid->input_image_name + c - 3, "tga") == 0) {
      int xsize, ysize;
      unsigned char *cbuf, *rowbuf, *ptrc;

      /* load in targa image */
      if (!loadTGAhead(grid->input_image_name, &TGAspec)) {
        fprintf(stderr, "ornament: cannot open input targa image %s.\n",
                grid->input_image_name);
        return 0;
      }
      xsize = TGAspec.Xres;
      ysize = TGAspec.Yres;

      if ((cbuf = (unsigned char *)malloc(xsize * ysize *
                                          sizeof(unsigned char))) == NULL) {
        fprintf(stderr, "ornament: cannot allocate memory for image %s!\n",
                grid->input_image_name);
        return 0;
      }

      if ((rowbuf = (unsigned char *)malloc(xsize * 3 *
                                            sizeof(unsigned char))) == NULL) {
        fprintf(stderr, "ornament: cannot allocate memory for image %s!\n",
                grid->input_image_name);
        return 0;
      }

      /* read the image in - starting with the last row */
      /* image is stored in rows of R, G, and B. We need R,G,B for each
         pixel */
      for (y = ysize - 1; y >= 0; y--) {
        loadTGArow(&TGAspec, y, rowbuf);

        ptrc = cbuf + y * xsize;
        for (x = 0; x < xsize; x++) {
          /* get only green */
          *(ptrc++) = rowbuf[x * 3 + 1];
        }
      }

      xratio = (float)(xsize - 1) / (float)(grid->size[X] - 1);
      yratio = (float)(ysize - 1) / (float)(grid->size[Y] - 1);

      /* set the 2d grid - assuming that it is bigger than the input image*/
      ptr = grid->data;

      for (y = 0; y < grid->size[Y]; y++)
        for (x = 0; x < grid->size[X]; x++) {
          ptr->light = cbuf[xsize * (int)(y * yratio) + (int)(x * xratio)];
          ptr->objects = 0;
          ptr++;
        }

      loadTGAfinish(&TGAspec);
      free(cbuf);
      free(rowbuf);
      
      return 1;
    }

  /* otherwice open rgb image */
  if ((image = iopen(grid->input_image_name, "r", 0, 0, 0, 0, 0)) == NULL) {
    fprintf(stderr, "ornament - cannot open image %s!\n",
            grid->input_image_name);
    return 0;
  }

  xratio = (float)(image->xsize - 1) / (float)(grid->size[X] - 1);
  yratio = (float)(image->ysize - 1) / (float)(grid->size[Y] - 1);

  /* one line buffer */
  if ((buf = (unsigned short *)malloc(image->xsize * image->ysize *
                                      sizeof(unsigned short))) == NULL) {
    fprintf(stderr,
            "ornament - cannot allocate memory for one channel of image %s!\n",
            grid->input_image_name);
    return 0;
  }

  /* Read the image in. Only green channel is stored .*/
  /* Later on, R and B channel can be used for nutrients or obstales. */

  for (y = 0; y < image->ysize; y++) {
    /* green */
    getrow(image, buf + image->ysize * y, y, 1);
  }

  /* set the 2d grid - assuming that it is bigger than the input image*/
  ptr = grid->data;

  for (y = 0; y < grid->size[Y]; y++)
    for (x = 0; x < grid->size[X]; x++) {
      ptr->light = buf[image->xsize * (int)(y * yratio) + (int)(x * xratio)];
      ptr->objects = 0;
      ptr++;
    }

  free(buf);
  iclose(image);

  return 1;
}

/****************************************************************************/
void InitializeGrid(grid_type *grid) {
  int x;

  if ((grid->data = (CELL_TYPE *)malloc(grid->size[X] * grid->size[Y] *
                                        sizeof(CELL_TYPE))) == NULL) {
    fprintf(stderr, "ornament - cannot allocate memory for grid!\n");
    exit(0);
  }

  if (grid->input_image_name == NULL) {
    fprintf(stderr, "ornament - no input image. Full light assumed.\n");

    for (x = 0; x < grid->size[Y] * grid->size[X]; x++) {
      grid->data[x].light = 255;
      grid->data[x].objects = 0;
    }
  } else
    LoadInitialData(grid);

  if (verbose)
    fprintf(stderr, "ornament - grid initialized.\n");
}

/****************************************************************************/
CELL_TYPE *GetCell(grid_type *grid, int x, int y) {
  if (x < 0)
    return NULL;
  if (x >= grid->size[X])
    return NULL;

  if (y < 0)
    return NULL;
  if (y >= grid->size[Y])
    return NULL;

  return grid->data + y * grid->size[X] + x;
}

/****************************************************************************/
void ProcessQuery(grid_type *grid, int master, unsigned long module_id,
                  Cmodule_type *comm_symbol, CTURTLE *tu) {
  int c;
  CELL_TYPE *cell;
  float radius;
  int node[2], range[2], x, y;

  if (comm_symbol->num_params != 2) {
    if (verbose)
      fprintf(stderr, "ornament - two parameters for ?E required!\n");
    return;
  }

  if (tu->positionC < 2) {
    fprintf(stderr,
            "ornament - turtle position wasn't sent to the environment.\n");
    return;
  }

  if (num_queries >= query_array_size) {
    /* rellocate the array */
    query_array_size *= 2;
    if ((queries = (QUERY_UNIT_TYPE *)realloc(
             queries, query_array_size * sizeof(QUERY_UNIT_TYPE))) == NULL) {
      fprintf(stderr, "ornament - cannot reallocate memory for query array!\n");
      exit(0);
    }
    if (verbose)
      fprintf(stderr, "ornament - query array reallocated to size %d.\n",
              query_array_size);
  }

  queries[num_queries].dist = module_id;
  queries[num_queries].master = master;

  for (c = X; c <= Y; c++)
    queries[num_queries].pos[c] = tu->position[c];

  num_queries++;

  radius = comm_symbol->params[1].value;

  for (c = X; c <= Y; c++) {
    node[c] = floor((tu->position[c] - grid->pos[c]) / grid->range[c] *
                    (float)grid->size[c]);

    range[c] = radius / grid->range[c] * (float)grid->size[c] + 1;
  }

  for (y = -range[Y]; y < range[Y]; y++)
    for (x = -range[X]; x < range[X]; x++)
      if ((float)(x * x) / (float)(range[X] * range[X]) +
              (float)(y * y) / (float)(range[Y] * range[Y]) <=
          1) {
        cell = GetCell(grid, node[X] + x, node[Y] + y);

        if (cell != NULL)
          if (cell->objects < 255)
            cell->objects += 1;
      }
}

/****************************************************************************/
void DetermineResponse(grid_type *grid) {
  int q, c, node[2];
  CELL_TYPE *cell;
  Cmodule_type comm_symbol;

  if (verbose) {
    fprintf(stderr,
            "ornament - start determining response for each out of %d"
            " queriess.\n",
            num_queries);
  }

  comm_symbol.num_params = 2;
  comm_symbol.params[0].set = 1;
  comm_symbol.params[1].set = 1;

  /* for all queries */
  for (q = 0; q < num_queries; q++) {
    for (c = X; c <= Y; c++)
      node[c] = floor((queries[q].pos[c] - grid->pos[c]) / grid->range[c] *
                      (float)grid->size[c]);

    if ((cell = GetCell(grid, node[X], node[Y])) == NULL) {
      comm_symbol.params[0].value = 0;
      comm_symbol.params[1].value = 0;
    } else {
      comm_symbol.params[0].value = cell->objects;
      comm_symbol.params[1].value = cell->light;
    }

    CSSendData(queries[q].master, queries[q].dist, &comm_symbol);

    if (verbose)
      fprintf(stderr, "ornament - objects: %f, field value: %f.\n",
              comm_symbol.params[0].value, comm_symbol.params[1].value);
  }
}

/****************************************************************************/
void ProcessArguments(int argc, char **argv) {
  FILE *fp;
  int i;
  char *keywords[] = {
      "grid size",     /*  0 */
      "grid position", /*  1 */
      "verbose",       /*  2 */
      "input image",   /*  3 */
      "grid range",    /*  4 */
      NULL             /* the last item must be NULL! */
  };
  char *token, input_line[255];

  FreeFieldStructures();

  /* defaults */
  verbose = 0;

  grid.size[X] = 512;
  grid.size[Y] = 512;
  grid.pos[X] = -1;
  grid.pos[Y] = -1;
  grid.range[X] = 2;
  grid.range[Y] = 2;

  if (argc == 1) {
    printf("ornament - not enough arguments!\n"
           "USAGE: ornament -e environment_file spec_file\n");

    exit(0);
  }

  /* read in environment file */
  if ((fp = fopen(argv[1], "r")) == NULL)
    fprintf(stderr, "ornament - cannot open specification file %s.\n", argv[1]);
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
                "ornament - unknown directive %s in the specification file.\n",
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
        break;

      case 1: /* grid position */
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        grid.pos[X] = atof(token);
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        grid.pos[Y] = atof(token);
        break;

      case 2: /* verbose */
        if ((token = strtok(NULL, ",; \t:\n")) == NULL)
          break;
        if (strcmp(token, "on") == 0)
          verbose = 1;
        break;

      case 3: /* input image */
        if ((token = strtok(NULL, ",; \t:\n")) == NULL)
          break;
        grid.input_image_name = strdup(token);
        break;

      case 4: /* grid range */
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        grid.range[X] = atof(token);
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        grid.range[Y] = atof(token);
        break;
      }
    }
  }

  if (verbose) {
    fprintf(stderr, "ornament - grid position: (%g, %g)\n", grid.pos[X],
            grid.pos[Y]);

    fprintf(stderr, "ornament - grid range: %g x %g\n", grid.range[X],
            grid.range[Y]);

    fprintf(stderr, "ornament - grid size: %dx%d\n", grid.size[X],
            grid.size[Y]);

    fprintf(stderr, "\nornament - specification file processed.\n\n");
  }

  InitializeGrid(&grid);
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
      fprintf(stderr, "ornament - start processing data.\n");

    InitializeQueryArray();

    for (i = 0; i < grid.size[Y] * grid.size[X]; i++)
      grid.data[i].objects = 0;

    /* process the data */
    while (CSGetData(&master, &module_id, two_modules, &turtle)) {

      ProcessQuery(&grid, master, module_id, &two_modules[0], &turtle);
    }

    DetermineResponse(&grid);

    if (verbose)
      fprintf(stderr, "ornament - data processed.\n");

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
