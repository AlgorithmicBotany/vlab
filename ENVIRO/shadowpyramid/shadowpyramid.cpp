/*
  Environmental process for shadow-pyramid light environment
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

inline float ran(float max) { return max * drand48(); }

#include "lintrfc.h"
#include "comm_lib.h"
#include "shadowpyramid.hpp"

/**** field specific variables ****/
struct QueryModule {
  V3f pos;
  int master;
  unsigned long moduleID;
  int numParams;
};
std::vector<QueryModule> queries;

/* prototypes */
void FreeFieldStructures(void);

/* global variables */
LightModel *env = NULL;
char verbose;
float SampleRadius;

/****************************************************************************/
void FreeFieldStructures(void) {
  if (env != NULL)
    delete env;
  env = NULL;
  queries.clear();
}

/****************************************************************************/
void ProcessQuery(int master, unsigned long module_id,
                  Cmodule_type *two_modules, CTURTLE *tu) {
  if (tu->positionC < 3) {
    fprintf(
        stderr,
        "shadowpyramid - turtle position wasn't sent to the environment.\n");
    return;
  }

  QueryModule thisQuery;
  thisQuery.master = master;
  thisQuery.moduleID = module_id;
  thisQuery.numParams = two_modules[0].num_params;
  thisQuery.pos = V3f(tu->position[0], tu->position[1], tu->position[2]);

  queries.push_back(thisQuery);

  if (two_modules[0].params[0].value > 0) {
    env->shadow3D(thisQuery.pos, true);
    if (verbose) {
      int x, y, z;
      env->getVoxelIndex(thisQuery.pos, x, y, z);
      fprintf(stderr,
              "shadowpyramid - added shadow from object %d at (%g,%g,%g) "
              "(voxel (%d,%d,%d))\n",
              int(module_id), thisQuery.pos.x, thisQuery.pos.y, thisQuery.pos.z,
              x, y, z);
    }
  } else if (verbose)
    fprintf(stderr, "shadowpyramid - added query for object %d at (%g,%g,%g)\n",
            int(module_id), thisQuery.pos.x, thisQuery.pos.y, thisQuery.pos.z);
}

/****************************************************************************/
void DetermineResponse(void) {
  Cmodule_type comm_symbol;

  if (verbose)
    fprintf(stderr,
            "shadowpyramid - start determining response for each out of %d "
            "objects.\n",
            int(queries.size()));

  comm_symbol.params[0].set = 1;
  for (int c = 4; c < CMAXPARAMS; c++)
    comm_symbol.params[c].set = 0;

  /* for all leaves */
  for (size_t i = 0; i < queries.size(); ++i) {
    comm_symbol.num_params = queries[i].numParams;
    if (queries[i].numParams >= 4)
      comm_symbol.params[1].set = comm_symbol.params[2].set =
          comm_symbol.params[3].set = 1;
    else
      comm_symbol.params[1].set = comm_symbol.params[2].set =
          comm_symbol.params[3].set = 0;

    comm_symbol.params[0].value = env->getContinuousExposure(queries[i].pos);
    if (queries[i].numParams >= 4) {
      V3f dir = env->getContinuousEnvDir(queries[i].pos, SampleRadius);
      comm_symbol.params[1].value = dir.x;
      comm_symbol.params[2].value = dir.y;
      comm_symbol.params[3].value = dir.z;
    }

    CSSendData(queries[i].master, queries[i].moduleID, &comm_symbol);

    if (verbose) {
      fprintf(stderr, "shadowpyramid - perceived sunlight for %d is %g.\n",
              int(queries[i].moduleID), comm_symbol.params[0].value);

      if (comm_symbol.num_params >= 4)
        fprintf(stderr,
                "shadowpyramid - brightest direction for %d is (%g,%g,%g)\n",
                int(queries[i].moduleID), comm_symbol.params[1].value,
                comm_symbol.params[2].value, comm_symbol.params[3].value);
    }
  }
}

