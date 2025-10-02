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

#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#endif

#include "comm_lib.h"
#include "communication.h"

#include <stdarg.h>

static void Message(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
}

#define MAXHOSTNAME 1024

char process_name[256] = {0};

int confirm_socket = -1;
/* place where to confirm execution and send info about	termination */

/****************************************************************************/
int InitializeSharedMemory(field_type *env_field) {
#ifndef WIN32
  switch (env_field->specified) {
  case MASTER:
    /* release the shared memory */
    shmctl(env_field->shmid, IPC_RMID, NULL);

    /* set up the shared memory */
    if ((env_field->shmid =
             shmget(env_field->unique_key - 1, sizeof(shared_memory_type),
                    IPC_CREAT | 0600)) != -1)
      break;
    /* otherwise try without IPC_CREAT flag */
  case SLAVE:
    if ((env_field->shmid = shmget(env_field->unique_key - 1,
                                   sizeof(shared_memory_type), 0600)) == -1) {
      Message("%s - shared memory access failed!\n", process_name);
      return 0;
    }
  }

  /* bind the shared memory */
  if ((env_field->shmadd = (shared_memory_type *)(shmat(
           env_field->shmid, NULL, 0))) == (shared_memory_type *)-1) {
    Message("%s - error bindind shared memory address!\n", process_name);
    return 0;
  }

  return 1;

#else
  assert(!"Shared memory not implemented under Win32");
  return 0;
#endif
}

/****************************************************************************/
/* MAKE SURE THAT THIS PROCEDURE IS ALWAYS CALLED BEFORE EXIT!!!*/
void ReleaseSharedMemory(field_type *env_field) {
#ifndef WIN32
  if (env_field->shmadd != NULL) {
    /* unbind the shared memory */
    if (shmdt(env_field->shmadd) != 0) {
      Message("%s - error unbinding address\n", process_name);
      return;
    }
    env_field->shmadd = NULL;
  }

  if (env_field->specified == MASTER) {
    /* master releases the memory */
    if (env_field->shmid != -1) {
      /* release the shared memory */
      shmctl(env_field->shmid, IPC_RMID, NULL);
      env_field->shmid = -1;
    }
  }
#endif
}

/****************************************************************************/
int InitializeSemaphores(field_type *env_field) {
#ifndef WIN32
  union semun arg;
#else
  char bf[32];
  sprintf(bf, "a%d", env_field->unique_key);
#endif

  switch (env_field->specified) {
  case MASTER:
    /* get a pair of semaphores */
#ifdef WIN32
    env_field->semid.hSem0 = CreateSemaphore(NULL, 0, 1, bf);
    if (NULL == env_field->semid.hSem0) {
      Message("%s - cannot create semaphore\n", process_name);
      return 0;
    }

    bf[0] = 'b';
    env_field->semid.hSem1 = CreateSemaphore(NULL, 1, 1, bf);
    if (NULL == env_field->semid.hSem1) {
      CloseHandle(env_field->semid.hSem0);
      Message("%s - cannot create semaphore\n", process_name);
      return 0;
    }
#else
    if ((env_field->semid =
             semget(env_field->unique_key, 2, IPC_CREAT | 0600)) == -1) {
      Message("%s - cannot get semaphore id!\n", process_name);
      return 0;
    }

    /* set the semval to 0 */
    arg.val = 0;
    semctl(env_field->semid, 0, SETVAL, *(union semun *)&arg);
    arg.val = 1;
    semctl(env_field->semid, 1, SETVAL, *(union semun *)&arg);
#endif
    break;

  case SLAVE:
    /* get a pair of semaphores */
#ifdef WIN32
    env_field->semid.hSem0 = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, bf);
    if (NULL == env_field->semid.hSem0) {
      Message("%s - cannot get semaphore\n", process_name);
      return 0;
    }

    bf[0] = 'b';
    env_field->semid.hSem1 = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, bf);
    if (NULL == env_field->semid.hSem1) {
      CloseHandle(env_field->semid.hSem0);
      Message("%s - cannot get semaphore\n", process_name);
      return 0;
    }

    assert(NULL != env_field->semid.hSem1);
    WaitForSingleObject(env_field->semid.hSem1, 0);
