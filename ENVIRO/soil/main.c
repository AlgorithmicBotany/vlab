/*
  Environmental process - main module
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bstring.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include "communication.h"
#include "comm_local.h"
#include "main.h"

static struct field_type {
  char comm_type;      /* memory / files */
  key_t semaphore_key; /* unique key for allocating semaphores and the
                        * shared memory */
  int semid;           /* id of a pair of semaphores */

  int in_num; /* number of queries */
  int out_num;

  int in_flag;  /* combination of FIRST_CHUNK and LAST_CHUNK,
                   or PROCESS_EXIT */
  int out_flag; /* combination of FIRST_CHUNK and LAST_CHUNK */

  FILE *in_fp; /* file pointer of the data file/pipe */
  FILE *out_fp;
  char *extension; /* file additional extension */

  int shmid;                  /* index of shared memory - if files not used */
  shared_memory_type *shmadd; /* address of shared memory  */
  int out_end;                /* index of the active end of the shared string */
} env_field;

static char *env_filename; /* name of the environment file */

turtle_num_type tu_par;  /* number of parameters for each tutle item */
turtle_num_type tu_skip; /* how much text should be skiped before parameter */

/* local verbose - for debugging */
static char verbose = 0;

/****************************************************************************/
int InitializeSharedMemory(void) {
  /* set up the shared memory */
  if ((env_field.shmid = shmget(env_field.semaphore_key - 1,
                                sizeof(shared_memory_type), 0600)) == -1) {
    fprintf(stderr, "Field process - shared memory access failed! Exiting.\n");
    exit(0);
  }

  /* bind the shared memory - it is initialized in process 2 */
  if ((env_field.shmadd = (shared_memory_type *)(shmat(
           env_field.shmid, NULL, 0))) == (shared_memory_type *)-1) {
    fprintf(stderr,
            "Field process - error bindind shared memory address! Exiting.\n");
    exit(0);
  }

  return 1;
}

/****************************************************************************/
/* MAKE SURE THAT THIS PROCEDURE IS ALWAYS CALLED BEFORE EXIT!!!*/
void ReleaseSharedMemory(void) {
  if (env_field.shmadd != NULL) {
    /* unbind the shared memory */
    if (shmdt(env_field.shmadd) != 0) {
      fprintf(stderr, "Field process - error unbinding address! Exiting.\n");
      return;
    }
    env_field.shmadd = NULL;
  }
}

/****************************************************************************/
int InitializeSemaphores(void) {
  union semun arg;

  /* get a pair of semaphores */
  if ((env_field.semid = semget(env_field.semaphore_key, 2, 0600)) == -1) {
    fprintf(stderr, "Field process - cannot get semaphore id!\n");
    exit(0);
  }

  /* set the semval to 0 */
  arg.val = 0;
  semctl(env_field.semid, 1, SETVAL, arg);

  return 1;
}

/****************************************************************************/
void FreeCommunicationStructures(void) {
  ReleaseSharedMemory();

  FreeFieldStructures();
}

/****************************************************************************/
/* couunts number of parameters in a printf-like format string */
int CountParameters(char *str, int *skip) {
  int first, num = 0;
  char *beg = str;

  if (str == NULL)
    return 0;

  first = 1;
  *skip = 0;

  while ((str = strchr(str, '%')) != NULL) {
    if (*(++str) != '%') {
      if (first) {
        first = 0;
        *skip = (int)(str - 1 - beg);
      }
      num++;
    } else
      str++;
  }

  if (num > 3)
    return 3;
  return num;
}
/****************************************************************************/
/* Parameter - name of the environment file.
 */
