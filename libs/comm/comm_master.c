/*
  Functions handling communication with the field process.
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

#include <unistd.h>

#include <errno.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#include <fcntl.h>

#endif

#include "comm_lib.h" /* mainly outside visible prototypes */
#include "communication.h"
#include "compression.h"

#include "message.h"

static field_type env_field[CMAXFIELDS];

static int num_fields;

static char first_clear = 1; /* must be 1 */

/* local prototypes */
static int BeginTransmission(int index);
static int EndTransmission(int index, int current_step);
static int BeginTransmissionIn(int index);
static int EndTransmissionIn(int index);
static int Terminate(int index);

static int LetSlaveRunAndWait(int index);

#ifdef WIN32
#define getpid GetCurrentProcessId
#define sleep(s) Sleep(s * 1000)
#endif

// union semun test;

/****************************************************************************/
void CMFreeStructures(void) {
  int index;

  for (index = 0; index < num_fields; index++) {
    /* send EXIT to all field processes */
    Terminate(index);

    if (env_field[index].executable != NULL) {
      free(env_field[index].executable);
      env_field[index].executable = NULL;
    }

    if (env_field[index].comm_spec_file != NULL) {
      free(env_field[index].comm_spec_file);
      env_field[index].comm_spec_file = NULL;
    }

    if (env_field[index].host != NULL) {
      free(env_field[index].host);
      env_field[index].host = NULL;
    }

    if (env_field[index].formats.position != NULL) {
      free(env_field[index].formats.position);
      env_field[index].formats.position = NULL;
    }
    if (env_field[index].formats.heading != NULL) {
      free(env_field[index].formats.heading);
      env_field[index].formats.heading = NULL;
    }
    if (env_field[index].formats.left != NULL) {
      free(env_field[index].formats.left);
      env_field[index].formats.left = NULL;
    }
    if (env_field[index].formats.up != NULL) {
      free(env_field[index].formats.up);
      env_field[index].formats.up = NULL;
    }
    if (env_field[index].formats.line_width != NULL) {
      free(env_field[index].formats.line_width);
      env_field[index].formats.line_width = NULL;
    }
    if (env_field[index].formats.scale_factor != NULL) {
      free(env_field[index].formats.scale_factor);
      env_field[index].formats.scale_factor = NULL;
    }

    ReleaseSharedMemory(env_field + index);

    /* release semaphores */
    ReleaseSemaphores(env_field + index);

    switch (env_field[index].comm_type) {
    case COMM_FILES:
      unlink(env_field[index].in_name);
      unlink(env_field[index].out_name);
      break;

    case COMM_PIPES:
    case COMM_SOCKETS:
      if (env_field[index].in_fp != NULL)
        fclose(env_field[index].in_fp);
      env_field[index].in_fp = NULL;

      if (env_field[index].out_fp != NULL)
        fclose(env_field[index].out_fp);
      env_field[index].out_fp = NULL;
      break;
    }

    env_field[index].specified = 0;
  }
}

/****************************************************************************/
int CMGetNumberOfSlaves(void) { return num_fields; }