#else
    if ((env_field->semid = semget(env_field->unique_key, 2, 0600)) == -1) {
      Message("%s - cannot get semaphore id!\n", process_name);
      return 0;
    }

    /* set the semval to 0 */
    arg.val = 0;
    semctl(env_field->semid, 1, SETVAL, arg);
#endif

    break;
  }

  return 1;
}

/****************************************************************************/
void ReleaseSemaphores(field_type *env_field) {
#ifdef WIN32
  if (NULL != env_field->semid.hSem0) {
    CloseHandle(env_field->semid.hSem0);
    env_field->semid.hSem0 = NULL;
  }
  if (NULL != env_field->semid.hSem1) {
    CloseHandle(env_field->semid.hSem1);
    env_field->semid.hSem1 = NULL;
  }
#else
  if (env_field->semid != -1) {
    /* release semaphores */
#ifdef VLAB_LINUX
    semctl(env_field->semid, 0, IPC_RMID, 0);
#else
#ifdef VLAB_MACX
  //union semun empty=0; /* PASCAL, the next initialization semaphore was using a variable
		       // "empty" for initialization, I don't thik it's correct neither necessary */
    semctl(env_field->semid, 0, IPC_RMID, 0);
#else
    semctl(env_field->semid, 0, IPC_RMID);
#endif /* VLAB_MACX */
#endif /* VLAB_LINUX */
    env_field->semid = -1;
  }
#endif /* WIN32 */
}

/****************************************************************************/
/* code to establish a socket; originally from bzs@bu-cs.bu.edu
   Returns the socket's fd or -1 when it fails. */
int *EstablishSocket(field_type *env_field,char *hostname) {
#ifndef WIN32
#ifndef VLAB_MACX
  char myname[MAXHOSTNAME + 1];
  static int s[2], count;
  struct sockaddr_in sa;
  struct hostent *hp;

  gethostname(myname, MAXHOSTNAME); /* who are we? */
  hp = gethostbyname(myname);       /* get our address info */

  if (hp == NULL) /* we don't exist !? */
    return NULL;

  if ((s[0] = socket(AF_INET, SOCK_STREAM, 0)) < 0) { /* create socket */
    Message("%s - cannot create socket 1.\n", process_name);
    return NULL;
  }

  memset(&sa, 0, sizeof(struct sockaddr_in)); /* clear our address */

  sa.sin_family = hp->h_addrtype; /* this is our host address */
  sa.sin_addr.s_addr = INADDR_ANY;
  sa.sin_port = htons((u_short)env_field->unique_key);
  /* this is our port number */

  /* Set the options of the socket */
  count = 1;
  setsockopt(s[0], SOL_SOCKET, SO_REUSEADDR, &count, sizeof count);

  if (bind(s[0], &sa, sizeof sa) < 0) { /* bind address to socket */
    /* try to close the socket and create it again */
    close(s[0]);

    memset(&sa, 0, sizeof(struct sockaddr_in)); /* clear our address */

    sa.sin_family = hp->h_addrtype; /* this is our host address */
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_port = htons((u_short)env_field->unique_key);
    /* this is our port number */

    if ((s[0] = socket(AF_INET, SOCK_STREAM, 0)) < 0) { /* create socket */
      Message("%s - cannot create socket 1.\n", process_name);
      return NULL;
    }

    /* Set the options of the socket */
    count = 1;
    setsockopt(s[0], SOL_SOCKET, SO_REUSEADDR, &count, sizeof count);

    if (bind(s[0], &sa, sizeof sa) < 0) { /* bind address to socket */
      Message("%s - cannot bind socket 1.\n", process_name);
      close(s[0]);
      return NULL;
    }
  }

  if ((s[1] = socket(AF_INET, SOCK_STREAM, 0)) < 0) { /* create socket */
    Message("%s - cannot create socket 2.\n", process_name);
    return NULL;
  }

  memset(&sa, 0, sizeof(struct sockaddr_in)); /* clear our address */

  sa.sin_family = hp->h_addrtype; /* this is our host address */
  sa.sin_addr.s_addr = INADDR_ANY;
  sa.sin_port = htons((u_short)(env_field->unique_key + 1));

  /* Set the options of the socket */
  count = 1;
  setsockopt(s[1], SOL_SOCKET, SO_REUSEADDR, &count, sizeof count);

  if (bind(s[1], &sa, sizeof sa) < 0) { /* bind address to socket */
    /* try to close the socket and create it again */
    close(s[1]);

    if ((s[1] = socket(AF_INET, SOCK_STREAM, 0)) < 0) { /* create socket */
      Message("%s - cannot create socket 2.\n", process_name);
      return NULL;
    }

    memset(&sa, 0, sizeof(struct sockaddr_in)); /* clear our address */

    sa.sin_family = hp->h_addrtype; /* this is our host address */
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_port = htons((u_short)(env_field->unique_key + 1));

    /* Set the options of the socket */
    count = 1;
    setsockopt(s[1], SOL_SOCKET, SO_REUSEADDR, &count, sizeof count);

    if (bind(s[1], &sa, sizeof sa) < 0) { /* bind address to socket */
      Message("%s - cannot bind socket 2.\n", process_name);
      close(s[1]);
      return NULL;
    }
  }

  listen(s[0], 3); /* max # of queued connects */
  listen(s[1], 3); /* max # of queued connects */

  return (s);
#else
  assert(!"Sockets not implemented under OS/X");
  return NULL;
#endif /* VLAB_MACX */
#else  /* WIN32 */
  assert(!"Sockets not implemented under Win32");
  return NULL;
#endif /* WIN32 */
}

