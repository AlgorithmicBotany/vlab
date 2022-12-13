/*
  Environmental process - density (2D)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "image.h"
#include "density.h"
#include "comm_lib.h"
#include "targa.h"

/**** field specific variables ****/
#define QUERY_ARRAY_SIZE 1000

struct item_type {
  float position[2];
  unsigned long dist;
  int master;
  char answer;
};
typedef struct item_type item_type;

item_type *queries;
int num_queries;
int query_array_size;

/* sources */
#define NUM_SOURCES 100
struct source_type {
  float position[2];
  float val;
};
typedef struct source_type source_type;

source_type sources[NUM_SOURCES];
int num_sources;

char verbose;
char one_point; /* how is the density field value determined */
int quant_step;
float sgamma;
char return_vector;

/* grid for density output */
typedef float DATA_TYPE;

#define X 0
#define Y 1
#define Z 2

struct grid_type {
  int size[3];    /* size of the grid (in nodes) */
  float range[3]; /* size in coordinates */
  float pos[3];   /* position of lower left front corner */
  DATA_TYPE min, max;
  char *outimagename;
};
typedef struct grid_type grid_type;

grid_type grid;

/* to output files just at some specific points */
struct ANIMINTERVAL {
  int from;
  int to;
  int step;
};
typedef struct ANIMINTERVAL ANIMINTERVAL;

#define MAXANIMINTERVALS 1000

ANIMINTERVAL intervals[MAXANIMINTERVALS];

/***** prototypes *****/
void ComputeDensity(int which, float *pos, float *dmin, float *gra, float *gr);
float GetDensityInPoint(int which, float x, float y);

/****************************************************************************/
void FreeFieldStructures(void) {
  if (queries != NULL)
    free(queries);
  queries = NULL;

  if (grid.outimagename != NULL)
    free(grid.outimagename);
  grid.outimagename = NULL;
}

/****************************************************************************/
void InitializeFieldStructures(void) {
  FreeFieldStructures();
  num_queries = 0;
  num_sources = 0;
  query_array_size = QUERY_ARRAY_SIZE;
  if ((queries = (item_type *)malloc(query_array_size * sizeof(item_type))) ==
      NULL) {
    fprintf(stderr, "density - cannot allocate memory for querries.\n");
    exit(0);
  }
}