/****************************************************************************/
static int InitializeSemaphoreCommunication(int index, int key) {
  char *str, *str2, buff[1024], efile[1024], file_extension[30];
#ifdef WIN32
  BOOL res;
#endif

#ifndef WIN32
  if (env_field[index].verbose)
    Message("%s - initializing semaphores.\n", process_name);
#endif

  /* get a unique key */
  env_field[index].unique_key = key;

  if (!InitializeSemaphores(env_field + index))
    return 0;

  switch (env_field[index].comm_type) {
  case COMM_MEMORY:
    /* initialize shared memory */
    if (env_field[index].verbose)
      Message("%s - initializing shared memory.\n", process_name);
    if (!InitializeSharedMemory(env_field + index))
      return 0;
    break;

  case COMM_FILES:
    strcpy(buff, FILENAME_TO_FIELD);
    sprintf(file_extension, "%d.%d", getpid(), index);
    strcat(buff, file_extension);
    env_field[index].out_name = strdup(buff);

    strcpy(buff, FILENAME_FROM_FIELD);
    strcat(buff, file_extension);
    env_field[index].in_name = strdup(buff);
    break;
  }

  /* add the -k switch */
  str = strtok(env_field[index].executable, " \n");
  str2 = strtok(NULL, "\n");

  if (strncmp(str2, "-e", 2)) {
    /* switch -e missing -> get it from comm_spec_file */
    strcpy(efile, " -e ");
    strcat(efile, env_field[index].comm_spec_file);
    strcat(efile, " ");
  } else
    strcpy(efile, " "); /* must be at least one space */

  if (env_field[index].comm_type == COMM_MEMORY)
    sprintf(buff, "%s%s-k %ld %s", str, efile,
            (long int)env_field[index].unique_key, str2);
  else
    sprintf(buff, "%s%s-k %ld -ext %s %s", str, efile,
            (long int)env_field[index].unique_key, file_extension, str2);

  free(env_field[index].executable);
  env_field[index].executable = strdup(buff);

  if (env_field[index].verbose)
    Message("%s - spawning the field process: %s\n", process_name,
            env_field[index].executable);

    /* spawn the field process */
#ifndef WIN32
  system(env_field[index].executable);
#else

  {

    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    {
      si.cb = sizeof(STARTUPINFO);
      si.lpReserved = NULL;
      si.lpDesktop = NULL;
      si.lpTitle = NULL;
      si.dwX = si.dwY = 0;
      si.dwXSize = si.dwYSize = 0;
      si.dwXCountChars = si.dwYCountChars = 0;
      si.dwFillAttribute = 0;
      si.dwFlags = STARTF_USESHOWWINDOW;
      si.wShowWindow = SW_SHOWMINNOACTIVE;
      si.cbReserved2 = 0;
      si.lpReserved2 = NULL;
      si.hStdInput = si.hStdOutput = si.hStdError = NULL;
    }

    res = CreateProcess(NULL, env_field[index].executable, NULL, NULL, TRUE, 0,
                        NULL, NULL, &si, &pi);
  }

#endif

  /* give it some time to start */
  sleep(1);

#ifdef WIN32
  return (res != 0);
#else
  /* check if started - semaphore semval should be 0 */
#ifdef VLAB_LINUX
  return semctl(env_field[index].semid, 1, GETVAL, 0) == 0;
#else
#ifdef VLAB_MACX
  //union semun empty=0; /* PASCAL, the next initialization semaphore was using a variable
		       // "empty" for initialization, I don't thik it's correct neither necessary */
  return semctl(env_field[index].semid, 1, GETVAL, 0) == 0;
#else
  return semctl(env_field[index].semid, 1, GETVAL) == 0;
#endif /* VLAB_MACX */
#endif /* VLAB_LINUX */
#endif /* WIN32 */
}

/****************************************************************************/

#ifndef WIN32
static char *GetPath(char *path) {
  static char cmd[1024];

  if (path[0] != '/') {
    FILE *fp;

    strcpy(cmd, "which ");
    strcat(cmd, path);

    /* try to get the path */
    if ((fp = popen(cmd, "r")) != NULL) {
      fgets(cmd, sizeof(cmd), fp);

      if (cmd[strlen(cmd) - 1] == '\n')
        cmd[strlen(cmd) - 1] = 0;

      if (cmd[0] != path[0]) {
        /* path found */
        path = cmd;
      }
    }
  }
  return path;
}
#endif

/****************************************************************************/

