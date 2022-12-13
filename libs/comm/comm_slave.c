/*
  Environmental process - main module
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32

#pragma warning(disable : 4201)
#pragma warning(disable : 4214)
#pragma warning(disable : 4514)
#pragma warning(error : 4013)

#include <windows.h>

#else

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>

#endif

#include "comm_lib.h"
#include "communication.h"
#include "compression.h"

#include "message.h"

static char *file_extension = NULL;
static char *host_name = NULL;

static field_type env_field[CMAXFIELDS];

static int num_fields = 0;

static int current_index; /* index of the current master to get data from */

static char first_clear = 1; /* must be 1 */

/****************************************************************************/
static void CSFreeStructures(void) {
  int index;

  for (index = 0; index < num_fields; index++) {
    if (env_field[index].host != NULL) {
      free(env_field[index].host);
      env_field[index].host = NULL;
    }

    ReleaseSharedMemory(env_field + index);

    env_field[index].specified = 0;
  }
}

/****************************************************************************/
int CSGetNumberOfMasters(void) { return num_fields; }

/****************************************************************************/
int CSInitializeStructures(void) {
  int i;

  if (first_clear) {
    /* no free, no releasing is necessary */
    first_clear = 0;

    for (i = 0; i < CMAXFIELDS; i++) {
      env_field[i].specified = 0;
#ifdef WIN32
      env_field[i].semid.hSem0 = env_field[i].semid.hSem1 = NULL;
#else
      env_field[i].semid = -1;
#endif
      env_field[i].shmid = -1;
      env_field[i].shmadd = NULL;
      env_field[i].in_fp = NULL;
      env_field[i].out_fp = NULL;
      env_field[i].formats.position = NULL;
      env_field[i].formats.heading = NULL;
      env_field[i].formats.left = NULL;
      env_field[i].formats.up = NULL;
      env_field[i].formats.line_width = NULL;
      env_field[i].formats.scale_factor = NULL;
    }
  } else
    CSFreeStructures();

  num_fields = 0;

  return 1;
}

/****************************************************************************/
int CSAddConnection(char *filename, int key, char *host_name) {
  int index;
  int s[2];

  /* no file no fun */
  if (filename == NULL)
    return -1;
  if (host_name == NULL)
    return -1;

  if (num_fields >= CMAXFIELDS) {
    Message("%s - max. number of slave fields reached, ignored!\n",
            process_name);
    return 0;
  }

  index = num_fields;

  env_field[index].specified = SLAVE;
  env_field[index].unique_key = key;

  /* sockets */
  if ((s[0] = CallSocket(host_name, env_field[index].unique_key)) < 0) {
    Message("%s - cannot connect to socket!\n", process_name);
    exit(0);
  }

  if ((s[1] = CallSocket(host_name, env_field[index].unique_key + 1)) < 0) {
    Message("%s - cannot connect to socket!\n", process_name);
    exit(0);
  }

  env_field[index].in_fp = fdopen(s[1], "r");
  env_field[index].out_fp = fdopen(s[0], "w");

  env_field[index].shmid = -1;
  env_field[index].shmadd = NULL;
  env_field[index].specified = SLAVE;

  if (!ReadInEnvironmentalFile(env_field + index, filename))
    return 0;

  /* for now, force socket communication - cannot be debugged, but
prevents crushes when the other types are used by accident */
  env_field[index].comm_type = COMM_SOCKETS;

  num_fields++;

  if (env_field[index].verbose)
    Message("%s - environment file processed\n", process_name);

  return 1;
}

/****************************************************************************/
/* Parameter - name of the environment file.
 */
static int InitializeSlave(char *env_filename) {
  char buff[1024];

  env_field[0].shmid = -1;
  env_field[0].shmadd = NULL;
  env_field[0].specified = SLAVE;

  if (!ReadInEnvironmentalFile(env_field, env_filename))
    return 0;

  switch (env_field[0].comm_type) {
  case COMM_MEMORY:
    /* initialize shared memory */
    if (!InitializeSharedMemory(env_field)) {
      env_field[0].specified = 0;
      return 0;
    }
    break;

  case COMM_FILES:
    strcpy(buff, FILENAME_FROM_FIELD);
    strcat(buff, file_extension);
    env_field[0].out_name = strdup(buff);

    strcpy(buff, FILENAME_TO_FIELD);
    strcat(buff, file_extension);
    env_field[0].in_name = strdup(buff);
    break;

  case COMM_PIPES:
    env_field[0].in_fp = stdin;
    env_field[0].out_fp = stdout;
    break;

  case COMM_SOCKETS:
    /* in_fp and out_fp already set */
    break;
  }

  num_fields = 1;

  if (env_field[0].verbose)
    Message("%s - environment file processed\n", process_name);

  return 1;
}

