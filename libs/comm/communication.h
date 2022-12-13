/* ******************************************************************** *
   Copyright (C) 1990-2022 University of Calgary
  
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
  
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * ******************************************************************** */




#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__

/************************************************************************/
/* types of communication */
#define COMM_MEMORY 0
#define COMM_FILES  1
#define COMM_PIPES  2
#define COMM_SOCKETS 3

/************************************************************************/
/* semaphores  - used for memory or files communication */
/* semaphore key is determined from the cpfg's pid: (pid<<2)+0x13 */

#ifndef WIN32
/* semun definition stolen from lpfg:semaphoreLnx.imp */
#if (defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)) || defined(__MACH__)
    /* union semun is defined by including <sys/sem.h> */
#else
    /* according to X/OPEN we have to define it ourselves */
    union semun {
        int val;                    /* value for SETVAL */
        struct semid_ds *buf;       /* buffer for IPC_STAT, IPC_SET */
        unsigned short int *array;  /* array for GETALL, SETALL */
        struct seminfo *__buf;      /* buffer for IPC_INFO */
    };
#endif /* __GNU_LIBRARY__ etc. */
#endif /* !WIN32 */

/* values of the flag */
#define FIRST_CHUNK  1
#define LAST_CHUNK   4 /* when you are recompiling all main.c's change it to 2*/
#define PROCESS_EXIT 8


#define LENGTH_OF_BINARY_CHUNK 80

/* shared memory  - if files not used */
/* shared memory key is always the actual semaphore key minus one */

#define TO_FIELD_LENGTH    50000
#define FROM_FIELD_LENGTH  10000

#define MAX_QUERIES_IN_FILE 1000 /* unless changed in specification file */
#define MAX_QUERIES_IN_PIPE 100  /* even pipe can choke on too much input,
				    especially if environment answers
				    immediately */

struct shared_memory_type {
  char to_field[TO_FIELD_LENGTH];
  char from_field[FROM_FIELD_LENGTH];
};

typedef struct shared_memory_type shared_memory_type;

/* files */

#define FILENAME_TO_FIELD   ".to_field"
#define FILENAME_FROM_FIELD ".from_field"

#define DATA_BEGIN "Dbegin"
#define DATA_END   "Dend"


/* structure for all necessary environmental parameters 
   Same for both master and slave although svale doesn't need all parameters.
*/
#define MASTER 1
#define SLAVE  2

/* format strings for saving the turtle */
struct turtle_format_type
{
  char *position;
  int  positionC, positionS;  /* number of parameters sent, how many characters to skip */
  char *heading;
  int  headingC, headingS;
  char *left;
  int  leftC, leftS;
  char *up;
  int  upC, upS;
  char *line_width;
  int  line_widthC, line_widthS;
  char *scale_factor;
  int  scale_factorC, scale_factorS;
};
typedef struct turtle_format_type turtle_format_type;


#define LLEN 2048 /* length of buffered line */

#ifdef WIN32
typedef int key_t;

typedef struct twosemaphores
{
	HANDLE hSem0, hSem1;
} TwoSemaphores;
#endif

struct field_type
{
  char verbose;
  char specified;        /* 0 - not valid, 1 - master, 2 - slave */
  char *executable;
  char *comm_spec_file;
  char comm_type;        /* memory / files /pipes */
  key_t unique_key;      /* unique key for allocating semaphores, shared
			  * memory, and sockets */
#ifdef WIN32
  TwoSemaphores semid;
#else
  int semid;             /* id of a pair of semaphores */
#endif

  int  in_num;           /* number of queries */
  int  out_num;
  int  in_flag;          /* combination of FIRST_CHUNK and LAST_CHUNK */
  int  out_flag;         /* combination of FIRST_CHUNK and LAST_CHUNK, 
			    or PROCESS_EXIT */

  int current_step;      /* current generation step */

  char strings_only;     /* processes exchange just strings */
  char binary_data;

  char following_module; /* send also the module following ?E */

  char interpret_all;    /* interpret all modules following ?E */
  unsigned char interpreted_modules[256];
                         /* for what modules with what number of 
			    parameters are interpreted? 
			     the item is stored as 8 bit flags:
			     bit 0:all parameters, otherwise
			     bits 1-7: allow 0-6 parameters*/
  char data_out ;         /* a set of strings follows the symbol */
  char expect_data_in ;   /* a set of strings follows the symbol */

  char expect_input_from_env;

  char *host;            /* name of the machine where the process is spawned */

  FILE *in_fp;           /* file pointer of the input data file/pipe */
  char *in_name;
  char line[LLEN];
  FILE *out_fp;          /* file pointer of the output data file/pipe */
  char *out_name;

  int max_queries_in_file;

  turtle_format_type formats;  /* format strings for saving the turtle */
  char *strbeg;          /* pointer to the pointer to the string */

  int shmid;             /* index of shared memory - if files not used */
  shared_memory_type  *shmadd;     /* address of shared memory  */
  int  shstringend;     /* index of the active end of the shared string */
  int  out_end;         /* index of the active end of the shared string */

#ifdef XXX
  int (*SetCommModulePars)(unsigned long distance,
			   Cmodule_type *comm_module,
			   int index);
                /* function called when results from the servant are coming */
#endif
  char *act_ptr;        /* used for reading in the text information */

};

typedef struct field_type field_type;


extern char process_name[256];    /* master or slave */

int  CMAddConnection(char *filename, int key);
int CSAddConnection(char *filename, int key, char *host_name);
int CSInitializeStructures(void);

/* prototypes */
int InitializeSharedMemory(field_type *env_field);
void ReleaseSharedMemory(field_type *env_field);

int InitializeSemaphores(field_type *env_field);
void ReleaseSemaphores(field_type *env_field);

int SocketRead(int s, char *buf, int n);
int SocketWrite(int s, char *buf, int n);
int *EstablishSocket(field_type *env_field, char *host);
int GetSocketConnection(int s);
int CallSocket(char *hostname, u_short portnum);

int ReadInEnvironmentalFile(field_type *env_field, char *filename);
char *LoadOneToken(field_type *env_field, char *sep); 
char *LoadOneDataItem(field_type *env_field, char *sep, char *data, 
		      int length); 
int SaveOneItem(field_type *env_field, char *string, char test);
int SaveOneDataItem(field_type *env_field, char *data, int length, char test);

#else
	#error File already included
#endif