/****************************************************************************/
int GetSocketConnection(int s) /* socket created with EstablishSocket() */
{
#ifdef WIN32
  assert(!"Sockets not implemented under Win32");
  return -1;
#else
#ifdef VLAB_MACX
  assert(!"Sockets not implemented under OS/X");
  return -1;
#else
  struct sockaddr_in isa; /* address of socket */
  int i;                  /* size of address */
  int t;                  /* socket of connection */

  i = sizeof(isa);          /* find socket's address */
  getsockname(s, &isa, &i); /* for accept() */

  if ((t = accept(s, &isa, &i)) < 0) { /* accept connection if there is one */
    Message("%s - socket connection was not accepted!\n", process_name);
    return -1;
  }
  return t;
#endif /* VLAB_MACX */
#endif /* WIN32 */
}

/****************************************************************************/
int CallSocket(char *hostname,u_short portnum) {
#ifdef WIN32
  assert(!"Sockets not implemented under Win32");
  return -1;
#else
#ifdef VLAB_MACX
  assert(!"Sockets not implemented under OS/X");
  return -1;
#else
  struct sockaddr_in sa;
  struct hostent *hp;
  int /*a,*/ s;

  /* do we know the host's address*/
  if ((hp = gethostbyname(hostname)) == NULL) {
    Message("%s - socket connection refused!\n", process_name);
    return (-1);
  }

  memset(&sa, 0, sizeof(sa));
  memcpy((char *)&sa.sin_addr, hp->h_addr, hp->h_length); /* set address */
  sa.sin_family = hp->h_addrtype;
  sa.sin_port = htons((u_short)portnum);

  if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) { /* get socket */
    Message("%s - cannot get the socket.\n", process_name);
    return -1;
  }

  if (connect(s, &sa, sizeof sa) < 0) { /* connect */
    Message("%s - cannot connect the socket.\n", process_name);
    close(s);
    return -1;
  }

  return s;
#endif /* VLAB_MACX */
#endif /* WIN32 */
}

/****************************************************************************/
/* parameters are: connected socket ,pointer to the buffer, and number of
   characters (bytes) to be read */
int SocketRead(int s, char *buf, int n) {
#ifdef WIN32
  assert(!"Sockets not implemented under Win32");
  return -1;
#else
  int bcount, /* counts bytes read */
      br;     /* bytes read this pass */

  bcount = 0;
  br = 0;

  while (bcount < n) { /* loop until full buffer */
    if ((br = read(s, buf, n - bcount)) > 0) {
      bcount += br; /* increment byte counter */
      buf += br;    /* move buffer ptr for next read */
    }
    if (br < 0) /* signal an error to the caller */
      return -1;
  }
  return bcount;
#endif
}

/****************************************************************************/
/* parameters are: connected socket ,pointer to the buffer, and number of
   characters (bytes) to be written */