/****************************************************************************/
int SaveImage(grid_type *grid) {
  unsigned short *row;
  unsigned char *rowb;
  IMAGE *oimage;
  int y, x;
  DATA_TYPE val;
  int size[2], c;
  float pt[2];
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
    if ((TGAspec.fp = fopen(grid->outimagename, "w")) == NULL) {
      fprintf(stderr, "density - cannot open targa file %s\n",
              grid->outimagename);
      return 0;
    }
    TGAspec.type = TGA_TRUECOLOR_RLE;
    TGAspec.Xres = size[X];
    TGAspec.Yres = size[Y];

    saveTGAhead(&TGAspec);

    if ((rowb = (unsigned char *)malloc(
             (unsigned int)(3 * size[X] * sizeof(unsigned char)))) == NULL)
      return 0;

    /* the bottom rows are always black */
    for (x = 0; x < 3 * size[X]; x++)
      rowb[x] = 0;

    for (y = grid->size[Y]; y < size[Y]; y++) {
      saveTGArow(&TGAspec, y, rowb);
    }

    for (y = grid->size[Y] - 1; y >= 0; y--) {
      for (x = 0; x < grid->size[X]; x++) {
        pt[X] = grid->pos[X] +
                ((float)x + 0.5) / (float)grid->size[X] * grid->range[X];
        pt[Y] = grid->pos[Y] +
                ((float)y + 0.5) / (float)grid->size[Y] * grid->range[Y];

        val = GetDensityInPoint(-1, pt[X], pt[Y]);

        val = (val - grid->min) / (grid->max - grid->min) * 255.0;

        if (val < 0)
          val = 0;
        if (val > 255)
          val = 255;

        val = ((int)val / quant_step) * quant_step;

        val = (DATA_TYPE)pow((double)val / 255.0, (double)sgamma) * 255.0;

        rowb[3 * x] = rowb[3 * x + 1] = rowb[3 * x + 2] = (unsigned char)val;
      }

      saveTGArow(&TGAspec, y, rowb);
    }

    saveTGAfinish(&TGAspec);

    free(rowb);

    if (verbose)
      fprintf(stderr, "density - image %s saved.\n", grid->outimagename);
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
    row[x] = 0;

  for (y = 0; y < grid->size[Y]; y++) {
    for (x = 0; x < grid->size[X]; x++) {
      pt[X] = grid->pos[X] +
              ((float)x + 0.5) / (float)grid->size[X] * grid->range[X];
      pt[Y] = grid->pos[Y] +
              ((float)y + 0.5) / (float)grid->size[Y] * grid->range[Y];

      val = GetDensityInPoint(-1, pt[X], pt[Y]);

      val = (val - grid->min) / (grid->max - grid->min) * 255.0;

      if (val < 0)
        val = 0;
      if (val > 255)
        val = 255;

      val = ((int)val / quant_step) * quant_step;

      val = powf(val / 255.0, sgamma) * 255.0;

      row[x] = row[size[X] + x] = row[2 * size[X] + x] = (unsigned short)val;
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
    fprintf(stderr, "density - image %s saved.\n", grid->outimagename);

  return 1;
}

/****************************************************************************/
float ApplySources(float x, float y, float val) {
  int ind;
  float d, aux[2];

  for (ind = 0; ind < num_sources; ind++) {
    aux[0] = x - sources[ind].position[0];
    aux[1] = y - sources[ind].position[1];

    d = 1.0 / (1.0 + sqrt(aux[0] * aux[0] + aux[1] * aux[1]));

    /* it behaves as 'val' object parts */
    /* val can be even negative */
    val += sources[ind].val * d;
  }
  return val;
}

/****************************************************************************/
/* ind determines which object point should be skipped */
float GetDensityInPoint(int which, float x, float y) {
  int ind;
  float d, aux[2];

  /* sum of squares of the reciprocals of the distances to all neighbouring
     parts of the pattern */
  d = 0;
  for (ind = 0; ind < which; ind++) {
    aux[0] = (x - queries[ind].position[0]);
    aux[1] = (y - queries[ind].position[1]);

    d += 1.0 / (1.0 + aux[0] * aux[0] + aux[1] * aux[1]);
  }
  for (ind = which + 1; ind < num_queries; ind++) {
    aux[0] = (x - queries[ind].position[0]);
    aux[1] = (y - queries[ind].position[1]);

    d += 1.0 / (1.0 + aux[0] * aux[0] + aux[1] * aux[1]);
  }

  return ApplySources(x, y, d);
}

/****************************************************************************/
#define XSAMPLES 6
#define YSAMPLES 6

void ComputeDensity(int which, float *pos, float *dmin, float *gra, float *gr) {
  float x, y;
  int i, j;
  float density[XSAMPLES][YSAMPLES];
  float grvec[2];

  if (one_point) {
    /* field value in the tested point - which is skipped in the computation */
    *dmin = GetDensityInPoint(which, pos[0], pos[1]);

    grvec[0] = -(GetDensityInPoint(which, pos[0] + 0.5, pos[1]) -
                 GetDensityInPoint(which, pos[0] - 0.5, pos[1]));
    grvec[1] = -(GetDensityInPoint(which, pos[0], pos[1] + 0.5) -
                 GetDensityInPoint(which, pos[0], pos[1] - 0.5));

  } else {
    /* compute density at sample points */
    for (i = 0, x = pos[0] - 0.5; i < XSAMPLES;
         i++, x += 1.0 / (float)(XSAMPLES - 1))
      for (j = 0, y = pos[1] - 0.5; j < YSAMPLES;
           j++, y += 1.0 / (float)(YSAMPLES - 1))
        density[i][j] = GetDensityInPoint(which, x, y);

    /* find the local minimum */
    *dmin = density[0][0];
    for (i = 0; i < XSAMPLES; i++)
      for (j = 0; j < YSAMPLES; j++)
        if (*dmin > density[i][j])
          *dmin = density[i][j];

    grvec[0] = -(density[XSAMPLES / 2][YSAMPLES / 2] -
                 density[XSAMPLES / 2 - 1][YSAMPLES / 2]) /
               (float)XSAMPLES;
    grvec[1] = -(density[XSAMPLES / 2][YSAMPLES / 2] -
                 density[XSAMPLES / 2][YSAMPLES / 2 - 1]) /
               (float)YSAMPLES;
  }

  if (verbose)
    fprintf(stderr, "Gradient vector is (%g,%g).\n", grvec[0], grvec[1]);

  if (return_vector) {
    /* return unnormalized 2D gradient vector */
    *gra = grvec[0];
    *gr = grvec[1];

    return;
  }

  *gr = sqrt(grvec[0] * grvec[0] + grvec[1] * grvec[1]);
  *gra = 90;

  if (*gr > 0.0) {
    /* normalize */
    grvec[0] /= *gr;
    grvec[1] /= *gr;

    *gra = acos(grvec[0]) / M_PI * 180;
    if (grvec[1] < 0)
      *gra = 360 - *gra;
  }
}

/****************************************************************************/
void DetermineResponse(void) {
  int i;
  Cmodule_type comm_symbol;

  comm_symbol.num_params = 3;

  for (i = 0; i < comm_symbol.num_params; i++)
    comm_symbol.params[i].set = 1;

  /* for all queries */
  for (i = 0; i < num_queries; i++)
    if (queries[i].answer) {
      ComputeDensity(i, queries[i].position, &comm_symbol.params[0].value,
                     &comm_symbol.params[1].value,
                     &comm_symbol.params[2].value);

      CSSendData(queries[i].master, queries[i].dist, &comm_symbol);
    }
}

/****************************************************************************/
void StoreQuery(int master, unsigned long module_id, Cmodule_type *comm_symbol,
                CTURTLE *tu) {
  int i;

  if (tu->positionC < 2) {
    fprintf(stderr,
            "density - turtle position wasn't sent to the environment.\n");
    return;
  }

  if (comm_symbol->num_params == 1) {
    /* source */
    if (num_sources >= NUM_SOURCES) {
      fprintf(stderr, "density - too many sources, the rest ignored!\n");
      return;
    }

    for (i = 0; i < 2; i++)
      sources[num_sources].position[i] = tu->position[i];

    /* either positive or negative */
    sources[num_sources].val = comm_symbol->params[0].value;
    num_sources++;

    if (verbose)
      fprintf(stderr, "density - source at (%f,%f) with value %f.\n",
              tu->position[0], tu->position[1], comm_symbol->params[0].value);

    return;
  }

  if (num_queries >= query_array_size) {
    /* reallocate */
    query_array_size *= 2;

    if ((queries = (item_type *)realloc(
             queries, query_array_size * sizeof(item_type))) == NULL) {
      fprintf(stderr, "density - cannot reallocate memory for querries.\n");
      exit(0);
    }
    if (verbose)
      fprintf(stderr, "density - queries reallocated. New size is %d.\n",
              query_array_size);
  }

  for (i = 0; i < 2; i++)
    queries[num_queries].position[i] = tu->position[i];

  queries[num_queries].dist = module_id;
  queries[num_queries].master = master;
  queries[num_queries].answer = comm_symbol->num_params == 3;

  num_queries++;
}

/************************************************************************/
/*
   according to the current derivation step and given set of intervals
   determines whether output file(s) should be saved.
   */

static int SaveFiles(int current_step) {
  int ind;

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
  int i, ind;
  char *keywords[] = {
      "verbose",           /*  0 */
      "range",             /*  1 */
      "position",          /*  2 */
      "output image",      /*  3 */
      "frame intervals",   /*  4 */
      "quantization step", /*  5 */
      "gamma",             /*  6 */
      "one point value",   /*  7 */
      "return vector",     /*  8 */
      NULL                 /* the last item must be NULL! */
  };
  char *token, input_line[255];

  if (argc == 1) {
    printf("density - not enough arguments!\n"
           "USAGE: density -e environment_file density_spec_file\n");
    exit(0);
  }

  /* defaults */
  verbose = 0;
  quant_step = 1;
  sgamma = 1.0;
  one_point = 0;
  return_vector = 0;

  grid.size[X] = 1;
  grid.size[Y] = 1;
  grid.size[Z] = 1;
  grid.range[X] = 2.0;
  grid.range[Y] = 2.0;
  grid.range[Z] = 2.0;
  grid.pos[X] = -1.0;
  grid.pos[Y] = -1.0;
  grid.pos[Z] = -1.0;

  grid.min = 0;
  grid.max = 1;

  grid.outimagename = NULL;

  intervals[0].from = 0;

  InitializeFieldStructures();

  /* read in environment file */
  if ((fp = fopen(argv[1], "r")) == NULL)
    fprintf(stderr, "density - cannot open specification file %s.\n", argv[1]);
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

      case 3: /* output image - only 2d */
        /* min and max */
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        grid.min = atof(token);
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        grid.max = atof(token);

        /* image size */
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        grid.size[X] = atof(token);
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        grid.size[Y] = atof(token);

        /* output image name */
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        grid.outimagename = strdup(token);
        break;

      case 4: /* frame intervals */
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

      case 5: /* quantization step */
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        quant_step = atof(token);
        break;

      case 6: /* gamma */
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        sgamma = atof(token);
        break;

      case 7: /* one point value */
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        if (strcmp(token, "on") == 0)
          one_point = 1;
        break;

      case 8: /* return vector */
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        if (strcmp(token, "yes") == 0)
          return_vector = 1;
        break;
      }
    }
  }
  if (verbose) {
    fprintf(stderr, "density - domain size:     %gx%gx%g\n", grid.range[X],
            grid.range[Y], grid.range[Z]);
    fprintf(stderr, "density - position:       (%g,%g,%g)\n", grid.pos[X],
            grid.pos[Y], grid.pos[Z]);
    fprintf(stderr, "density - number of nodes: %dx%dx%d\n", grid.size[X],
            grid.size[Y], grid.size[Z]);

    fprintf(stderr, "density - min: %f, max: %f.\n", grid.min, grid.max);

    if (grid.outimagename != NULL)
      fprintf(stderr, "Output image name: %s.\n", grid.outimagename);

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
  }
}

/****************************************************************************/
void MainLoop(void) {
  Cmodule_type two_modules[2];
  unsigned long module_id;
  int master;
  CTURTLE turtle;
  int in;
  int current_step;

  /* infinite loop - until signal 'exit' comes */
  for (;;) {
    CSBeginTransmission();

    if (verbose)
      fprintf(stderr, "density - start processing data.\n");

    num_queries = 0;
    num_sources = 0;
    in = 0;

    /* process the data */
    while (CSGetData(&master, &module_id, two_modules, &turtle)) {
      in = 1;

      StoreQuery(master, module_id, two_modules, &turtle);
    }

    DetermineResponse();

    current_step = module_id;

    if (in) {
      if (SaveFiles(current_step + 1)) {
        if (grid.outimagename != NULL) {
          if (verbose)
            fprintf(stderr, "density - saving image %s.\n", grid.outimagename);
          SaveImage(&grid);
        } else if (verbose)
          fprintf(stderr, "density - where is the image name?.\n");
      } else if (verbose)
        fprintf(stderr, "density - image not saved in this step.\n");
    }
    if (verbose)
      fprintf(stderr, "density - data processed.\n");

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