static int InitializePipeCommunication(int index) {
#ifdef WIN32
  assert(!"Pipes not implemented under Win32");
  return 0;
#else
  int fd1[2], fd2[2]; /* file descriptors for two pies */
  char *token, c, *path;
  int ind;
  char *(argv[256]);

  if (pipe(fd1) || pipe(fd2)) {
    Message("%s - cannot open pipes!\n", process_name);
    return 0;
  }

  if (fork() != 0) {
    /* parent - comm. master */
    env_field[index].in_fp = fdopen(dup(fd1[0]), "r");
    env_field[index].out_fp = fdopen(dup(fd2[1]), "w");

    if ((env_field[index].in_fp == NULL) || (env_field[index].out_fp == NULL)) {
      Message("%s - cannot get stream for the pipes!", process_name);
      return 0;
    }

    /* close the unused ones */
    close(fd1[1]); /* pipe1 output */
    close(fd2[0]); /* pipe2 input  */
    close(fd1[0]);
    close(fd2[1]);

    /* check whether the child is running OK */
    if (env_field[index].verbose)
      Message("%s - checking whether environmental process is running.\n",
              process_name);

    c = 0;
    fread(&c, 1, 1, env_field[index].in_fp);

    if (env_field[index].verbose) {
      if (c == 1)
        Message("%s - environmental process is up and running.\n",
                process_name);
      else
        Message("%s - environmental process is NOT running!\n", process_name);
    }

    return c == 1;
  }

  /* child */
  if (dup2(fd1[1], 1) == -1) {
    /* to stdout */
    Message("comm_servant - cannot duplicate into stdout.");
    goto send_zero;
  }

  close(fd1[0]);
  close(fd1[1]);

  if (dup2(fd2[0], 0) == -1) {
    /* to stdin */
    Message("comm_servant - cannot duplicate into stdin.");
    goto send_zero;
  }

  close(fd2[0]);
  close(fd2[1]);

  /* get the env. process arguments */
  token = strtok(env_field[index].executable, " \n\t");
  ind = 0;

  do {
    if (ind == 1) {
      if (strncmp(token, "-e", 2)) {
        char temp[5];

        /* switch -e missing -> get it from comm_spec_file */
        strcpy(temp, "-e");
        argv[ind++] = strdup(temp);

        argv[ind++] = strdup(env_field[index].comm_spec_file);
      }
    }

    argv[ind++] = strdup(token);
  } while ((token = strtok(NULL, " \n\t")) != NULL);

  if (argv[ind - 1][0] == '&')
    ind--;

  argv[ind] = NULL;

  path = GetPath(argv[0]);

  /* spawn the environmental process */
  execv(path, argv);

  /* an error occured */
  Message("comm_servant - process %s cannot be spawned!\n", path);

send_zero:
  /* pipe 0 back */
  c = 0;
  fwrite(&c, 1, 1, stdout);
  fflush(stdout);
  exit(-1);
  return -1;
#endif /* WIN32 */
}

/****************************************************************************/

static int InitializeSocketCommunication(int index, int key) {
#ifdef WIN32
  assert(!"Sockets not implemented under Win32");
  return 0;
#else
  int *s, t1, t2;
  char *str, *str2, c, *path;
  char buff[1024];
  char so1, so2;
  char hostname[1024];
  struct timeval time;
  fd_set fd;
  extern int confirm_socket;

  /* get a unique key */
  env_field[index].unique_key = key;

  gethostname(hostname, sizeof(hostname)); /* who are we? */

  if ((s = EstablishSocket(env_field + index, hostname)) == NULL)
    return 0; /* have to return 0 - sockets not set up */

  if (env_field[index].executable != NULL) {
    /* add the -k switch */
    str = strtok(env_field[index].executable, " \n\t");
    str2 = strtok(NULL, "\n");

    path = GetPath(str);

    if (env_field[index].host != NULL)
      sprintf(buff, "rsh %s %s -k %ld -host %s %s", env_field[index].host, path,
              (long int)env_field[index].unique_key, hostname, str2);
    else
      sprintf(buff, "%s -k %ld -host %s %s", path,
              (long int)env_field[index].unique_key, hostname, str2);

    free(env_field[index].executable);
    env_field[index].executable = strdup(buff);

    if (env_field[index].verbose)
      Message("%s - spawning the field process: %s\n", process_name,
              env_field[index].executable);

    /* spawn the field process */
    system(env_field[index].executable);

    if (env_field[index].verbose)
      Message("%s - waiting for a call on socket.\n", process_name);

    sleep(1);
  }

  if (env_field[index].verbose)
    Message("%s - waiting for both sockets.\n", process_name);

  so1 = 0;
  so2 = 0;

  for (;;) {
    FD_ZERO(&fd);

    if (confirm_socket != -1)
      /* check the confirm_socket */
      FD_SET(confirm_socket, &fd);

    if (!so1)
      /* first socket */
      FD_SET(s[0], &fd);

    if (!so2)
      /* second socket */
      FD_SET(s[1], &fd);

    /* wait 60s  */
    time.tv_sec = 60;
    time.tv_usec = 0;

    if (select(getdtablesize(), &fd, NULL, NULL, &time) > 0) {

      if (confirm_socket != -1)
        if (FD_ISSET(confirm_socket, &fd)) {
          if (read(confirm_socket, &c, 1) != 1)
            Message("The symbol not read!\n");

          if (c != 'K')
            Message("%s - Symbol %c not expected on the confirm_socket.\n",
                    process_name, c);

          Message("%s - terminated.\n", process_name);
          return 0;
        }

      if (FD_ISSET(s[0], &fd)) {
        if (env_field[index].verbose)
          Message("%s - response on socket 1.\n", process_name);

        if ((t1 = accept(s[0], (struct sockaddr *)NULL, (socklen_t *)NULL)) <
            0) {
          Message("%s - error connecting the socket 1!\n", process_name);
          return 0;
        }
        so1 = 1;
        if (so2)
          break;
      }

      if (FD_ISSET(s[1], &fd)) {
        if ((t2 = accept(s[1], (struct sockaddr *)NULL, (socklen_t *)NULL)) <
            0) {
          Message("%s - error connecting the socket 2!\n", process_name);
          return 0;
        }
        so2 = 1;
        if (so1)
          break;
      }
    } else {
      Message("%s - timeout. Slave not running yet!\n", process_name);
      return 0;
    }
  }

  close(s[0]);
  close(s[1]);

  env_field[index].in_fp = fdopen(t1, "r");
  env_field[index].out_fp = fdopen(t2, "w");

  if (env_field[index].executable != NULL) {
    fread(&c, 1, 1, env_field[index].in_fp);

    if (env_field[index].verbose) {
      if (c == 1)
        Message("%s - environmental process is up and running.\n",
                process_name);
      else
        Message("%s - environmental process is NOT running!\n", process_name);
    }
    return c == 1;
  }

  return 1;
#endif /* WIN32 */
}

