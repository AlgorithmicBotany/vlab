/*
  Environmental process - clover
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "lodepng.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifdef WIN32
/*#include <bmp.h>*/
#endif

#include "image.h"
#include "targa.h"
#include "clover.h"
#include "comm_lib.h"

/**** field specific variables ****/
/* cell type */
typedef struct {
  unsigned char objects, light, ratio;
} CELL_TYPE;

/* quaries */
#define QUERY_ARRAY_SIZE 1000
struct QUERY_UNIT_TYPE {
  float pos[2];
  int num_params;
  unsigned long dist; /* position in L-system string */
  int master;
};
typedef struct QUERY_UNIT_TYPE QUERY_UNIT_TYPE;

QUERY_UNIT_TYPE *queries = NULL;
int num_queries;
int query_array_size;

#define X 0
#define Y 1
#define Z 2

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

char remove_old_leaves, z_is_up;

float transmittance;

float transmittance_table[256]; /* powers of transmittance */

int LoadPNGTexture(grid_type *grid);

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
    fprintf(stderr, "clover - cannot allocate memory for queries!\n");
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
  int c, xsize, ysize;

  if ((c = strlen(grid->input_image_name)) > 3)
    if (strcmp(grid->input_image_name + c - 3, "tga") == 0) {
      int xsize, ysize;
      unsigned char *cbuf, *rowbuf, *ptrc;

      /* load in targa image */
      if (!loadTGAhead(grid->input_image_name, &TGAspec)) {
        fprintf(stderr, "clover: cannot open input targa image %s.\n",
                grid->input_image_name);
        return 0;
      }
      xsize = TGAspec.Xres;
      ysize = TGAspec.Yres;

      if ((cbuf = (unsigned char *)malloc(xsize * ysize * 2)) == NULL) {
        fprintf(stderr, "clover: cannot allocate memory for image %s!\n",
                grid->input_image_name);
        return 0;
      }

      if ((rowbuf = (unsigned char *)malloc(xsize * 3 *
                                            sizeof(unsigned char))) == NULL) {
        fprintf(stderr, "clover: cannot allocate memory for image %s!\n",
                grid->input_image_name);
        return 0;
      }

      /* read the image in - starting with the last row */
      /* image is stored in rows of R, G, and B. We need R,G,B for each
         pixel */
      for (y = ysize - 1; y >= 0; y--) {
        loadTGArow(&TGAspec, y, rowbuf);

        ptrc = cbuf + y * xsize * 2;
        for (x = 0; x < xsize; x++) {
          /* get red and  green */
          *(ptrc++) = rowbuf[x * 3 + 1];
          *(ptrc++) = rowbuf[x * 3 + 2];
        }
      }

      xratio = (float)(xsize - 1) / (float)(grid->size[X] - 1);
      yratio = (float)(ysize - 1) / (float)(grid->size[Y] - 1);

      /* set the 2d grid - assuming that it is bigger than the input image*/
      ptr = grid->data;

      for (y = 0; y < grid->size[Y]; y++)
        for (x = 0; x < grid->size[X]; x++) {
          ptr->light =
              cbuf[2 * xsize * (int)(y * yratio) + 2 * (int)(x * xratio) + 1];
          ptr->ratio =
              cbuf[2 * xsize * (int)(y * yratio) + 2 * (int)(x * xratio)];
          ptr->objects = 0;
          ptr++;
        }

      loadTGAfinish(&TGAspec);
      free(cbuf);
      free(rowbuf);


      return 1;
    }

  if (strcmp(grid->input_image_name + c - 3, "rgb") == 0) {

    /* otherwise open rgb image */
    if ((image = iopen(grid->input_image_name, "r", 0, 0, 0, 0, 0)) == NULL) {
      fprintf(stderr, "clover - cannot open image %s!\n",
              grid->input_image_name);
      return 0;
    }
    xsize = image->xsize;
    ysize = image->ysize;
    xratio = (float)(xsize - 1) / (float)(grid->size[X] - 1);
    yratio = (float)(ysize - 1) / (float)(grid->size[Y] - 1);

    /* one line buffer */
    if ((buf = (unsigned short *)malloc(xsize * ysize * 2 *
                                        sizeof(unsigned short))) == NULL) {
      fprintf(stderr,
              "clover - cannot allocate memory for one channel of image %s!\n",
              grid->input_image_name);
      return 0;
    }

    /* Read the image in. Only green channel is stored .*/
    /* Later on, R and B channel can be used for nutrients or obstales. */
    for (y = 0; y < ysize; y++) {
      /* red and green */
      getrow(image, buf + 2 * y * xsize, y, 0);
      getrow(image, buf + (2 * y + 1) * xsize, y, 1);
    }
    /* set the 2d grid - assuming that it is bigger than the input image*/
    ptr = grid->data;

    for (y = 0; y < grid->size[Y]; y++)
      for (x = 0; x < grid->size[X]; x++) {
        ptr->light =
            buf[xsize * (1 + 2 * (int)(y * yratio)) + (int)(x * xratio)];
        ptr->ratio = buf[xsize * 2 * (int)(y * yratio) + (int)(x * xratio)];
        ptr->objects = 0;
        ptr++;
      }

    free(buf);
    iclose(image);

    return 1;
  }
  return LoadPNGTexture(grid);
}