/****************************************************************************/
static char NextFileModule(char **str, Cmodule_type *module) {
  char *ptr = (*str)++; /* local for *str */
  char *end, *ptr2;

  /* get the current symbol and set up the basic token structure */
  module->symbol[0] = *(ptr++);

  if (module->symbol[0] == 0) {
    module->num_params = 0;
    return 0;
  }

  if ((module->symbol[0] == '~') || (module->symbol[0] == '@')) {
    module->symbol[1] = *(ptr++);
    module->symbol[2] = '\0';
  } else
    module->symbol[1] = '\0';

  module->num_params = 0;

  /* extract the parameters if present */
  if (*(ptr++) == '(') {
    if ((end = strchr(ptr, ')')) == NULL) {
      Message("%s - field - warning: Unmatched parenthesis.\n", process_name);
      return 0;
    }
    for (;;) {
      ptr2 = strchr(ptr, ',');

      if (module->num_params >= CMAXPARAMS) {
        Message("%s - field - warning: Too many parameters in module.\n",
                process_name);
        return 0;
      }

      if ((ptr2 == NULL) || (ptr2 > end))
        break;

      if (*ptr == ',')
        module->params[module->num_params++].set = 0;
      else {
        *ptr2 = 0;
        module->params[module->num_params].set = 1;
        module->params[module->num_params++].value = atof(ptr);
      }

      ptr = ptr2 + 1;
    }

    if (*ptr == ')')
      module->params[module->num_params++].set = 0;
    else {
      *end = 0;
      module->params[module->num_params].set = 1;
      module->params[module->num_params++].value = atof(ptr);
    }

    *str = end + 1; /* update *str */
  }

  return (module->symbol[0]);
}

/****************************************************************************/
static int BeginTransmissionIn(int index) {
  if (env_field[index].specified == 0)
    return 1;

  env_field[index].in_num = 0;
  return 1;
}

/****************************************************************************/
static int BeginTransmissionOut(int index) {
#ifndef WIN32
  struct sembuf sops;
#endif

  if (env_field[index].specified == 0)
    return 0;

  switch (env_field[index].comm_type) {
  case COMM_MEMORY:
  case COMM_FILES:
    /* wait on semaphore 0 */
    if (env_field[index].verbose)
      Message("%s - waiting on semaphore 0.\n", process_name);

#ifdef WIN32
    WaitForSingleObject(env_field[index].semid.hSem0, INFINITE);
#else
    sops.sem_num = 0;
    sops.sem_op = -1;
    sops.sem_flg = 0;
    semop(env_field[index].semid, &sops, 1);
#endif
    break;
  }

  env_field[index].out_flag = FIRST_CHUNK | LAST_CHUNK;
  env_field[index].out_num = 0;

  switch (env_field[index].comm_type) {
  case COMM_MEMORY:
    /* initialize the shared memory */
    env_field[index].out_end = 0;
    break;

  case COMM_FILES:
    /* open the data file */
    if ((env_field[index].out_fp = fopen(env_field[index].out_name, "w")) ==
        NULL) {
      Message("%s - cannot open data file %s.\n", process_name,
              env_field[index].out_name);
      return 0;
    }
    break;
  }

  if (env_field[index].verbose)
    Message("%s - BeginTransmission.\n", process_name);

  return 1;
}

/****************************************************************************/
int CSBeginTransmission(void) {
  BeginTransmissionIn(0); /* waiting for the first master */
  BeginTransmissionOut(0);

  current_index = 0;

  return 1;
}

/****************************************************************************/
static int EndTransmissionIn(int index) {
  if (env_field[index].specified == 0)
    return 0;

  if (env_field[index].comm_type == COMM_FILES) {
    if (NULL != env_field[index].in_fp) {
      fclose(env_field[index].in_fp);
      env_field[index].in_fp = NULL;
    }
  }

  return 1;
}

