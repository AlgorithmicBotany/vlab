/*
  Modified version of honda81.
  The modification consists of equating vigor with the radius of circles.
  Consequently, the radius read from the environmental file is ignored.
  Modification date: July 29, 1997.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ecosystem.h"
#include "grid.h"
#include "comm_lib.h"

/**** field specific variables ****/
#define QUERY_ARRAY_SIZE 100

item_type *queries;
int num_queries;
int query_array_size;

char verbose;
char is3d;
char vigor;

float min_pos[3];
float max_pos[3];

/****************************************************************************/
void FreeFieldStructures(void) {
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
    fprintf(stderr, "ecosystem - cannot allocate memory for querries.\n");
    exit(0);
  }
}

/****************************************************************************/
void DetermineResponse(void) {
  int i;
  Cmodule_type comm_symbol;

  ResetGrid(min_pos, max_pos);

  /* add all objects */
  for (i = 0; i < num_queries; i++)
    AddObject(queries + i);

  comm_symbol.num_params = 1;
  comm_symbol.params[0].set = 1;

  /* for all queries determine intersection with another sphere by checking all
   spheres in all voxes occyppied by the current sphere */
  for (i = 0; i < num_queries; i++) {

    /* returns 0 if there is an intersection */
    comm_symbol.params[0].value = TestIntersection(queries + i);

    CSSendData(queries[i].master, queries[i].dist, &comm_symbol);
  }
}

/****************************************************************************/
void StoreQuery(int master, unsigned long module_id, Cmodule_type *comm_symbol,
                CTURTLE *tu) {
  int c;

  if (tu->positionC < 3) {
    fprintf(stderr,
            "ecosystem - turtle position wasn't sent to the environment.\n");
    return;
  }

  if (comm_symbol->num_params < 1 + vigor) {
    fprintf(stderr, "ecosystem - not enough parameters associated with ?E.\n");
    return;
  }

  if (num_queries >= query_array_size) {
    /* reallocate */
    query_array_size *= 2;

    if ((queries = (item_type *)realloc(
             queries, query_array_size * sizeof(item_type))) == NULL) {
      fprintf(stderr, "ecosystem - cannot reallocate memory for querries.\n");
      exit(0);
    }
    if (verbose)
      fprintf(stderr, "ecosystem - queries reallocated. New size is %d.\n",
              query_array_size);
  }

  queries[num_queries].position[0] = tu->position[0];
  if (is3d) {
    /* 3d case */
    queries[num_queries].position[1] = tu->position[1];
    queries[num_queries].position[2] = tu->position[2];
  } else
  /* 2d case */ {
    queries[num_queries].position[1] = tu->position[2];
    queries[num_queries].position[2] = 0;
  }

  queries[num_queries].dist = module_id;
  queries[num_queries].master = master;
  queries[num_queries].radius = comm_symbol->params[0].value;

  if (vigor) {
    queries[num_queries].vigor =
        (comm_symbol->num_params > 1 ? comm_symbol->params[1].value : 1);
    queries[num_queries].index = 0;
  } else {
    queries[num_queries].vigor = 1;
    queries[num_queries].index =
        (comm_symbol->num_params > 1 ? comm_symbol->params[1].value : 0);
  }

  queries[num_queries].removed = 0;

  /* get the range for the grid */
  if (num_queries == 0) {
    for (c = 0; c < (is3d ? 3 : 2); c++) {
      min_pos[c] = tu->position[c] - queries[num_queries].radius;
      max_pos[c] = tu->position[c] + queries[num_queries].radius;
    }
  } else
    for (c = 0; c < (is3d ? 3 : 2); c++) {
      if (min_pos[c] >
          (queries[num_queries].position[c] - queries[num_queries].radius))
        min_pos[c] =
            (queries[num_queries].position[c] - queries[num_queries].radius);

      if (max_pos[c] <
          (queries[num_queries].position[c] + queries[num_queries].radius))
        max_pos[c] =
            (queries[num_queries].position[c] + queries[num_queries].radius);
    }

  num_queries++;
}

/****************************************************************************/
void ProcessArguments(int argc, char **argv) {
  FILE *fp;
  int i;
  char *keywords[] = {
      "verbose",   /*  0 */
      "grid size", /*  1 */
      "3d case",   /*  2 */
      "vigor",     /*  3 */
      NULL         /* the last item must be NULL! */
  };
  char *token, input_line[255];
  int size[3];

  if (argc == 1) {
    printf("ecosystem - not enough arguments!\n"
           "USAGE: ecosystem -e environment_file ecosystem_spec_file\n");
    exit(0);
  }

  /* defaults */
  verbose = 0;
  size[0] = size[1] = size[2] = 1;

  is3d = 0;
  vigor = 0;

  InitializeFieldStructures();

  /* read in environment file */
  if ((fp = fopen(argv[1], "r")) == NULL)
    fprintf(stderr, "ecosystem - cannot open specification file %s.\n",
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
                "ecosystem - unknown directive %s in the specification file.\n",
                token);
        continue;
      }

      switch (i) {
      case 0: /* verbose */
        token = strtok(NULL, "x,; \t:\n");
        if (token == NULL)
          break;
        if (!strcmp(token, "on") || !strcmp(token, "1"))
          verbose = 1;
        break;

      case 1: /* grid size */
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        if ((size[0] = atoi(token)) < 1)
          size[0] = 1;
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        if ((size[1] = atoi(token)) < 1)
          size[1] = 1;
        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        if ((size[2] = atoi(token)) < 1)
          size[2] = 1;

        break;

      case 2: /* is 3d case */
        token = strtok(NULL, "x,; \t:\n");
        if (token == NULL)
          break;
        if (!strcmp(token, "on") || !strcmp(token, "yes"))
          is3d = 1;
        break;

      case 3: /* vigor */
        token = strtok(NULL, "x,; \t:\n");
        if (token == NULL)
          break;
        if (!strcmp(token, "on"))
          vigor = 1;
        break;
      }
    }
  }

  InitializeGrid(size);
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
    if (verbose)
      fprintf(stderr, "ecosystem - start processing data.\n");

    num_queries = 0;

    CSBeginTransmission();

    /* process the data */
    while (CSGetData(&master, &module_id, two_modules, &turtle)) {
      if (verbose) {
        fprintf(stderr, "ecosystem - comm. symbol has %d parameters:\n      ",
                two_modules[0].num_params);
        for (i = 0; i < two_modules[0].num_params; i++)
          fprintf(stderr, " %g", two_modules[0].params[i].value);
        fprintf(stderr, "\n");

        fprintf(stderr, "\n");
      }

      StoreQuery(master, module_id, two_modules, &turtle);
    }

    DetermineResponse();

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
