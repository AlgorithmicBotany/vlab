/*
  Environmental process - density by Honda 1981
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "honda81.h"
#include "comm_lib.h"

/**** field specific variables ****/
#define QUERY_ARRAY_SIZE 100

/* queries to be answered */
struct item_type {
  float position[3];
  float vigor;
  int index;
  unsigned long dist;
  int master;
};
typedef struct item_type item_type;

item_type *queries;
int num_queries;
int query_array_size;

char verbose;
char is3d;

float radius2;  /* squared radius */


/****************************************************************************/
void FreeFieldStructures(void)
{
  if(queries != NULL) 
    free(queries);
  queries = NULL;
}

/****************************************************************************/
void InitializeFieldStructures(void)
{
  FreeFieldStructures();
  num_queries = 0;

  query_array_size = QUERY_ARRAY_SIZE;
  if((queries = (item_type*)malloc(query_array_size*sizeof(item_type)))
     == NULL) {
    fprintf(stderr,"honda81 - cannot allocate memory for querries.\n");
    exit(0);
  }
}

/****************************************************************************/
void DetermineResponse(void)
{
  int i, j;
  float vec[3];
  Cmodule_type comm_symbol;

  comm_symbol.num_params = 1;
  comm_symbol.params[0].set = 1;
  
  /* for all queries */
  for(i=0; i< num_queries; i++) {
    comm_symbol.params[0].value = 1;
    
    for(j=0; j< num_queries; j++) 
      if((i!=j)&&(queries[i].index == queries[j].index)&&
	 (queries[i].vigor <= queries[j].vigor)) {
	vec[0] = queries[i].position[0] - queries[j].position[0];
	vec[1] = queries[i].position[1] - queries[j].position[1];

	if(is3d) {
	  /* 3d case */
	  vec[2] = queries[i].position[2] - queries[j].position[2];


	  if(vec[0]*vec[0]+vec[1]*vec[1]+vec[2]*vec[2] <= radius2)
	    comm_symbol.params[0].value = 0;
	}
	else
	  /* 2d case */
	  if(vec[0]*vec[0]+vec[1]*vec[1] <= radius2)
	    comm_symbol.params[0].value = 0;
      }
      
    CSSendData(queries[i].master, queries[i].dist, &comm_symbol);
  }
}

/****************************************************************************/
void StoreQuery(int master, unsigned long module_id, Cmodule_type *comm_symbol,
		CTURTLE *tu)
{
  if(tu->positionC < 3) {
    fprintf(stderr,
	    "honda81 - turtle position wasn't sent to the environment.\n");
    return;
  }


  if(comm_symbol->num_params == 0) {
    fprintf(stderr,
	    "honda81 - not enough parameters associated with ?E.\n");
    return;
  }

  if(num_queries >= query_array_size) {
    /* reallocate */
    query_array_size *=2;
    
    if((queries = (item_type*)realloc(queries,
				      query_array_size*sizeof(item_type)))
       == NULL) {
      fprintf(stderr,"honda81 - cannot reallocate memory for querries.\n");
      exit(0);
    }
    if(verbose)
      fprintf(stderr,"honda81 - queries reallocated. New size is %d.\n",
	      query_array_size);
  }

  queries[num_queries].position[0] = tu->position[0];
  if(is3d) {
    /* 3d case */
    queries[num_queries].position[1] = tu->position[1];
    queries[num_queries].position[2] = tu->position[2];
  }
  else
    /* 2d case */
    queries[num_queries].position[1] = tu->position[2];

  queries[num_queries].dist = module_id;
  queries[num_queries].master = master;
  queries[num_queries].vigor = comm_symbol->params[0].value;
  
  queries[num_queries].index = (comm_symbol->num_params > 1 ? 
				comm_symbol->params[1].value : 0);

  num_queries++;
}

/****************************************************************************/
void ProcessArguments(int argc, char **argv)
{
  FILE *fp;
  int i;
  char *keywords[] = {
    "verbose",              /*  0 */
    "radius",               /*  1 */
    "3d case",              /*  2 */
    NULL                    /* the last item must be NULL! */
  };
  char *token, input_line[255];

  if(argc == 1) {
    printf("honda81 - not enough arguments!\n"
	   "USAGE: honda81 -e environment_file honda81_spec_file\n");
    exit(0);
  }

  /* defaults */
  verbose = 0;
  is3d = 0;

  radius2 = 0;

  InitializeFieldStructures();

  /* read in environment file */
  if((fp = fopen(argv[1],"r")) == NULL)
    fprintf(stderr, "honda81 - cannot open specification file %s.\n", argv[1]);
  else {
    /* process the file line by line */
    while(!feof(fp)) {
      /* get the whole line */
      if(fgets(input_line, 255, fp) == NULL) break;
	
      /* get the keyword */
      token = strtok(input_line,"\t:");
	
      /* look for a keyword in the table */
      i = 0;
      while( keywords[i] != NULL) {
	if(strcmp(keywords[i], token) == 0) break;
	i++;
      }
      
      if(keywords[i] == NULL) {
	fprintf(stderr,
		"honda81 - unknown directive %s in the specification file.\n", 
		token);
	continue; 
      }
      
      switch(i) {
       case 0: /* verbose */
	token = strtok(NULL,"x,; \t:\n");
	if(token==NULL) break;
	if(strcmp(token,"on")==0) verbose = 1;
	break;

       case 1: /* radius */
	token = strtok(NULL,"x,; \t:\n");
	if(token==NULL) break;
	radius2 = (float) atof(token);
	radius2 *=radius2;
	break;

       case 2: /* is 3d case */
	token = strtok(NULL,"x,; \t:\n");
	if(token==NULL) break;
	if(strcmp(token,"on")==0) is3d = 1;
	break;

      }
    }
  }

  if(verbose) {
    fprintf(stderr, "honda81 - squared radius: %g\n", radius2);
  }
}

/****************************************************************************/
void MainLoop(void)
{
  Cmodule_type two_modules[2];
  unsigned long module_id;
  int master;
  CTURTLE turtle;
  int i;

  /* infinite loop - until signal 'exit' comes */
  for(;;) {
    if(verbose)
      fprintf(stderr, "honda81 - start processing data.\n");  

    num_queries = 0;

    CSBeginTransmission();

    /* process the data */
    while(CSGetData(&master, &module_id, two_modules, &turtle)) {
      if(verbose) {
	fprintf(stderr,"honda81 - comm. symbol has %d parameters:\n      ",
		two_modules[0].num_params);
	for(i=0; i<two_modules[0].num_params; i++)
	  fprintf(stderr," %g", two_modules[0].params[i].value);
	fprintf(stderr,"\n");
	
	fprintf(stderr,"\n");
      }

      StoreQuery(master, module_id, two_modules, &turtle);
    }

    DetermineResponse();

    /* End transmission returns 1 when the process is requested to exit */
    if(CSEndTransmission()) break;
  }
}
 
/****************************************************************************/
int main(int argc, char **argv)
{
  char *process_name = strdup(argv[0]);

  /* initialize the communication as the very first thing */
  CSInitialize(&argc, &argv);

  ProcessArguments(argc,argv);

  fprintf(stderr, "Field process %s successfully initialized.\n", process_name);

  MainLoop();

  FreeFieldStructures();

  fprintf(stderr, "Field process %s exiting.\n", process_name);

  /* should be the last function called */
  CTerminate();

  return 1;
}