/****************************************************************************/
static int EndTransmissionOut(int index) {
#ifndef WIN32
  struct sembuf sops;
#endif
  char buff[1048], item[50];

  if (env_field[index].specified == 0)
    return 0;

  if (env_field[index].data_out) {
    env_field[index].data_out = 0;

    strcpy(item, DATA_END);
    strcat(item, "\n");
  } else
    item[0] = '\0';

  sprintf(buff, "%sControl: %d\n", item, env_field[index].out_flag);
  SaveOneItem(env_field + index, buff, 0);

  switch (env_field[index].comm_type) {
  case COMM_FILES:
    if (env_field[index].out_fp != NULL) {
      fclose(env_field[index].out_fp);
      env_field[index].out_fp = NULL;
    }
    break;
  case COMM_PIPES:
  case COMM_SOCKETS:
    fflush(env_field[index].out_fp);
    break;
  }

  switch (env_field[index].comm_type) {
  case COMM_MEMORY:
  case COMM_FILES:
    /* raise semaphore 1 */
#ifdef WIN32
    ReleaseSemaphore(env_field[index].semid.hSem1, 1, NULL);
#else
    sops.sem_num = 1;
    sops.sem_op = 1;
    sops.sem_flg = 0;
    semop(env_field[index].semid, &sops, 1);
#endif

    if (env_field[index].verbose)
      Message("%s - semaphore 1 raised (in EndTransmission).\n", process_name);
    break;
  }

  if (env_field[index].verbose)
    Message("%s - EndTransmission.\n", process_name);

  return 1;
}

/****************************************************************************/
/* returns 1 when the process should exit */
int CSEndTransmission(void) {
  int index;
  int exit = 1;

  /* finish communication with all masters */

  for (index = 0; index < num_fields; index++) {
    EndTransmissionIn(index);
    EndTransmissionOut(index);

    if ((env_field[index].in_flag & PROCESS_EXIT) == 0)
      exit = 0; /* only if all masters send exit, it dies */
  }

  return exit;
}

/****************************************************************************/
/* returns 0 when Control line not encountered, 1 when there is more data
   on the input for the same master (2 for the next master), and 3 when the
   input has finished.
   */
int GetControlStatus(char *token, int *index) {
  int current_step;

  if (strncmp(token, "Control", 7) != 0)
    return 0;

  current_step = 0;
  sscanf(token + 8, "%d %d", &env_field[*index].in_flag, &current_step);

  env_field[*index].current_step = current_step;

  if ((env_field[*index].in_flag & (LAST_CHUNK | PROCESS_EXIT)) != 0) {
    if (env_field[*index].verbose)
      Message("%s - the end of the last incoming chunk.\n", process_name);

    if (*index >= num_fields - 1)
      return 3; /* return 3, if this is the end of the last chunk
     from the last master */

    /* otherwise increase the master index */
    current_index++;
    *index = current_index;

    BeginTransmissionIn(*index);
    BeginTransmissionOut(*index);
    return 2;
  }

  EndTransmissionIn(*index);
  EndTransmissionOut(*index);

  BeginTransmissionIn(*index);
  BeginTransmissionOut(*index);

  return 1;
}

/****************************************************************************/
/* returns 0 when DATA_END encountered or data are not expected. */
int CSGetString(int *master, char *str, int length) {
  char *token;
  int index, c;

  index = current_index;

  for (;;) {
    if (env_field[index].specified == 0) {
      Message("%s - master number %d not specified!\n", process_name, index);
      return 0;
    }

    if (!env_field[index].expect_data_in && !env_field[index].strings_only) {
      return 0;
    }

    if ((token = LoadOneToken(env_field + index, "\n")) == NULL)
      return 0;

    if ((c = GetControlStatus(token, &index)) == 0)
      break;

    if (c == 3)
      /* return when the end of data form all masters */
      return 0;
  }

  if (!strcmp(token, DATA_END)) {
    env_field[index].expect_data_in = 0;
    return 0;
  }

  if (strlen(token) + 1 > (unsigned int)length)
    token[length - 1] = 0;

  strcpy(str, token);

  *master = index;
  return 1;
}