int InitializeCommunication(void) {
  FILE *fp;
  int i;
  char *keywords[] = {
      "communication type",  /* 0 */
      "executable",          /* 1 */
      "turtle position",     /* 2 */
      "turtle heading",      /* 3 */
      "turtle left",         /* 4 */
      "turtle up",           /* 5 */
      "turtle line width",   /* 6 */
      "turtle scale factor", /* 7 */
      NULL                   /* the last item must be NULL! */
  };
  char *token, input_line[255];

  /* default(s) */
  env_field.comm_type = COMM_PIPES;

  /* no free, no releasing is necessary */
  env_field.shmid = -1;
  env_field.shmadd = NULL;

  tu_par.position = 0;
  tu_par.heading = 0;
  tu_par.left = 0;
  tu_par.up = 0;
  tu_par.line_width = 0;
  tu_par.scale_factor = 0;

  /* no file - try to use shared memory */
  if (env_filename == NULL)
    fprintf(stderr, "No environment file specified, using shared memory.\n");
  else {
    /* read in environment file */
    if ((fp = fopen(env_filename, "r")) == NULL) {
      fprintf(stderr, "Cannot open environment file %s, using shared memory.\n",
              env_filename);
    } else {
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
          fprintf(stderr, "Unknown directive %s in environmental file.\n",
                  token);
          continue;
        }

        switch (i) {
        case 0: /* communication type */
          token = strtok(NULL, " \t:\n");

          if (strcmp(token, "files") == 0) {
            env_field.comm_type = COMM_FILES;
            break;
          }

          if (strcmp(token, "memory") == 0) {
            env_field.comm_type = COMM_MEMORY;
            break;
          }

          if (strcmp(token, "pipes") == 0) {
            env_field.comm_type = COMM_PIPES;
            break;
          }

          fprintf(stderr,
                  "field - unknown mean of communication: %s! Pipes used.\n",
                  token);
          break;

        case 1: /* executable */
          /* ignored */
          break;

        case 2:                       /* turtle position */
          token = strtok(NULL, "\n"); /* till eol */
          tu_par.position = CountParameters(token, &tu_skip.position);
          break;

        case 3:                       /* turtle heading */
          token = strtok(NULL, "\n"); /* till eol */
          tu_par.heading = CountParameters(token, &tu_skip.heading);
          break;

        case 4:                       /* turtle left */
          token = strtok(NULL, "\n"); /* till eol */
          tu_par.left = CountParameters(token, &tu_skip.left);
          break;

        case 5:                       /* turtle up */
          token = strtok(NULL, "\n"); /* till eol */
          tu_par.up = CountParameters(token, &tu_skip.up);
          break;

        case 6:                       /* turtle line width */
          token = strtok(NULL, "\n"); /* till eol */
          tu_par.line_width = CountParameters(token, &tu_skip.line_width);
          break;

        case 7:                       /* turtle scale factor */
          token = strtok(NULL, "\n"); /* till eol */
          tu_par.scale_factor = CountParameters(token, &tu_skip.scale_factor);
          break;

        default:
          fprintf(stderr, "Environment file: unknown keyword %s.\n", token);
        }
      }
    }
  }

  switch (env_field.comm_type) {
  case COMM_MEMORY:
    /* initialize shared memory */
    InitializeSharedMemory();
    break;

  case COMM_PIPES:
    env_field.in_fp = stdin;
    env_field.out_fp = stdout;

    break;
  }

  return 1;
}

/****************************************************************************/
/****************************************************************************/
/* structures  visible outside */
ETURTLE *tu;             /* pointer to the turtle */
module_type comm_symbol; /* communication symbol */
module_type next_symbol; /* symbol following the communication symbol */

/* for file communication */
/*static char line[MAX_LINE_LEN];    / * to store the curent piece of string */

static ETURTLE loc_turtle; /* to store the turtle */
unsigned long dist;        /* index of the query */

/****************************************************************************/
char skiptok_sep[256] = {0};

/* works similarly to strtok, but updates the source pointer and places it
   behind the first found separator following the found token. */
char *skiptok(char **str, char *sep) {
  int i, len;
  char *ptr = *str;
  char *beg = NULL;

  /* trivial cases */
  if (ptr == NULL)
    return NULL;
  if (*ptr == 0)
    return NULL;

  /* set array of separator flags - assumed to be initialized to 0 */
  len = strlen(sep);
  for (i = 0; i <= len; i++)
    skiptok_sep[sep[i]] = 1;

  /* skip separators */
  while (skiptok_sep[*ptr])
    if (*ptr++ == 0)
      goto end; /* separators till the end of the string */

  /* ptr points to the first token character */
  beg = ptr;

  while (!skiptok_sep[*(++ptr)])
    ; /* '\0' is always a separator */

  if (*ptr == 0)
    *str = ptr;
  else {
    *str = ptr + 1; /* just behind the separator */
    *ptr = 0;       /* terminate the token */
  }

end:
  for (i = 0; i <= len; i++)
    skiptok_sep[sep[i]] = 0;

  return beg;
}