int LoadPNGTexture(grid_type *grid) // const char* filename, int index)
{
  const unsigned int MAX_SIZE = 4096;
  unsigned int error;
  unsigned char *image;
  unsigned int width, height;
  unsigned int jump = 1;
  unsigned short *buf;
  float xratio, yratio;
  CELL_TYPE *ptr;
  unsigned int x, y;

  error =
      lodepng_decode32_file(&image, &width, &height, grid->input_image_name);
  if (error) {
    printf("error %u: %s\n", error, lodepng_error_text(error));
    return 0;
  }
  /* one line buffer */
  if ((buf = (unsigned short *)malloc(width * height * 2 *
                                      sizeof(unsigned short))) == NULL) {
    fprintf(stderr,
            "clover - cannot allocate memory for one channel of image %s!\n",
            grid->input_image_name);
    return 0;
  }

  xratio = (float)(width - 1) / (float)(grid->size[X] - 1);
  yratio = (float)(height - 1) / (float)(grid->size[Y] - 1);

  if (width >= MAX_SIZE || height >= MAX_SIZE) {
    fprintf(stderr,
            "clover - width and height of input image must be stricly less "
            "than %u.\n",
            MAX_SIZE);
  }

  /* Read the image in. Only green channel is stored .*/
  /* Later on, R and B channel can be used for nutrients or obstales. */
  if (width / MAX_SIZE >= jump)
    jump = width / MAX_SIZE + 1;
  if (height / MAX_SIZE >= jump)
    jump = height / MAX_SIZE + 1;

  for (y = 0; y + jump - 1 < height - 1; y += jump) {
    for (x = 0; x + jump - 1 < width - 1; x += jump) {
      buf[x + 2 * y * width] =
          image[4 * (height - y - jump) * width + 4 * x + 0]; // r
      buf[x + (2 * y + 1) * width] =
          image[4 * (height - y - jump) * width + 4 * x + 1]; // g
    }
  }

  /*plot the pixels of the PNG file*/
  /* set the 2d grid - assuming that it is bigger than the input image*/
  ptr = grid->data;

  for (y = 0; y < (size_t)grid->size[Y]; y++)
    for (x = 0; x < (size_t)grid->size[X]; x++) {
      ptr->light = buf[width * (1 + 2 * (int)(y * yratio)) + (int)(x * xratio)];
      ptr->ratio = buf[width * 2 * (int)(y * yratio) + (int)(x * xratio)];

      ptr->objects = 0;
      ptr++;
    }

  return 1;
}