/****************************************************************************/
int CMInitialize(void) {
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
    CMFreeStructures();

  num_fields = 0;

  return 1;
}

/****************************************************************************/
/* Parameter - name of the environment file. Returns the index of the field
   or -1 when not successful.
 */
int CMAddProcess(char *filename) {
  int i;
  int index, key;

  /* no file no fun */
  if (filename == NULL) {
    Message("%s - cannot add a process, no .e file specified!\n", process_name);
    return -1;
  }

  if (process_name[0] == 0) /* so it doesn't overwrites the proper name
          if also connection to a master used */
    strcpy(process_name, "comm_master");

  if (num_fields >= CMAXFIELDS) {
    Message("%s - max. number of fields reached, ignored!\n", process_name);
    return -1;
  }

  index = num_fields;

  env_field[index].specified = MASTER;
  env_field[index].expect_data_in = 0;
#ifdef XXX
  env_field[index].SetCommModulePars = SetCommModulePars;
#endif

  if (!ReadInEnvironmentalFile(env_field + index, filename))
    return -1;

  env_field[index].comm_spec_file = strdup(filename);

  /* check if an executable specified */
  if (env_field[index].executable == NULL) {
    Message("%s - no executable for field process specified. "
            "Environment ignored.\n",
            process_name);
    return -1;
  }

  key =
#ifdef WIN32
      (unsigned short)
#else
      (u_short)
#endif
          ((getpid() << 2) + index * 4);

  switch (env_field[index].comm_type) {
  case COMM_MEMORY:
  case COMM_FILES:
    i = InitializeSemaphoreCommunication(index, key);
    break;

  case COMM_PIPES:
    i = InitializePipeCommunication(index);
    break;

  case COMM_SOCKETS:
    i = InitializeSocketCommunication(index, key);
    break;

  default:
    Message("%s - unknown mean of communication! Pipes used.\n", process_name);
    i = InitializePipeCommunication(index);
  }

  if (!i) {
    Message("Field process %s is not started. Environment ignored.\n",
            process_name);
    CMFreeStructures();
    return -1;
  }

  if (env_field[index].verbose)
    Message("%s - environment file processed.\n", process_name);

  num_fields++;

  return index;
}
/****************************************************************************/
/* Parameter - name of the environment file and socket key. Called from
   CInitialize(). ONLY for socket communication.
   Returns the index of the field or -1 when not successful.
 */
int CMAddConnection(char *filename, int key) {
  int i;
  int index;

#ifdef WIN32
  assert(!"CMAddConnection should not be called under Win32");
#endif

  /* no file no fun */
  if (filename == NULL)
    return -1;

  if (num_fields >= CMAXFIELDS) {
    Message("%s - max. number of fields reached, ignored!\n", process_name);
    return -1;
  }

  index = num_fields;

  env_field[index].specified = MASTER;
  env_field[index].expect_data_in = 0;
#ifdef XXX
  env_field[index].SetCommModulePars = SetCommModulePars;
#endif

  if (!ReadInEnvironmentalFile(env_field + index, filename))
    return -1;

  /* for now, force socket communication - cannot be debugged, but
prevents crashes when the other types are used by accident */
  env_field[index].comm_type = COMM_SOCKETS;

  env_field[index].executable = NULL; /* process will be started by another
  program */
  i = InitializeSocketCommunication(index, key);

  if (!i) {
    Message("Field process %s is not started. Environment ignored.\n",
            process_name);
    return -1;
  }

  if (env_field[index].verbose)
    Message("%s - environment file processed.\n", process_name);

  num_fields++;

  return index;
}