/****************************************************************************/
/* returns 0 when OK. Negative length means that item is a string. */
static int SendDataItem(int index, char *item, int length) {
  /* save item */
  if (SaveOneDataItem(env_field + index, item, length, 1) == 0) {
    /* saving failed */
    env_field[index].out_flag &= ~LAST_CHUNK; /* reset LAST_CHUNK */
    env_field[index].data_out = 0;
    EndTransmissionOut(index);
    env_field[index].data_out = 1;

    BeginTransmissionOut(index);
    env_field[index].out_flag &= ~FIRST_CHUNK; /* reset FIRST_CHUNK */

    SaveOneDataItem(env_field + index, item, length, 1);
    env_field[index].out_num++;
  }

  return 0;
}

/****************************************************************************/
/* returns 0 when OK */
int CSSendString(int index, char *item) {
  if ((index < 0) || (index >= num_fields))
    return 0;
  if (env_field[index].specified == 0)
    return 0;

  /* cannot output string when it is not expected */
  if (!env_field[index].data_out && !env_field[index].strings_only) {
    Message("%s - cannot send string. Not expected!\n", process_name);
    return 1;
  }

  return SendDataItem(index, item, -1);
}

/****************************************************************************/
/* returns 0 when OK */
int CSSendBinaryData(int index, char *item, int item_size, int nitems) {
  char len[4];
  int length, ret;
  char *buff;

  if ((index < 0) || (index >= num_fields))
    return 0;
  if (env_field[index].specified == 0)
    return 0;

  /* cannot output string when it is not expected */
  if (!env_field[index].binary_data) {
    Message("%s - cannot send binary data. Not expected!\n", process_name);
    return 1;
  }

  if ((buff = (char *)malloc(item_size * nitems * 2)) == NULL) {
    Message("%s - cannot allocate temporary buffer.\n", process_name);
    return 1;
  }

  length = RleEncode(item, item_size, nitems, buff);

  item = buff; /* buff must be kept to be able to free it later */

  len[0] = length & 0xff;
  len[1] = (length >> 8) & 0xff;
  len[2] = (length >> 16) & 0xff;
  len[3] = (length >> 24) & 0xff;

  if (SendDataItem(index, len, 4))
    return 1;
  while (length > LENGTH_OF_BINARY_CHUNK) {
    if (SendDataItem(index, item, LENGTH_OF_BINARY_CHUNK))
      return 1;
    item += LENGTH_OF_BINARY_CHUNK;
    length -= LENGTH_OF_BINARY_CHUNK;
  }

  ret = SendDataItem(index, item, length);

  free(buff);

  return ret;
}

/****************************************************************************/
/* returns 0 if no data avaliable */