int SocketWrite(int s, char *buf, int n) {
#ifdef WIN32
  assert(!"Sockets not implemented under Win32");
  return -1;
#else
  int bcount, /* counts bytes read */
      br;     /* bytes read this pass */

  bcount = 0;
  br = 0;

  while (bcount < n) { /* loop until full buffer */
    if ((br = write(s, buf, n - bcount)) > 0) {
      bcount += br; /* increment byte counter */
      buf += br;    /* move buffer ptr for next read */
    }
    if (br < 0) /* signal an error to the caller */
      return -1;
  }
  return bcount;
#endif
}

/****************************************************************************/
int CShouldTerminate(void) {
#ifdef WIN32
  return 0;
#else
  struct timeval time;
  fd_set fd;
  char c;

  if (confirm_socket != -1) {
    /* check the confirm_socket */
    FD_ZERO(&fd);
    FD_SET(confirm_socket, &fd);

    /* wait 0s - perform poll */
    time.tv_sec = 0;
    time.tv_usec = 0;

    if (select(getdtablesize(), &fd, NULL, NULL, &time) > 0) {
      /* there is something on the socket */
      read(confirm_socket, &c, 1);

      if (c == 'K') {
        return 1;
      }
    }
  }
  return 0;
#endif
}

/****************************************************************************/
static char skiptok_sep[256] = {0};

/* works similarly to strtok, but updates the source pointer and places it
   behind the first found separator following the found token. */
static char *skiptok(char **str, char *sep) {
  int i, len;
  char *ptr;
  char *beg = NULL;

  /* trivial cases */
  if (str == NULL)
    return NULL;
  ptr = *str;
  if (ptr == NULL)
    return NULL;
  if (*ptr == 0)
    return NULL;

  /* set array of separator flags - assumed to be initialized to 0 */
  len = strlen(sep);
  for (i = 0; i <= len; i++)
    skiptok_sep[(int)sep[i]] = 1;

  /* skip separators */
  while (skiptok_sep[(int)(*ptr)])
    if (*ptr++ == 0) {
      ptr--;
      goto end; /* separators till the end of the string */
    }

  /* ptr points to the first token character */
  beg = ptr;

  while (!skiptok_sep[(int)(*(++ptr))])
    ; /* '\0' is always a separator */

  if (*ptr == 0)
    *str = ptr;
  else {
    *str = ptr + 1; /* just behind the separator */
    *ptr = 0;       /* terminate the token */
  }

end:
  for (i = 0; i <= len; i++)
    skiptok_sep[(int)(sep[i])] = 0;

  return beg;
}

/****************************************************************************/
char *LoadOneToken(field_type *env_field, char *sep) {
  return LoadOneDataItem(env_field, sep, NULL, -1);
}