/****************************************************************************/
/* returns 0 when OK. Negative length means that item is a string. */
static int SendDataItem(int index, char *item, int length) {
  /* test whether enough room first */
  if (SaveOneDataItem(env_field + index, item, length, 1) == 0) {
    /* saving failed */
    env_field[index].out_flag &= ~LAST_CHUNK; /* reset LAST_CHUNK */
    env_field[index].data_out = 0;
    if (!EndTransmission(index, 0))
      Message("%s - cannot end transmission for the chunk. "
              "expect trouble.\n",
              process_name);

    env_field[index].data_out = 1;

    if (!BeginTransmission(index))
      Message("%s - cannot begin transmission for the next chunk. "
              "expect trouble.\n",
              process_name);

    env_field[index].out_flag &= ~FIRST_CHUNK; /* reset FIRST_CHUNK */

    SaveOneDataItem(env_field + index, item, length, 1);
    env_field[index].out_num++;
  }

  return 0;
}

/****************************************************************************/
/* returns 0 when OK */
int CMSendString(int index, char *item) {
  if ((index < 0) || (index >= num_fields))
    return 1;
  if (env_field[index].specified == 0)
    return 1;

  /* cannot output string when it is not expected */
  if (!env_field[index].data_out && !env_field[index].strings_only) {
    Message("%s - cannot send string. Not expected!\n", process_name);
    return 1;
  }

  return SendDataItem(index, item, -1);
}