int CSGetData(int *master, unsigned long *distance, Cmodule_type *two_modules,
              CTURTLE *turtle) {
  int i, c;
  char *ptr, *token;
  unsigned char flags;
  int index;

  index = current_index;

  if (env_field[index].specified == 0)
    return 0;
  if (env_field[index].strings_only)
    return 0;

  if (env_field[index].expect_data_in) {
    /* expect data */
    env_field[index].expect_data_in = 0;

    if (env_field[index].verbose)
      Message("%s - trying to reach %s.\n", process_name, DATA_END);

    do {
      if ((token = LoadOneToken(env_field + index, "\n")) == NULL)
        return 0;
      if (GetControlStatus(token, &index) >= 2) {
        Message("%s - warning! Input from a master unexpectedly finished!\n",
                process_name);
        break;
      }
    } while (strcmp(token, DATA_END));
  }

  for (;;) {
    if ((token = LoadOneToken(env_field + index, "\n")) == NULL)
      return 0;

    if (env_field[index].verbose)
      Message("%s - received token from master: %s\n", process_name, token);

    if ((c = GetControlStatus(token, &index)) == 0)
      /* control line not encountered */
      break;

    if (c == 3) {
      /* end of the last chunk from the last master */
      *distance = env_field[num_fields - 1].current_step;
      return 0;
    }

    /* otherwise continue - either with the next chunk or next master */
  }

  if (sscanf(token, "%lu", distance) != 1) {
    Message("%s - index of symbol E is missing!\n", process_name);
    return 0;
  }

  /* token contains ?E with the following symbol */
  if ((ptr = strchr(token, 'E')) == NULL) {
    Message("%s - string '%s' doesn't contain symbol E!\n", process_name,
            token);
    return 0;
  }

  NextFileModule(&ptr, &two_modules[0]);
  if (env_field[index].following_module)
    NextFileModule(&ptr, &two_modules[1]);
  else
    two_modules[1].symbol[0] = 0;

  /*  read in the turtle */
  if (env_field[index].formats.positionC > 0) {
    if ((token = LoadOneToken(env_field + index, "\n")) == NULL)
      return 0;
    for (i = 0; i < env_field[index].formats.positionS; i++)
      token++;

    sscanf(token, "%f %f %f", &turtle->position[0], &turtle->position[1],
           &turtle->position[2]);
  }
  turtle->positionC = env_field[index].formats.positionC;

  if (env_field[index].formats.headingC > 0) {
    if ((token = LoadOneToken(env_field + index, "\n")) == NULL)
      return 0;
    for (i = 0; i < env_field[index].formats.headingS; i++)
      token++;

    sscanf(token, "%f %f %f", &turtle->heading[0], &turtle->heading[1],
           &turtle->heading[2]);
  }
  turtle->headingC = env_field[index].formats.headingC;

  if (env_field[index].formats.leftC > 0) {
    if ((token = LoadOneToken(env_field + index, "\n")) == NULL)
      return 0;
    for (i = 0; i < env_field[index].formats.leftS; i++)
      token++;

    sscanf(token, "%f %f %f", &turtle->left[0], &turtle->left[1],
           &turtle->left[2]);
  }
  turtle->leftC = env_field[index].formats.leftC;

  if (env_field[index].formats.upC > 0) {
    if ((token = LoadOneToken(env_field + index, "\n")) == NULL)
      return 0;
    for (i = 0; i < env_field[index].formats.upS; i++)
      token++;

    sscanf(token, "%f %f %f", &turtle->up[0], &turtle->up[1], &turtle->up[2]);
  }
  turtle->upC = env_field[index].formats.upC;

  if (env_field[index].formats.line_widthC > 0) {
    if ((token = LoadOneToken(env_field + index, "\n")) == NULL)
      return 0;
    for (i = 0; i < env_field[index].formats.line_widthS; i++)
      token++;

    sscanf(token, "%f", &turtle->line_width);
  }
  turtle->line_widthC = env_field[index].formats.line_widthC;

  if (env_field[index].formats.scale_factorC > 0) {
    if ((token = LoadOneToken(env_field + index, "\n")) == NULL)
      return 0;
    for (i = 0; i < env_field[index].formats.scale_factorS; i++)
      token++;

    sscanf(token, "%f", &turtle->scale_factor);
  }
  turtle->scale_factorC = env_field[index].formats.scale_factorC;

  flags = env_field[index]
              .interpreted_modules[(unsigned char)two_modules[1].symbol[0]];

  if (env_field[index].following_module) {
    if ((flags & 0x80) || (flags & (0x40 >> two_modules[1].num_params))) {
      /* expect graphics */
      env_field[index].expect_data_in = 1;

      if ((token = LoadOneToken(env_field + index, "\n")) == NULL)
        return 0;

      if (strcmp(token, DATA_BEGIN)) {
        Message("%s - %s not present! Expect the worse.\n", process_name,
                DATA_BEGIN);
        return 0;
      }
    }
  }

  *master = index;

  return 1;
}

/****************************************************************************/
void CSSendData(int index, unsigned long dist, Cmodule_type *comm_symbol) {
  int i;
  char buff[256];
  char item[5000];

  if (env_field[index].specified == 0)
    return;

  if (comm_symbol->num_params > 0) {
    env_field[index].out_num++;

    sprintf(item, "%lu E(", dist);

    for (i = 0; i < comm_symbol->num_params;) {
      if (comm_symbol->params[i].set) {
        sprintf(buff, "%g", comm_symbol->params[i].value);
        strcat(item, buff);
      }

      if (++i < comm_symbol->num_params)
        strcat(item, ",");
    }
    strcat(item, ")\n");

    /* save item */
    if (SaveOneItem(env_field + index, item, 1) == 0) {
      /* saving failed */
      env_field[index].out_flag &= ~LAST_CHUNK; /* reset LAST_CHUNK */
      EndTransmissionOut(index);

      BeginTransmissionOut(index);
      env_field[index].out_flag &= ~FIRST_CHUNK; /* reset FIRST_CHUNK */

      SaveOneItem(env_field + index, item, 1);
      env_field[index].out_num++;
    }
  }
}