/****************************************************************************/
void InitializeGrid(grid_type *grid) {
  int x;

  if ((grid->data = (CELL_TYPE *)malloc(grid->size[X] * grid->size[Y] *
                                        sizeof(CELL_TYPE))) == NULL) {
    fprintf(stderr, "clover - cannot allocate memory for grid!\n");
    exit(0);
  }

  if (grid->input_image_name == NULL) {
    fprintf(stderr, "clover - no input image. Full light assumed.\n");

    for (x = 0; x < grid->size[Y] * grid->size[X]; x++) {
      grid->data[x].light = 255;
      grid->data[x].objects = 0;
    }
  } else
    LoadInitialData(grid);

  if (verbose)
    fprintf(stderr, "clover - grid initialized.\n");
}

/****************************************************************************/
void SetTransmittanceTable(void) {
  int i;

  transmittance_table[0] = 1.0;

  for (i = 1; i <= 255; i++)
    transmittance_table[i] = transmittance * transmittance_table[i - 1];
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
  signed char add;
  int node[2], range[2], x, y;

  if (comm_symbol->num_params < 1) {
    if (verbose)
      fprintf(stderr, "clover - one or more parameters for ?E required!\n");
    return;
  }

  if (tu->positionC < (z_is_up ? 2 : 3)) {
    fprintf(stderr,
            "clover - turtle position wasn't sent to the environment.\n");
    return;
  }

  if (!z_is_up) /* so we can use x and y coordinates as if z axis were up */
    tu->position[Y] = tu->position[Z];

  if (comm_symbol->num_params == 1 || comm_symbol->params[0].value == 0) {
    /* store query */

    if (num_queries >= query_array_size) {
      /* rellocate the array */
      query_array_size *= 2;
      if ((queries = (QUERY_UNIT_TYPE *)realloc(
               queries, query_array_size * sizeof(QUERY_UNIT_TYPE))) == NULL) {
        fprintf(stderr, "clover - cannot reallocate memory for query array!\n");
        exit(0);
      }
      if (verbose)
        fprintf(stderr, "clover - query array reallocated to size %d.\n",
                query_array_size);
    }

    queries[num_queries].dist = module_id;
    queries[num_queries].master = master;
    queries[num_queries].num_params = comm_symbol->num_params;

    for (c = X; c <= Y; c++)
      queries[num_queries].pos[c] = tu->position[c];

    num_queries++;

    return;
  }

  /* add (1) or remove (2) leaf */
  add = comm_symbol->params[0].value == 1 ? 1 : -1;

  if (comm_symbol->params[1].value < 0.0000001)
    return;

  radius = sqrt(comm_symbol->params[1].value / M_PI);

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
          if (((add == 1) && (cell->objects < 255)) ||
              ((add == -1) && (cell->objects > 0)))
            cell->objects += add;
      }
}

/****************************************************************************/
void DetermineResponse(grid_type *grid) {
  int q, c, node[2];
  CELL_TYPE *cell;
  Cmodule_type comm_symbol;

  if (verbose) {
    fprintf(stderr,
            "clover - start determining response for each out of %d"
            " queriess.\n",
            num_queries);
  }

  comm_symbol.params[0].set = 1;
  comm_symbol.params[1].set = 1;
  comm_symbol.params[2].set = 0;

  /* for all queries */
  for (q = 0; q < num_queries; q++) {
    for (c = X; c <= Y; c++)
      node[c] = floor((queries[q].pos[c] - grid->pos[c]) / grid->range[c] *
                      (float)grid->size[c]);

    cell = GetCell(grid, node[X], node[Y]);

    comm_symbol.params[0].value =
        (cell == NULL
             ? 0
             : (float)cell->light / 255.0 * transmittance_table[cell->objects]);

    if ((comm_symbol.num_params = queries[q].num_params) >= 2) {
      comm_symbol.num_params = 2;
      comm_symbol.params[1].value =
          cell == NULL ? 0 : (float)cell->ratio / 128.0;
    }

    CSSendData(queries[q].master, queries[q].dist, &comm_symbol);

    if (verbose)
      fprintf(stderr, "clover - perceived sunlight: %g.\n",
              comm_symbol.params[0].value);
  }
}