/****************************************************************************/
/* returns 0 when OK */
int CMSendBinaryData(int index, char *item, int item_size, int nitems) {
  char len[4]; // [PASCAL] do we need to unsigned this char[] ?
  int length, ret;
  char *buff;

  if ((index < 0) || (index >= num_fields))
    return 1;
  if (env_field[index].specified == 0)
    return 1;

  /* cannot output binary data when it is not expected */
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
/* string - pointer to the original string
   distance - distance from the string beginning

   returns 1 when the next symbol's graphics is required
   */
int SaveCommunicationItem(int index, unsigned long distance,
                          Cmodule_type *two_modules, CTURTLE *turtle) {
  int i, it;
  unsigned char flags;
  char buff[1024];
  char item[5024];

  if (two_modules == NULL)
    return 0;

  if (env_field[index].data_out) {
    env_field[index].data_out = 0;

    strcpy(item, DATA_END);
    strcat(item, "\n");
  } else
    item[0] = '\0';

  sprintf(buff, "%lu ", distance);
  strcat(item, buff);

  /* save the part of the string in the data file */

  for (it = 0; it < 1 + env_field[index].following_module; it++) {
    if (two_modules[it].symbol[0] != '\0') {
      sprintf(buff, "%s", two_modules[it].symbol);
      strcat(item, buff);

      if (two_modules[it].num_params > 0) {
        strcat(item, "(");
        for (i = 0; i < two_modules[it].num_params; i++) {
          sprintf(buff, "%g", two_modules[it].params[i].value);
          strcat(item, buff);
          if (i != two_modules[it].num_params - 1)
            strcat(item, ",");
        }
        strcat(item, ")");
      }

      if (it == 1) {
        flags =
            env_field[index]
                .interpreted_modules[(unsigned char)two_modules[1].symbol[0]];

        if ((flags & 0x80) || (flags & (0x40 >> two_modules[1].num_params)))
          /* output the symbol */
          env_field[index].data_out = 1;
      }
    }
  }
  strcat(item, "\n");

  /* output the turtle */
  if (env_field[index].formats.position != NULL) {
    sprintf(buff, env_field[index].formats.position, turtle->position[0],
            turtle->position[1], turtle->position[2]);
    strcat(buff, "\n");
    strcat(item, buff);
  }
  if (env_field[index].formats.heading != NULL) {
    sprintf(buff, env_field[index].formats.heading, turtle->heading[0],
            turtle->heading[1], turtle->heading[2]);
    strcat(buff, "\n");
    strcat(item, buff);
  }
  if (env_field[index].formats.left != NULL) {
    sprintf(buff, env_field[index].formats.left, turtle->left[0],
            turtle->left[1], turtle->left[2]);
    strcat(buff, "\n");
    strcat(item, buff);
  }
  if (env_field[index].formats.up != NULL) {
    sprintf(buff, env_field[index].formats.up, turtle->up[0], turtle->up[1],
            turtle->up[2]);
    strcat(buff, "\n");
    strcat(item, buff);
  }
  if (env_field[index].formats.line_width != NULL) {
    sprintf(buff, env_field[index].formats.line_width, turtle->line_width);
    strcat(buff, "\n");
    strcat(item, buff);
  }
  if (env_field[index].formats.scale_factor != NULL) {
    sprintf(buff, env_field[index].formats.scale_factor, turtle->scale_factor);
    strcat(buff, "\n");
    strcat(item, buff);
  }

  if (env_field[index].data_out) {
    strcat(item, DATA_BEGIN);
    strcat(item, "\n");
  }

  /* expect graphics must be set to 1 otherwise the string won't be sent */
  i = env_field[index].data_out;
  env_field[index].data_out = 1;
  CMSendString(index, item);
  env_field[index].data_out = i;

  return env_field[index].data_out;
}

/****************************************************************************/
static int LoadCommunicationItem(int index, unsigned long *dist,
                                 Cmodule_type *comm_module) {
  char *str, *ptr, *end, *token;
  int num;

  /* get the distance from the begining of the string */
  if ((token = LoadOneToken(env_field + index, " ,;:\n")) == NULL)
    return 0;

  if (sscanf(token, "%lu", dist) != 1) {
    Message("%s - token '%s' doesn't contain index of ?E!\n", process_name,
            token);
    return 0;
  }

  if ((token = LoadOneToken(env_field + index, "\n")) == NULL) {
    Message("%s - symbol E expected in from the environment.\n", process_name);
    return 0;
  }

  if ((str = strchr(token, 'E')) == NULL) {
    Message("%s - symbol E expected in from the environment.\n", process_name);
    return 0;
  }

  if (env_field[index].verbose)
    Message("%s - received token from slave: %s\n", process_name, token);

  num = -1;

  str++;
  if (*(str++) == '(') {
    if ((end = strchr(str, ')')) == NULL) {
      Message("%s - closing bracket of ?E not found\n", process_name);
      return 0;
    }

    while ((ptr = strchr(str, ',')) != NULL) {
      if (*str == ',') {
        /* no parameter */
        comm_module->params[++num].set = 0;
      } else {
        *ptr = '\0';

        comm_module->params[++num].value = (float)atof(str);
        comm_module->params[num].set = 1;
      }
      str = ptr + 1;
    }

    if (*str == ')') {
      /* no parameter */
      comm_module->params[++num].set = 0;
    } else {
      *end = '\0';
      comm_module->params[++num].value = (float)atof(str);
      comm_module->params[num].set = 1;
    }
  }

  comm_module->num_params = num + 1;

  return 1;
}

/****************************************************************************/
static int LetSlaveRunAndWait(int index) {
#ifndef WIN32
  struct sembuf sops;
#endif

  EndTransmissionIn(index); /* New */
  BeginTransmissionIn(index);

  switch (env_field[index].comm_type) {
  case COMM_MEMORY:
  case COMM_FILES:
    /* raise semaphore 0 */
#ifdef WIN32
    ReleaseSemaphore(env_field[index].semid.hSem0, 1, NULL);
#else
    sops.sem_num = 0;
    sops.sem_op = 1;
    sops.sem_flg = 0;
    semop(env_field[index].semid, &sops, 1); /* let the slave process run */
#endif

    if (env_field[index].verbose)
      Message("%s - communication - semaphore 0 raised\n", process_name);

    /* wait for the response - semaphore 1 */
    if (env_field[index].verbose)
      Message("%s - communication - waiting for semaphore 1\n", process_name);

#ifdef WIN32
    WaitForSingleObject(env_field[index].semid.hSem1, INFINITE);
#else
    sops.sem_num = 1;
    sops.sem_op = -1;
    sops.sem_flg = 0;
    semop(env_field[index].semid, &sops, 1);
#endif
    /* waiting for some input from the field process */
    break;

  case COMM_PIPES:
  case COMM_SOCKETS:
    /* nothing */
    break;
  }

  env_field[index].in_num = 0;
  return 1;
}


/****************************************************************************/
/* returns 0 when no more modules from the slave - at present */
int CMGetCommunicationModule(int index, unsigned long *module_id,
                             Cmodule_type *comm_module) {
  if (!env_field[index].expect_data_in)
    return 0;

  if (env_field[index].verbose)
    Message("%s - getting the module.\n", process_name);

  for (;;) {
    if (LoadCommunicationItem(index, module_id, comm_module))
      return 1;

    if ((env_field[index].in_flag & LAST_CHUNK) != 0) {
      /* end of file */
      env_field[index].expect_data_in = 0;
      return 0;
    }

    /* wait for the beginning of the next chunk */
    LetSlaveRunAndWait(index);
  }
}

/****************************************************************************/
/* returns 0 when DATA_END encountered or data are not expected. */
int CMGetString(int index, char *str, int length) {
  char *token;

  if (env_field[index].specified == 0) {
    Message("%s - slave number %d not specified!\n", process_name, index);
    return 0;
  }

  if (!env_field[index].expect_data_in) {
    Message("%s - input from a slave not expected now!\n", process_name);
    return 0;
  }

  if (!env_field[index].strings_only) {
    Message("%s - string input from a slave not expected now!\n", process_name);
    return 0;
  }

  for (;;) {
    if ((token = LoadOneToken(env_field + index, "\n")) != NULL) {
      if (!strcmp(token, DATA_END)) {
        return 0;
      }

      if (strlen(token) + 1 > (unsigned int)length)
        token[length - 1] = 0;

      strcpy(str, token);
      return 1;
    }

    if ((env_field[index].in_flag & LAST_CHUNK) != 0) {
      /* end of file */
      env_field[index].expect_data_in = 0;
      return 0;
    }

    /* wait for the beginning of the next chunk */
    LetSlaveRunAndWait(index);
  }
}

/****************************************************************************/
/* returns 0 when the end of input encountered. */
static int GetBinaryItem(int index, char *data, int length) {
  for (;;) {
    if (LoadOneDataItem(env_field + index, "\n", data, length) != NULL)
      return 1;

    if ((env_field[index].in_flag & LAST_CHUNK) != 0) {
      /* end of file */
      env_field[index].expect_data_in = 0;
      return 0;
    }

    /* wait for the beginning of the next chunk */
    LetSlaveRunAndWait(index);
  }
}

/****************************************************************************/
/* returns number of items read */
int CMGetBinaryData(int index, char *data, int item_size, int nitems) {
  char *buff, *ptr;
  char len[4]; // [PASCAL] do we need to unsigned this char[] ?
  int length;

  if (env_field[index].specified == 0) {
    Message("%s - slave number %d not specified!\n", process_name, index);
    return 0;
  }

  if (!env_field[index].expect_data_in) {
    Message("%s - input from a slave not expected now!\n", process_name);
    return 0;
  }

  if (!env_field[index].binary_data) {
    Message("%s - binary data input from a slave not expected now!\n",
            process_name);
    return 0;
  }

  if (!GetBinaryItem(index, len, 4))
    return 0;

  length = ((int)len[0]) + (((int)len[1]) << 8) + (((int)len[2]) << 8) +
           (((int)len[3]) << 24);

  if ((buff = (char *)malloc(length)) == NULL) {
    Message("%s - cannot allocate temporary buffer.\n", process_name);
    return 1;
  }

  ptr = buff;

  while (length > LENGTH_OF_BINARY_CHUNK) {
    if (!GetBinaryItem(index, buff, LENGTH_OF_BINARY_CHUNK))
      return 0;
    buff += LENGTH_OF_BINARY_CHUNK;
    length -= LENGTH_OF_BINARY_CHUNK;
  }

  if (!GetBinaryItem(index, buff, length))
    return 0;

  length = RleDecode(ptr, item_size, nitems, data);

  free(ptr);

  return length; /* the number of bytes read */
}

/****************************************************************************/
static int BeginTransmission(int index) {
  if ((index < 0) || (index >= num_fields))
    return 0;
  if (env_field[index].specified == 0)
    return 0;

  env_field[index].out_num = 0;
  env_field[index].out_flag = FIRST_CHUNK | LAST_CHUNK;

  if (env_field[index].verbose)
    Message("%s - begin transmission (index %d).\n", process_name, index);

  switch (env_field[index].comm_type) {
  case COMM_MEMORY:
    /* initialize the shared memory */
    env_field[index].shstringend = 0;
    break;

  case COMM_FILES:
    /* open the data file */
    if ((env_field[index].out_fp = fopen(env_field[index].out_name, "w")) ==
        NULL)
      Message("%s - cannot open communication file. Environment ignored.\n",
              process_name);
    break;

  case COMM_PIPES:
  case COMM_SOCKETS:
    /* nothing */
    break;
  }

  return 1;
}

/****************************************************************************/
int CMBeginTransmission(void) {
  int index;

  for (index = 0; index < num_fields; index++)
    BeginTransmission(index);

  return 1;
}

/****************************************************************************/
static int BeginTransmissionIn(int index) {
  if (env_field[index].specified == 0)
    return 1;

  env_field[index].in_num = 0;

  if (env_field[index].verbose)
    Message("%s - setting counter of modules from slave to 0.\n", process_name);

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

    if (env_field[index].verbose)
      Message("%s - closing input file\n", process_name);
  }

  return 1;
}

