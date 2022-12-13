/*
  Environmental process - skeleton
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include "comm_lib.h"

#define MAX_FIELDS 20
static int env_fields[MAX_FIELDS];
static int num_fields;

char verbose;

/****************************************************************************/
void FreeFieldStructures(void) {
  CMFreeStructures();

  num_fields = 0;
}

/****************************************************************************/
int ReceiveCommModulePars(int master, unsigned long distance,
                          Cmodule_type *comm_module, int index) {
  int i;

  for (i = comm_module->num_params; i > 0; i--) {
    comm_module->params[i].value = comm_module->params[i - 1].value;
    comm_module->params[i].set = comm_module->params[i - 1].set;
  }

  comm_module->num_params++;

  /* find index in env_fields */
  for (i = 0; i < CMAXFIELDS; i++)
    if (env_fields[i] == index)
      break;

  comm_module->params[0].set = 0;

  CSSendData(master, distance, comm_module);

  return 1;
}

/****************************************************************************/
/* process additional arguments of the field process, usually a text */
/* specification file. Initialize field structures accordingly. */
void ProcessArguments(int argc, char **argv) {
  FILE *fp;
  int i;
  char *keywords[] = {
      "field communication file", /*  0 */
      "verbose",                  /*  1 */
      NULL                        /* the last item must be NULL! */
  };
  char *token, input_line[255];

  /* defaults */
  verbose = 0;

  num_fields = 0;

  if (argc == 1) {
    printf("multiple - not enough arguments!\n"
           "USAGE: multiple -e environment_file multiple_spec_file\n");

    exit(0);
  }

  /* read in environment file */
  if ((fp = fopen(argv[1], "r")) == NULL)
    fprintf(stderr, "multiple - cannot open specification file %s.\n", argv[1]);
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
                "multiple - unknown directive %s in the specification file.\n",
                token);
        continue;
      }

      switch (i) {
      case 0: /* field communication file */
        if ((token = strtok(NULL, ",; \t:\n")) == NULL)
          break;

        if (verbose)
          fprintf(stderr, "multiple - initializing %d. process\n",
                  num_fields + 1);

        if ((env_fields[num_fields] = CMAddProcess(token)) != -1) {
          num_fields++;
        }

        break;

      case 1: /* verbose */
        if ((token = strtok(NULL, ",; \t:\n")) == NULL)
          break;
        if (strcmp(token, "on") == 0)
          verbose = 1;
        break;
      }
    }
  }

  if (verbose) {
    fprintf(stderr, "\nmultiple - %d process(es) runing.\n", num_fields);
    fprintf(stderr, "multiple - specification file processed.\n");
  }
}

/****************************************************************************/
void MainLoop(void) {
  Cmodule_type two_modules[2], comm_module;
  unsigned long module_id;
  CTURTLE turtle;
  char string[2048];
  int current_step, master;
  int in;
  int i, index;
  int which;

  /* infinite loop - until signal 'exit' comes */
  for (;;) {
    in = 0;

    /* begin receiving */
    CSBeginTransmission();

    /* begin sending to all valid subprocesses */
    CMBeginTransmission();

    if (verbose)
      fprintf(stderr, "multiple - start processing data.\n");

    /* process the data */
    while (CSGetData(&master, &module_id, two_modules, &turtle)) {
      if (two_modules[0].num_params > 0) {
        if ((two_modules[0].params[0].value <= 0) ||
            (two_modules[0].params[0].value > num_fields)) {
          fprintf(stderr, "multiple - process index out of range.\n");
          continue;
        }

        which = env_fields[(int)two_modules[0].params[0].value - 1];

        /* move parameters one down */
        for (i = 0; i < two_modules[0].num_params - 1; i++) {
          two_modules[0].params[i].value = two_modules[0].params[i + 1].value;
          two_modules[0].params[i].set = two_modules[0].params[i + 1].set;
        }

        two_modules[0].num_params--;

        if (verbose)
          fprintf(stderr, "Sending communication symbol to process %d.\n",
                  which);

        if (CMSendCommSymbol(which, module_id, two_modules, &turtle)) {
          /* send graphics */
          while (CSGetString(&master, string, sizeof(string)))
            CMSendString(which, string);
        }

        /* in case the respond is already coming */
        while (CMGetCommunicationModule(which, &module_id, &comm_module)) {
          for (i = comm_module.num_params; i > 0; i--) {
            comm_module.params[i].value = comm_module.params[i - 1].value;
            comm_module.params[i].set = comm_module.params[i - 1].set;
          }

          comm_module.num_params++;

          comm_module.params[0].set = 0;

          CSSendData(0, module_id, &comm_module);
        }
      }
    }
    current_step = module_id;

    CMEndTransmission(current_step);

    /* process the rest of the input */
    for (index = 0; index < CMGetNumberOfSlaves(); index++)
      while (CMGetCommunicationModule(index, &module_id, &comm_module)) {
        for (i = comm_module.num_params; i > 0; i--) {
          comm_module.params[i].value = comm_module.params[i - 1].value;
          comm_module.params[i].set = comm_module.params[i - 1].set;
        }

        comm_module.num_params++;

        comm_module.params[0].set = 0;

        CSSendData(0, module_id, &comm_module);
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