/****************************************************************************/
void ProcessArguments(int argc, char **argv) {
  FILE *fp;
  int i;
  const char *keywords[] = {"verbose",             /*  0 */
                            "grid size",           /*  1 */
                            "grid span",           /*  2 */
                            "shadow spread",       /*  3 */
                            "shadow depth",        /*  4 */
                            "shadow weight",       /*  5 */
                            "shadow dissipation",  /*  6 */
                            "light randomness",    /*  7 */
                            "light vertical bias", /*  8 */
                            "sample radius",       /*  9 */
                            NULL};
  char *token, input_line[255];

  FreeFieldStructures();

  /* defaults */
  verbose = 0;

  int GridSize = 201;
  float GridSpan = 200;

  float ShadowSpread = 1;
  int ShadowDepth = 6;
  float ShadowWeight = 0.58;
  float ShadowDissipation = 6.9;

  float LightVerticalBias = 0;
  float LightRandomness = 0;

  SampleRadius = 1.6;

  if (argc == 1) {
    printf("shadowpyramid - not enough arguments!\n"
           "USAGE: shadowpyramid -e environment_file light_spec_file\n");

    exit(0);
  }

  /* read in spec file */
  if ((fp = fopen(argv[1], "r")) == NULL)
    fprintf(stderr, "shadowpyramid - cannot open specification file %s.\n",
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
        fprintf(
            stderr,
            "shadowpyramid - unknown directive %s in the specification file.\n",
            token);
        continue;
      }

      switch (i) {
      case 0: /* verbose */
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        if (strcmp(token, "on") == 0)
          verbose = 1;
        break;

      case 1: /* grid size */
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        GridSize = atoi(token);
        break;

      case 2: /* grid span */
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        GridSpan = atof(token);
        break;

      case 3: /* shadow spread */
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        ShadowSpread = atof(token);
        break;

      case 4: /* shadow depth */
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        ShadowDepth = atoi(token);
        break;

      case 5: /* shadow weight */
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        ShadowWeight = atof(token);
        break;

      case 6: /* shadow dissipation */
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        ShadowDissipation = atof(token);
        break;

      case 7: /* light randomness */
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        LightRandomness = atof(token);
        break;

      case 8: /* light vertical bias */
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        LightVerticalBias = atof(token);
        break;

      case 9: /* sample radius */
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        SampleRadius = atof(token);
        break;
      }
    }
  }

  if (verbose) {
    fprintf(stderr, "shadowpyramid - grid size %d, grid span %g\n", GridSize,
            GridSpan);

    fprintf(stderr, "shadowpyramid - shadow parameters (%g,%d,%g,%g)\n",
            ShadowSpread, ShadowDepth, ShadowWeight, ShadowDissipation);

    fprintf(stderr,
            "shadowpyramid - light randomness %g, light vertical bias %g\n",
            LightRandomness, LightVerticalBias);

    fprintf(stderr, "shadowpyramid - sample radius %g\n", SampleRadius);

    fprintf(stderr, "\nshadowpyramid - specification file processed.\n\n");
  }

  env = new LightModel(GridSize, GridSpan);
  env->setParam(ShadowSpread, ShadowDepth, ShadowWeight, ShadowDissipation,
                LightRandomness, LightVerticalBias);
}

/****************************************************************************/
void MainLoop(void) {
  Cmodule_type two_modules[2];
  unsigned long module_id;
  CTURTLE turtle;
  int master;

  /* infinite loop - until signal 'exit' comes */
  for (;;) {
    CSBeginTransmission();

    if (verbose)
      fprintf(stderr, "shadowpyramid - start processing data.\n");

    env->initExposureMap();
    queries.clear();

    /* process the data */
    while (CSGetData(&master, &module_id, two_modules, &turtle)) {
      ProcessQuery(master, module_id, two_modules, &turtle);
    }

    DetermineResponse();

    if (verbose)
      fprintf(stderr, "shadowpyramid - data processed.\n");

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

  fprintf(stderr, "Environmental program %s initialized.\n", process_name);

  MainLoop();

  FreeFieldStructures();

  fprintf(stderr, "Environmental program %s exiting.\n", process_name);

  /* should be the last function called */
  CTerminate();

  return 1;
}