/****************************************************************************/
static int EndTransmission(int index, int current_step) {
  char buff[40], item[50];

  if ((index < 0) || (index >= num_fields))
    return 0;
  if (env_field[index].specified == 0)
    return 0;

  if (env_field[index].data_out) {
    env_field[index].data_out = 0;

    strcpy(item, DATA_END);
    strcat(item, "\n");
  } else
    item[0] = '\0';

  /* send the data */
  sprintf(buff, "%sControl: %d %d\n", item, env_field[index].out_flag,
          current_step);

  SaveOneItem(env_field + index, buff, 0);

  if (env_field[index].verbose)
    Message("%s - end transmission (index %d).\n", process_name, index);

  switch (env_field[index].comm_type) {
  case COMM_FILES:
    if (env_field[index].out_fp != NULL) {
      fclose(env_field[index].out_fp);
    }
    break;
  case COMM_PIPES:
  case COMM_SOCKETS:
    fflush(env_field[index].out_fp);
    break;
  }

  LetSlaveRunAndWait(index);
  env_field[index].expect_data_in = 1;
  return 1;
}

/****************************************************************************/
int CMEndTransmission(int current_step) {
  int index;

  for (index = 0; index < num_fields; index++)
    EndTransmission(index, current_step);

  return 1;
}