/****************************************************************************/
#define LLEN 2048
char line[LLEN];
char *act_ptr;

char *LoadOneToken(char *sep) {
  char *token;
  char filename[1024];

  switch (env_field.comm_type) {
  case COMM_MEMORY:
    if (env_field.in_num++ == 0)
      act_ptr = env_field.shmadd->to_field;

    return skiptok(&act_ptr, sep);

  case COMM_FILES:
  case COMM_PIPES:
    if (env_field.in_num++ == 0) {
      if (env_field.comm_type == COMM_FILES) {
        /* open the data file */
        strcpy(filename, FILENAME_TO_FIELD);
        if (env_field.extension != NULL)
          strcat(filename, env_field.extension);

        if ((env_field.in_fp = fopen(filename, "r")) == NULL) {
          fprintf(stderr, "Error - field process cannot open data file %s.\n",
                  filename);
          return NULL;
        }
      }
      goto get_line;
    }

    /* get another token from the current line */
    if ((token = skiptok(&act_ptr, sep)) != NULL) {
      return token;
    }

  get_line:

    do {
      /* is it the end of the file? - shouldn't happen */
      if (feof(env_field.in_fp)) {
        if (env_field.comm_type == COMM_FILES)
          fclose(env_field.in_fp);
        return NULL;
      }

      if (verbose)
        fprintf(stderr, "Field - trying to read a line from the file.\n");

      /* read another line from the file */
      if (fgets(line, LLEN, env_field.in_fp) == NULL)
        return NULL;

      if (verbose)
        fprintf(stderr, "Field - line read.\n");

      act_ptr = line;
    } while ((token = skiptok(&act_ptr, sep)) == NULL);

    return token;
  }

  return NULL;
}

/****************************************************************************/
char NextFileModule(char **str, module_type *module) {
  char *ptr = (*str)++; /* local for *str */
  char *end, *ptr2;

  /* get the current symbol and set up the basic token structure */
  module->symbol = *(ptr++);
  module->num_params = 0;

  /* extract the parameters if present */
  if (*(ptr++) == '(') {
    if ((end = strchr(ptr, ')')) == NULL) {
      fprintf(stderr, "Field - warning: Unmatched parenthesis.\n");
      return 0;
    }
    for (;;) {
      ptr2 = strchr(ptr, ',');

      if (module->num_params >= MAX_PARAM) {
        fprintf(stderr, "Field - warning: Too many parameters in module.\n");
        return 0;
      }

      if ((ptr2 == NULL) || (ptr2 > end))
        break;

      *ptr2 = 0;
      module->params[module->num_params++] = atof(ptr);
      ptr = ptr2 + 1;
    }

    *end = 0;
    module->params[module->num_params++] = atof(ptr);

    *str = end + 1; /* update *str */
  }

  return (module->symbol);
}

/****************************************************************************/
/* returns 0 if no data avaliable */