/****************************************************************************/
void ProcessArguments(int argc, char **argv) {
  FILE *fp;
  int i;
  char *keywords[] = {
      "grid size",         /*  0 */
      "grid position",     /*  1 */
      "verbose",           /*  2 */
      "transmittance",     /*  3 */
      "input image",       /*  4 */
      "grid range",        /*  5 */
      "remove old leaves", /*  6 */
      "z is up",           /*  7 */
      NULL                 /* the last item must be NULL! */
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

  remove_old_leaves = 0;
  z_is_up = 1;

  transmittance = 0.6;

  if (argc == 1) {
    printf("clover - not enough arguments!\n"
           "USAGE: clover -e environment_file spec_file\n");

    exit(0);
  }

  /* read in environment file */
  if ((fp = fopen(argv[1], "r")) == NULL)
    fprintf(stderr, "clover - cannot open specification file %s.\n", argv[1]);
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
                "clover - unknown directive %s in the specification file.\n",
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

      case 5: /* grid range */
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        grid.range[X] = atof(token);
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        grid.range[Y] = atof(token);
        break;

      case 2: /* verbose */
        if ((token = strtok(NULL, ",; \t:\n")) == NULL)
          break;
        if (strcmp(token, "on") == 0)
          verbose = 1;
        break;

      case 3: /* transmittance */
        if ((token = strtok(NULL, ",; \t:\n")) == NULL)
          break;
        transmittance = atof(token);
        if (transmittance == 0) {
          transmittance = 0.0001;
          fprintf(stderr,
                  "clover - transmittance cannot be zero. Setting %g"
                  " instead.\n",
                  transmittance);
        }
        break;

      case 4: /* input image */
        if ((token = strtok(NULL, ",; \t:\n")) == NULL)
          break;
        grid.input_image_name = strdup(token);
        break;

      case 6: /* remove old leaves */
        if ((token = strtok(NULL, ",; \t:\n")) == NULL)
          break;
        if (!strcmp(token, "on") || !strcmp(token, "yes"))
          remove_old_leaves = 1;
        break;

      case 7: /* z is up */
        if ((token = strtok(NULL, ",; \t:\n")) == NULL)
          break;
        if (!strcmp(token, "no") || !strcmp(token, "off"))
          z_is_up = 0;
        break;
      }
    }
  }

  if (verbose) {
    fprintf(stderr, "clover - grid position: (%g, %g)\n", grid.pos[X],
            grid.pos[Y]);

    fprintf(stderr, "clover - grid range: %g x %g\n", grid.range[X],
            grid.range[Y]);

    fprintf(stderr, "clover - grid size: %dx%d\n", grid.size[X], grid.size[Y]);

    fprintf(stderr, "clover - transmittance: %g\n", transmittance);

    fprintf(stderr, "\nclover - specification file processed.\n\n");
  }

  InitializeGrid(&grid);
  SetTransmittanceTable();
}

/****************************************************************************/
void MainLoop(void) {
  Cmodule_type two_modules[2];
  unsigned long module_id;
  int master;
  CTURTLE turtle;

  /* infinite loop - until signal 'exit' comes */
  for (;;) {
    CSBeginTransmission();

    if (verbose)
      fprintf(stderr, "clover - start processing data.\n");

    InitializeQueryArray();

    if (remove_old_leaves) {
      int x, y;
      CELL_TYPE *ptr = grid.data;

      for (y = 0; y < grid.size[Y]; y++)
        for (x = 0; x < grid.size[X]; x++) {
          ptr->objects = 0;
          ptr++;
        }
    }

    /* process the data */
    while (CSGetData(&master, &module_id, two_modules, &turtle)) {

      ProcessQuery(&grid, master, module_id, &two_modules[0], &turtle);
    }

    DetermineResponse(&grid);

    if (verbose)
      fprintf(stderr, "clover - data processed.\n");

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