/****************************************************************************/
static int Terminate(int index) {
  char c;

  if ((index < 0) || (index >= num_fields))
    return 0;
  if (env_field[index].specified == 0)
    return 0;

  env_field[index].out_num = 0;
  env_field[index].out_flag = PROCESS_EXIT;

  switch (env_field[index].comm_type) {
  case COMM_MEMORY:
    env_field[index].shstringend = 0;
    break;

  case COMM_FILES:
    if ((env_field[index].out_fp = fopen(env_field[index].out_name, "w")) ==
        NULL) {
      Message("%s - cannot open communication file. "
              "Field process not killed.\n",
              process_name);
      return 0;
    }
    break;

  case COMM_PIPES:
  case COMM_SOCKETS:
    /* nothing */
    break;
  }

  EndTransmission(index, 0);

  switch (env_field[index].comm_type) {
  case COMM_MEMORY:
  case COMM_FILES:
    /* nothing */
    break;

  case COMM_SOCKETS:
  case COMM_PIPES:
    /* wait for the response - environment sends a character */
    fread(&c, 1, 1, env_field[index].in_fp);
    break;
  }

  return 1;
}

/****************************************************************************/
int CMTerminate(void) {
  int index;
  int ret = 1;

  for (index = 0; index < num_fields; index++)
    if (!Terminate(index))
      ret = 0;

  return ret;
}

/****************************************************************************/
/* string - pointer to the string ('(' of the communication symbol),
   len - length of the communication symbol and following symbol,
   */
int CMSendCommSymbol(int index, unsigned long distance,
                     Cmodule_type *two_modules, CTURTLE *turtle) {

  if ((index < 0) || (index >= num_fields)) {
    Message("%s - field index %d out of range!\n", process_name, index);
    return 0;
  }
  if (env_field[index].specified == 0) {
    Message("%s - the field with index %d not specified!\n", process_name,
            index);
    return 0;
  }
  if (env_field[index].strings_only) {
    Message("%s - the field with index %d allows to send only strings,"
            "not comm. modules!\n",
            process_name, index);
    return 0;
  }

  if (env_field[index].verbose)
    Message("%s - send comm. symbol (index %d).\n", process_name, index);

  env_field[index].out_num++;

  switch (env_field[index].comm_type) {
  case COMM_FILES:
  case COMM_PIPES:
  case COMM_SOCKETS:
    if (env_field[index].out_fp == NULL) {
      Message("%s - out file is closed!\n", process_name);
      return 1;
    }
    break;
  }

  return SaveCommunicationItem(index, distance, two_modules, turtle);
}