int GetData(int fl) {
  int i;
  char *ptr, *token;
  extern int current_step;

  switch (fl) {
  case GET_BEGIN:
    SendData(SEND_BEGIN);
    env_field.in_num = 0;
    break;

  case GET_DATA:
  get_data:
    if ((token = LoadOneToken("\n")) == NULL)
      return 0;

    if (verbose)
      fprintf(stderr, "Field - recieved token: %s\n", token);

    if (strncmp(token, "Control:", 8) == 0) {
      current_step = 0;
      sscanf(token + 8, "%d %d", &env_field.in_flag, &current_step);

      if ((env_field.in_flag & (LAST_CHUNK | PROCESS_EXIT)) != 0) {
        if (verbose)
          fprintf(stderr, "Field - the end of the last incoming chunk.\n");
        return 0; /* return 0, if this is the end of the last chunk */
      }

      GetData(GET_END);
      SendData(SEND_END); /* out_flag is set to LAST_CHUNK */

      GetData(GET_BEGIN);
      goto get_data;
    }

    if (sscanf(token, "%lu", &dist) != 1) {
      fprintf(stderr, "Field process - index of symbol E is missing!\n");
      return 0;
    }

    /* token contains ?E with the following symbol */
    if ((ptr = strchr(token, 'E')) == NULL) {
      fprintf(stderr, "Field process - string doesn't contain symbol E!\n");
      return 0;
    }

    NextFileModule(&ptr, &comm_symbol);
    NextFileModule(&ptr, &next_symbol);

    /*  read in the turtle */
    if (tu_par.position > 0) {
      if ((token = LoadOneToken("\n")) == NULL)
        return 0;
      for (i = 0; i < tu_skip.position; i++)
        token++;

      sscanf(token, "%f %f %f", &loc_turtle.position[0],
             &loc_turtle.position[1], &loc_turtle.position[2], );
    }

    if (tu_par.heading > 0) {
      if ((token = LoadOneToken("\n")) == NULL)
        return 0;
      for (i = 0; i < tu_skip.heading; i++)
        token++;

      sscanf(token, "%f %f %f", &loc_turtle.heading[0], &loc_turtle.heading[1],
             &loc_turtle.heading[2], );
    }

    if (tu_par.left > 0) {
      if ((token = LoadOneToken("\n")) == NULL)
        return 0;
      for (i = 0; i < tu_skip.left; i++)
        token++;

      sscanf(token, "%f %f %f", &loc_turtle.left[0], &loc_turtle.left[1],
             &loc_turtle.left[2], );
    }

    if (tu_par.up > 0) {
      if ((token = LoadOneToken("\n")) == NULL)
        return 0;
      for (i = 0; i < tu_skip.up; i++)
        token++;

      sscanf(token, "%f %f %f", &loc_turtle.up[0], &loc_turtle.up[1],
             &loc_turtle.up[2], );
    }

    if (tu_par.line_width > 0) {
      if ((token = LoadOneToken("\n")) == NULL)
        return 0;
      for (i = 0; i < tu_skip.line_width; i++)
        token++;

      sscanf(token, "%f", &loc_turtle.line_width);
    }

    if (tu_par.scale_factor > 0) {
      if ((token = LoadOneToken("\n")) == NULL)
        return 0;
      for (i = 0; i < tu_skip.scale_factor; i++)
        token++;

      sscanf(token, "%f", &loc_turtle.scale_factor);
    }

    /* setting of tu must be done here! */
    tu = &loc_turtle;
    break;

  case GET_END:
    if (env_field.comm_type == COMM_FILES)
      fclose(env_field.in_fp);
    if (verbose)
      fprintf(stderr, "Field - GetData(GET_END).\n");

    break;
  }

  return 1;
}

/****************************************************************************/
int SaveOneItem(char *string, char test) {
  int len;

  switch (env_field.comm_type) {
  case COMM_MEMORY:
    len = strlen(string);

    if (test && (env_field.out_end + len + 25 >= FROM_FIELD_LENGTH - 1))
      /* those 25 are for the last row */
      /* not enough room left */
      return 0;
    else {
      /* add to the end */
      strcpy(env_field.shmadd->from_field + env_field.out_end, string);
      env_field.out_end += len;
    }
    break;

  case COMM_FILES:
  case COMM_PIPES:
    /* prevent too big file */
    if (test && (env_field.out_num > MAX_QUERIES_IN_FILE))
      return 0;

    if (verbose)
      fprintf(stderr, "Field - saving one item into a file/pipe.\n");

    if (fputs(string, env_field.out_fp) == EOF)
      return 0;

    if (verbose)
      fprintf(stderr, "Field - the item saved.\n");

    break;
  }
  return 1;
}