/****************************************************************************/
void CSTerminate(void) {
  char c;
  int index;

  for (index = 0; index < num_fields; index++) {
    if (env_field[index].specified == 0)
      continue;

    switch (env_field[index].comm_type) {
    case COMM_PIPES:
    case COMM_SOCKETS:
      /* confirm exit */
      c = 1;
      fwrite(&c, 1, 1, env_field[index].out_fp);
      fflush(env_field[index].out_fp);
      break;
    }
  }

  CSFreeStructures();
}

/****************************************************************************/
void CSMainLoop(int (*AnswerQuery)(Cmodule_type *, CTURTLE *)) {
  Cmodule_type two_modules[2];
  unsigned long module_id;
  int master;
  CTURTLE turtle;

  for (;;) {
    CSBeginTransmission();

    /* process the data */
    while (CSGetData(&master, &module_id, two_modules, &turtle)) {

      if ((*AnswerQuery)(two_modules, &turtle))
        CSSendData(master, module_id, &two_modules[0]);
    }

    if (CSEndTransmission())
      break;
  }
}

/****************************************************************************/
void CSInitialize(int *argc, char ***argv) {
  char c;
  char *env_filename, *program_name = (*argv)[0], *ptr;
  int s[2];

  CSInitializeStructures();

  if (program_name != NULL) {
    /* remove possible path */
    if ((ptr = strrchr(program_name, '/')) != NULL)
      program_name = ptr + 1;

    if (strlen(program_name) > sizeof(process_name)) {
      strncpy(process_name, program_name, sizeof(process_name) - 1);
      process_name[sizeof(process_name) - 1] = 0;
    } else
      strcpy(process_name, program_name);
  } else
    strcpy(process_name, "field");

  /* default values */
  env_filename = NULL;
  host_name = NULL;
  env_field[0].unique_key = 0;

  /* get switches */
  while ((*argc > 1) && ((*argv)[1][0] == '-'))
    switch ((*argv)[1][1]) {
    case 'e':
      if (strncmp(&(*argv)[1][1], "ext", 3) == 0)
        /* communication file extension */
        file_extension = strdup((*argv)[2]);
      else
        /* environmental file */
        env_filename = (*argv)[2];

      (*argc) -= 2;
      (*argv) += 2;
      break;

    case 'k': /* unique key */
      env_field[0].unique_key = atoi((*argv)[2]);
      (*argc) -= 2;
      (*argv) += 2;
      break;

    case 'h': /* host */
      host_name = strdup((*argv)[2]);
      (*argc) -= 2;
      (*argv) += 2;
      break;

    case 'C': /* distributed communication */
      CInitialize(process_name, (*argv)[2]);
      (*argc) -= 2;
      (*argv) += 2;
      return;

    default:
      goto out;
    }

out:
  env_field[0].specified = SLAVE;

  if (host_name != NULL) {
    /* sockets */
    if ((s[0] = CallSocket(host_name, env_field[0].unique_key)) < 0) {
      Message("%s - cannot connect to socket!\n", process_name);
      exit(0);
    }

    if ((s[1] = CallSocket(host_name, env_field[0].unique_key + 1)) < 0) {
      Message("%s - cannot connect to socket!\n", process_name);
      exit(0);
    }

    env_field[0].in_fp = fdopen(s[1], "r");
    env_field[0].out_fp = fdopen(s[0], "w");

    c = 1;

    fwrite(&c, 1, 1, env_field[0].out_fp);
    fflush(env_field[0].out_fp);

    Message("%s - successful execution confirmed.\n", process_name);
  } else if (env_field[0].unique_key == 0) {
    /* pipes */

    /* confirm execution by sending 1 to stdout as fast as possible */
    c = 1;
    fwrite(&c, 1, 1, stdout);
    fflush(stdout);
  } else
    /* files or shared memory */
    InitializeSemaphores(env_field); /* set semaphores as fast as possible */

  if (env_field[0].verbose)
    Message("%s - successful execution confirmed.\n", process_name);

  InitializeSlave(env_filename);

  if (env_field[0].verbose)
    Message("Process %s initialized.\n", process_name);
}