/****************************************************************************/
/* when length is negative, a string is read. Otherwise 'length' bytes are */
/* copied to 'data' and a value of 'data' is returned. */
char *LoadOneDataItem(field_type *env_field, char *sep, char *data,
                      int length) {
  char *token;

  switch (env_field->comm_type) {
  case COMM_MEMORY:
    if (env_field->in_num++ == 0) {
      if (env_field->specified == MASTER)
        env_field->act_ptr = env_field->shmadd->from_field;
      else
        env_field->act_ptr = env_field->shmadd->to_field;
    }

    if (env_field->specified == MASTER)
      if (strncmp(env_field->act_ptr, "Control", 7) == 0) {
        sscanf(env_field->act_ptr + 8, "%d", &env_field->in_flag);
        return NULL;
      }

    if (length < 0)
      return skiptok(&env_field->act_ptr, sep);
    else {
      memcpy(data, env_field->act_ptr, length);
      return data;
    }

  case COMM_SOCKETS:
    if (CShouldTerminate()) {
      env_field->in_flag = LAST_CHUNK;
      fclose(env_field->in_fp);
      env_field->in_fp = NULL;
      return NULL;
    }
    /* no break ! */
  case COMM_FILES:
  case COMM_PIPES:
    if (env_field->in_num++ == 0) {
      if (env_field->comm_type == COMM_FILES) {
#ifdef WIN32
        /* Taken from the Windows code. I have no clue what
         * this does. -- bjl */
        {
          HANDLE hFile = INVALID_HANDLE_VALUE;
          while (INVALID_HANDLE_VALUE == hFile) {
            hFile = CreateFile(env_field->in_name, GENERIC_READ, 0, NULL,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            Sleep(20);
          }
          CloseHandle(hFile);
        }
#endif
        /* open the data file */
        if ((env_field->in_fp = fopen(env_field->in_name, "r")) == NULL) {
          Message("%s - field process cannot open data file %s.\n",
                  process_name, env_field->in_name);
          return NULL;
        }
      }
    } else if (length < 0)
      /* get another token from the current line */
      if ((token = skiptok(&env_field->act_ptr, sep)) != NULL)
        return token;

    do {
      /* is it the end of the file? - shouldn't happen */
      if (env_field->comm_type == COMM_FILES) {
        assert(NULL != env_field->in_fp);
        if (feof(env_field->in_fp)) {
          Message("%s - warning - the end of file reached!\n", process_name);
          env_field->in_flag = LAST_CHUNK;
          fclose(env_field->in_fp);
          env_field->in_fp = NULL;
          return NULL;
        }
      }

      if (length < 0) {
        /* read another line from the file */
        assert(NULL != env_field->in_fp);
        if (fgets(env_field->line, sizeof(env_field->line), env_field->in_fp) ==
            NULL) {
          Message("%s - didn't manage to read a line.\n", process_name);
          return NULL;
        }
        token = env_field->line;
      } else {
        if (data == NULL)
          return NULL;

        assert(NULL != env_field->in_fp);
        if (fread(data, 1, length, env_field->in_fp) != (unsigned int)length) {
          Message("%s - didn't manage to read %d bytes.\n", process_name,
                  length);
          return NULL;
        }
        token = data;
      }

      /*
      if(env_field->verbose)
        Message("%s  - line read.\n", process_name);
        */

      if (env_field->specified == MASTER)
        if (strncmp(token, "Control", 7) == 0) {
          sscanf(token + 8, "%d", &env_field->in_flag);
          if (env_field->comm_type == COMM_FILES) {
            if (NULL != env_field->in_fp) {
              fclose(env_field->in_fp);
              env_field->in_fp = NULL;
            }
          }

          if (env_field->verbose)
            Message("%s - command line encountered.\n", process_name);

          return NULL;
        }

      if (length >= 0)
        return data;

      env_field->act_ptr = env_field->line;
    } while ((token = skiptok(&env_field->act_ptr, sep)) == NULL);

    return token;
  }

  Message("%s - internal error.\n", process_name);
  return NULL;
}

/****************************************************************************/
int SaveOneItem(field_type *env_field, char *string, char test) {
  return SaveOneDataItem(env_field, string, -1, test);
}

/****************************************************************************/
/* when length is negative, the 'data' points to a string */
int SaveOneDataItem(field_type *env_field, char *data, int length, char test) {
  switch (env_field->comm_type) {
  case COMM_MEMORY:
    if (length < 0)
      length = strlen(data);

    if (env_field->specified == MASTER) {
      if (test && (env_field->shstringend + length + 25 >= TO_FIELD_LENGTH - 1))
        /* those 25 are for the last row */
        /* not enough room left */
        return 0;
      else {
        /* add to the end */
        memcpy(env_field->shmadd->to_field + env_field->shstringend, data,
               length);
        env_field->shstringend += length;
      }
    } else {
      /* slave */
      if (test && (env_field->out_end + length + 25 >= FROM_FIELD_LENGTH - 1))
        return 0;
      else {
        memcpy(env_field->shmadd->from_field + env_field->out_end, data,
               length);
        env_field->out_end += length;
      }
    }
    break;

  case COMM_SOCKETS:
#ifdef WIN32
    assert(!"Sockets not implemented under Win32");
#endif
    if (CShouldTerminate()) {
      return 0;
    }
    /* no break ! */
  case COMM_FILES:
  case COMM_PIPES:
    /* prevent too big file/pipe/socket */
    /* no testing for pipes or sockets */
    /*    if(env_field->comm_type == COMM_FILES)*/
    if (test && (env_field->out_num > env_field->max_queries_in_file))
      return 0;

    if (env_field->verbose)
      Message("%s - saving one item into a file/pipe/socket.\n", process_name);

    assert(NULL != env_field->out_fp);
    if (length < 0) {
      /* string */
      if (fputs(data, env_field->out_fp) == EOF)
        return 0;
    } else
        /* data */
        if (fwrite(data, 1, length, env_field->out_fp) !=
            (unsigned int)length) {
      Message("%s - cannot write %d bytes to file/pipe/socket", process_name,
              length);
      return 0;
    }

    if (env_field->verbose)
      Message("%s - the item saved.\n", process_name);

    break;
  }

  return 1;
}

/****************************************************************************/
/* counts number of parameters in a printf-like format string */
static int CountParameters(char *str, int *skip) {
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
/* Parameter - name of the environment file. Returns 0 when not successful.
 */
int ReadInEnvironmentalFile(field_type *env_field, char *filename) {
  FILE *fp;
  int i;
  unsigned char flag;
  char *keywords[] = {
      "communication type",  /* 0 */
      "executable",          /* 1 */
      "turtle position",     /* 2 */
      "turtle heading",      /* 3 */
      "turtle left",         /* 4 */
      "turtle up",           /* 5 */
      "turtle line width",   /* 6 */
      "turtle scale factor", /* 7 */
      "when interpret",      /* 8 */
      "verbose",             /* 9 */
      "remote host",         /* 10 */
      "strings only",        /* 11 */
      "binary data",         /* 12 */
      "interpreted modules", /* 13  - new version of 'when interpret' */
      "following module",    /* 14 */
      NULL                   /* the last item must be NULL! */
  };
#define LINELEN 4096
  char *token, input_line[LINELEN];

  /* no file no fun */
  if (filename == NULL)
    return 0;

  if (env_field == NULL)
    return 0;

  /* defaults */
  env_field->verbose = 0;
  for (i = 0; i < 256; i++)
    env_field->interpreted_modules[i] = 0;
  env_field->expect_data_in = 0;
  env_field->data_out = 0;
  env_field->strings_only = 0;
  env_field->following_module = 1;
  env_field->binary_data = 0;
  env_field->max_queries_in_file = MAX_QUERIES_IN_PIPE;
  env_field->comm_spec_file = NULL;

  /* necessary for the slave but won't hurt master */
  env_field->formats.positionC = 0;
  env_field->formats.headingC = 0;
  env_field->formats.leftC = 0;
  env_field->formats.upC = 0;
  env_field->formats.line_widthC = 0;
  env_field->formats.scale_factorC = 0;

  /* default(s) */
#ifdef WIN32
  env_field->comm_type = COMM_FILES;
#else
  env_field->comm_type = COMM_PIPES;
#endif
  env_field->host = NULL;

  /* read in environment file */
  if ((fp = fopen(filename, "r")) == NULL) {
    Message("%s - cannot open environment file %s.\nField ignored.\n",
            process_name, filename);
    env_field->specified = 0;
    return 0;
  }

  if (env_field->verbose)
    Message("%s - environment file %s opened.\n", process_name, filename);

  /* process the file line by line */
  while (!feof(fp)) {
    /* get the whole line */
    if (fgets(input_line, LINELEN, fp) == NULL)
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
      Message("%s - unknown directive %s in environmental file.\n",
              process_name, token);
      continue;
    }

    switch (i) {
    case 0: /* communication type */
      token = strtok(NULL, " \t:\n");

      if (strcmp(token, "files") == 0) {
        env_field->comm_type = COMM_FILES;
        if (env_field->verbose)
          Message("%s - field communication using files.\n", process_name);

        env_field->max_queries_in_file = MAX_QUERIES_IN_FILE;

        if ((token = strtok(NULL, " \t:\n")) != NULL)
          if ((env_field->max_queries_in_file = atoi(token)) <= 0)
            env_field->max_queries_in_file = MAX_QUERIES_IN_FILE;
        break;
      }

      if (strcmp(token, "memory") == 0) {
        env_field->comm_type = COMM_MEMORY;
        if (env_field->verbose)
          Message("%s - field communication using memory.\n", process_name);
        break;
      }

      if (strcmp(token, "pipes") == 0) {
        env_field->comm_type = COMM_PIPES;
        if (env_field->verbose)
          Message("%s - field communication using pipes.\n", process_name);

        if ((token = strtok(NULL, " \t:\n")) != NULL)
          if ((env_field->max_queries_in_file = atoi(token)) <= 0)
            env_field->max_queries_in_file = MAX_QUERIES_IN_PIPE;
        break;
      }

      if (strcmp(token, "sockets") == 0) {
        env_field->comm_type = COMM_SOCKETS;
        if (env_field->verbose)
          Message("%s - field communication using sockets.\n", process_name);

        if ((token = strtok(NULL, " \t:\n")) != NULL)
          if ((env_field->max_queries_in_file = atoi(token)) <= 0)
            env_field->max_queries_in_file = MAX_QUERIES_IN_PIPE;
        break;
      }

      Message("%s - unknown mean of communication: %s! Pipes used.\n",
              process_name, token);
      break;

    case 1: /* executable - ignored for slave */
      if (env_field->specified == MASTER) {
        if ((token = strtok(NULL, "\n")) == NULL)
          break;
        /* till eol (for possible parameters) */

        if ((env_field->executable = (char *)malloc(strlen(token) + 3)) ==
            NULL) {
          Message("%s - not enough memory!\n", process_name);
          return -1;
        }
        strcpy(env_field->executable, token);
        strcat(env_field->executable, " &");

        if (env_field->verbose)
          Message("%s - field process: %s\n", process_name,
                  env_field->executable);
      }
      break;

    case 2: /* turtle position */
      if ((token = strtok(NULL, "\n")) == NULL)
        break; /* till eol */
      if (env_field->specified == MASTER)
        env_field->formats.position = strdup(token);
      else
        env_field->formats.positionC =
            CountParameters(token, &env_field->formats.positionS);
      break;

    case 3: /* turtle heading */
      if ((token = strtok(NULL, "\n")) == NULL)
        break; /* till eol */
      if (env_field->specified == MASTER)
        env_field->formats.heading = strdup(token);
      else
        env_field->formats.headingC =
            CountParameters(token, &env_field->formats.headingS);
      break;

    case 4: /* turtle left */
      if ((token = strtok(NULL, "\n")) == NULL)
        break; /* till eol */
      if (env_field->specified == MASTER)
        env_field->formats.left = strdup(token);
      else
        env_field->formats.leftC =
            CountParameters(token, &env_field->formats.leftS);
      break;

    case 5: /* turtle up */
      if ((token = strtok(NULL, "\n")) == NULL)
        break; /* till eol */
      if (env_field->specified == MASTER)
        env_field->formats.up = strdup(token);
      else
        env_field->formats.upC =
            CountParameters(token, &env_field->formats.upS);
      break;

    case 6: /* turtle line width */
      if ((token = strtok(NULL, "\n")) == NULL)
        break; /* till eol */
      if (env_field->specified == MASTER)
        env_field->formats.line_width = strdup(token);
      else
        env_field->formats.line_widthC =
            CountParameters(token, &env_field->formats.line_widthS);
      break;

    case 7: /* turtle scale factor */
      if ((token = strtok(NULL, "\n")) == NULL)
        break; /* till eol */
      if (env_field->specified == MASTER)
        env_field->formats.scale_factor = strdup(token);
      else
        env_field->formats.scale_factorC =
            CountParameters(token, &env_field->formats.scale_factorS);
      break;

    case 8: /* when interpret */
      while ((token = strtok(NULL, "\n ,;\t"))) {
        i = atoi(token);
        if (i < 0)
          flag = 0xff;
        else {
          if (i > 6) {
            Message("%s - cannot distinguish modules with more than 6 "
                    "parameters, use command "
                    "interpreted modules"
                    ".\n",
                    process_name);
            continue;
          }

          flag = 0x40 >> i;
        }

        for (i = 0; i < 256; i++)
          env_field->interpreted_modules[i] |= flag;
      }
      break;

    case 9: /* verbose */
      if ((token = strtok(NULL, "\n ")) == NULL)
        break;
      if (strcmp(token, "on") == 0)
        env_field->verbose = 1;
      break;

    case 10: /* host - ignored for slave */
      if (env_field->specified == MASTER) {
        if ((token = strtok(NULL, "\n")) == NULL)
          break;
        env_field->host = strdup(token);
      }
      break;

    case 11: /* strings only */
      if ((token = strtok(NULL, "\n ")) == NULL)
        break;
      if (strcmp(token, "on") == 0)
        env_field->strings_only = 1;
      break;

    case 12: /* binary data */
      if ((token = strtok(NULL, "\n ")) == NULL)
        break;
      if (strcmp(token, "on") == 0) {
        env_field->binary_data = 1;
        env_field->strings_only = 1;
      }
      break;

    case 13: /* iterpreted modules */
      for (;;) {
        if ((token = strtok(NULL, ",;\n ")) == NULL)
          break;

        if (!strcmp(token, "all")) {
          for (i = 0; i < 256; i++)
            env_field->interpreted_modules[i] = 0x80;
          break;
        }

        switch (strlen(token)) {
        case 0:
          break;
        case 1:
          /* the module with any number of paramaters */
          env_field->interpreted_modules[(unsigned char)token[0]] = 0x80;
          break;

        case 4:
          /* the module with specified number of paramaters */
          env_field->interpreted_modules[(unsigned char)token[0]] =
              0x40 >> (token[2] - '0');
          break;

        default:
          Message("%s - unrecognized module %s (possibly parameter > 6).\n",
                  process_name, token);
        }
      }
      break;

    case 14: /* following module */
      if ((token = strtok(NULL, "\n ")) == NULL)
        break;
      if (strcmp(token, "yes") == 0)
        env_field->following_module = 1;
      if (strcmp(token, "no") == 0)
        env_field->following_module = 0;
      break;

    default:
      Message("%s - environment file: unknown keyword %s.\n", process_name,
              token);
    }
  }

  return 1;
}

/****************************************************************************/
void CTerminate(void) {
  CMFreeStructures();
  CSTerminate();

#ifdef WIN32

#else
  if (confirm_socket != -1) {
    char c = 'T';

    write(confirm_socket, &c, 1);
  }
#endif
}

/****************************************************************************/
void CInitialize(char *program_name, char *command_string) {
  char c;
  int initialize_masters = 1;
  int initialize_slaves = 1;
  int key = -1, aux;
  char *ptr, *token, *token2, *host = NULL;
  char verbose = 0;

  /* default values */
  host = NULL;
  confirm_socket = -1;

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

  if (command_string == NULL)
    return;

  token = skiptok(&command_string, " ,:");

  while (token != NULL) {
    /* get switches */
    if (token[0] != '-')
      Message("%s - switch -v, -m, -s, or -c expected.\n", process_name);
    else
      switch (token[1]) {
      case 'v': /* verbose */
        verbose = 1;
        break;

      case 'm': /* master */
        if (initialize_masters) {
          CMInitialize();
          initialize_masters = 0;
        }
        /* comm. specs file */
        if ((token = skiptok(&command_string, " ,:")) == NULL)
          break;

        /* socket number */
        if ((token2 = skiptok(&command_string, " ,:")) == NULL)
          break;
        aux = atoi(token2);

        if (CMAddConnection(token, aux) < 0) {
          Message("%s - process terminated.\n", process_name);
          CTerminate();
          exit(0);
        }

        if (verbose)
          Message("%s - one slave added.\n", process_name);

        break;

      case 's': /* slave */
        if (initialize_slaves) {
          CSInitializeStructures();
          initialize_slaves = 0;
        }

        /* comm. specs file */
        if ((token = skiptok(&command_string, " ,:")) == NULL)
          break;

        /* socket number */
        if ((token2 = skiptok(&command_string, " ,:")) == NULL)
          break;
        aux = atoi(token2);

        /* host */
        if ((token2 = skiptok(&command_string, " ,:")) == NULL)
          break;

        if (!CSAddConnection(token, aux, token2)) {
          Message("%s - process terminated.\n", process_name);
          CTerminate();
          exit(0);
        }

        if (verbose)
          Message("%s - one master added.\n", process_name);

        break;

      case 'c': /* where to confirm execution */
        if ((token = skiptok(&command_string, " ,:")) == NULL)
          break;
        key = atoi(token);

        if ((token = skiptok(&command_string, " ,:")) == NULL)
          break;
        host = strdup(token);

        /* confirm sucessful execution */

#ifdef WIN32

#else
        if ((key != -1) && (host != NULL)) {
          if ((confirm_socket = CallSocket(host, key)) < 0) {
            Message("%s - cannot connect to confirm socket %d!\n", process_name,
                    key);
            exit(0);
          }

          free(host);

          c = 1;
          write(confirm_socket, &c, 1);

          if (verbose)
            Message("%s - successful execution confirmed.\n", process_name);
        }
#endif
        break;

      default:
        Message("%s - switch -m, -s, or -c expected.\n", process_name);
        goto out;
      }
    token = skiptok(&command_string, " ,:");
  }

out:

  if (verbose)
    Message("Process %s initialized.\n", process_name);
}