/****************************************************************************/
void SendData(int flag) {
  int i;
  char buff[1048];
  char item[5000];
  char filename[1024];
  struct sembuf sops;

  switch (flag) {
  case SEND_BEGIN:
    switch (env_field.comm_type) {
    case COMM_MEMORY:
    case COMM_FILES:
      /* wait on semaphore 0 */
      if (verbose)
        fprintf(stderr,
                "Field - waiting on semaphore 0 (in GetData(GET_BEGIN)).\n");

      sops.sem_num = 0;
      sops.sem_op = -1;
      sops.sem_flg = 0;
      semop(env_field.semid, &sops, 1);
      break;
    }

    env_field.out_flag = FIRST_CHUNK | LAST_CHUNK;
    env_field.out_num = 0;

    switch (env_field.comm_type) {
    case COMM_MEMORY:
      /* initialize the shared memory */
      env_field.out_end = 0;
      break;

    case COMM_FILES:
      /* open the data file */
      strcpy(filename, FILENAME_FROM_FIELD);
      if (env_field.extension != NULL)
        strcat(filename, env_field.extension);

      if ((env_field.out_fp = fopen(filename, "w")) == NULL)
        fprintf(stderr, "Error - field process cannot open data file %s.\n",
                filename);
      break;
    }

    if (verbose)
      fprintf(stderr, "Field - SendData(SEND_BEGIN).\n");
    break;

  case SEND_DATA:
    if (comm_symbol.num_params > 0) {
      env_field.out_num++;

      sprintf(item, "%lu E(", dist);

      for (i = 0; i < comm_symbol.num_params;) {
        sprintf(buff, "%g", comm_symbol.params[i]);
        strcat(item, buff);

        if (++i < comm_symbol.num_params)
          strcat(item, ",");
      }
      strcat(item, ")\n");

      /* save item */
      if (SaveOneItem(item, 1) == 0) {
        /* saving failed */
        env_field.out_flag &= ~LAST_CHUNK; /* reset LAST_CHUNK */
        SendData(SEND_END);

        SendData(SEND_BEGIN);
        env_field.out_flag &= ~FIRST_CHUNK; /* reset FIRST_CHUNK */

        SaveOneItem(item, 1);
        env_field.out_num++;
      }
    }
    break;

  case SEND_END:
    sprintf(buff, "Control: %d\n", env_field.out_flag);
    SaveOneItem(buff, 0);

    if (verbose)
      fprintf(stderr, "Field - Just saved control line.\n");

    switch (env_field.comm_type) {
    case COMM_FILES:
      if (env_field.out_fp != NULL) {
        fclose(env_field.out_fp);
      }
      break;
    case COMM_PIPES:
      fflush(env_field.out_fp);
      break;
    }

    switch (env_field.comm_type) {
    case COMM_MEMORY:
    case COMM_FILES:
      /* raise semaphore 1 */
      sops.sem_num = 1;
      sops.sem_op = 1;
      sops.sem_flg = 0;
      semop(env_field.semid, &sops, 1);

      if (verbose)
        fprintf(stderr,
                "Field - semaphore 1 raised (in SendData(SEND_END)).\n");
      break;
    }
    if (verbose)
      fprintf(stderr, "Field - SendData(SEND_END).\n");

    break;
  }
}

/****************************************************************************/
void MainLoop(void) {
  char c;

  for (;;) {
    GetData(GET_BEGIN);
    /* process the data */
    ProcessData();
    GetData(GET_END);
    SendData(SEND_END);

    if (env_field.in_flag & PROCESS_EXIT)
      break;
  }

  if (env_field.comm_type == COMM_PIPES) {
    /* confirm exit */
    c = 1;
    fwrite(&c, 1, 1, env_field.out_fp);
    fflush(env_field.out_fp);
  }
}

/****************************************************************************/
void main(int argc, char **argv) {
  char *process_name = argv[0];
  char c;

  /* default values */
  env_filename = NULL;
  env_field.semaphore_key = 0;
  env_field.extension = NULL;

  /* get switches */
  while ((argc > 1) && (argv[1][0] == '-'))
    switch (argv[1][1]) {
    case 'e':
      if (strncmp(&argv[1][1], "ext", 3) == 0)
        /* communication file extension */
        env_field.extension = strdup(argv[2]);
      else
        /* environmental file */
        env_filename = argv[2];

      argc -= 2;
      argv += 2;
      break;
    case 'k': /* semaphore key */
      env_field.semaphore_key = atoi(argv[2]);
      argc -= 2;
      argv += 2;
      break;
    default:
      goto out;
    }

out:
  if (env_field.semaphore_key == 0) {
    /* pipes */

    /* confirm execution by sending 1 to stdout as fast as possible */
    c = 1;
    fwrite(&c, 1, 1, stdout);
    fflush(stdout);

    if (verbose)
      fprintf(stderr, "Field - successful execution confirmed.\n");
  } else
    InitializeSemaphores(); /* set semaphores as fast as possible */

  InitializeCommunication();

  ProcessArguments(argc, argv);

  fprintf(stderr, "Field process %s initialized.\n", process_name);

  MainLoop();

  FreeCommunicationStructures();

  FreeFieldStructures();

  fprintf(stderr, "Field process %s exiting.\n", process_name);
}
