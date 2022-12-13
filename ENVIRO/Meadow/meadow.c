/*
 *Environmental process - meadow
 * takes an rgb image and extracts green level to be used as plant density
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <image.h>
#include "targa.h"
#include "meadow.h"
#include "comm_lib.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif



/**** field specific variables ****/
/* cell type */
typedef struct {
unsigned char visits, plants;
} CELL_TYPE;

#define VRADIUS 2

/* queries - each represents an insect */
#define QUERY_ARRAY_SIZE 1000
struct QUERY_UNIT_TYPE {
  int insectid;
  float pos[2], dir[2];
  unsigned long dist;        /* position in L-system string */
  int master;
};
typedef struct QUERY_UNIT_TYPE QUERY_UNIT_TYPE;

QUERY_UNIT_TYPE *queries = NULL;
int num_queries;
int query_array_size;

#define X 0
#define Y 1

struct grid_type {
  int size[2];         /* size of the grid (in nodes) */
  float range[2];      /* size in coordinates */
  float pos[2];        /* position of lower left front corner */
  CELL_TYPE *data;
  char *input_image_name;
  char *input_data_name;
};
typedef struct grid_type grid_type;


grid_type grid = {0};

char verbose;

/****************************************************************************/
void FreeFieldStructures(void)
{
  if(grid.data != NULL) {
    free(grid.data);
    grid.data = NULL;
  }

  if(grid.input_image_name != NULL) {
    free(grid.input_image_name);
    grid.input_image_name = NULL;
  }

  if(grid.input_data_name != NULL) {
    free(grid.input_data_name);
    grid.input_data_name = NULL;
  }

  if(queries != NULL) {
    free(queries);
    queries = NULL;
  }
  num_queries = 0;
}

/****************************************************************************/
void InitializeQueryArray(void)
{
  if(queries != NULL) {
    free(queries);
    queries = NULL;
  }

  query_array_size = QUERY_ARRAY_SIZE;

  if((queries = (QUERY_UNIT_TYPE*)malloc(query_array_size*
					 sizeof(QUERY_UNIT_TYPE)))
     == NULL) {
    fprintf(stderr,"meadow - cannot allocate memory for queries!\n");
    exit(0);
  }
  num_queries = 0;
}

/****************************************************************************/
void InsertQueryInArray(int master, unsigned long module_id,
		  Cmodule_type *comm_symbol, CTURTLE *tu)
{
   int c;
   
   if(num_queries >= query_array_size) {
    /* rellocate the array */
    query_array_size *= 2;
    if((queries = (QUERY_UNIT_TYPE*)
	realloc(queries, query_array_size*sizeof(QUERY_UNIT_TYPE))) == NULL) {
      fprintf(stderr,"meadow - cannot reallocate memory for query array!\n");
      exit(0);
    }
    if(verbose)
      fprintf(stderr,"meadow - query array reallocated to size %d.\n",
	      query_array_size);
  }
  
  /* parameters for interprocess communications */
  queries[num_queries].dist = module_id;
  queries[num_queries].master = master;

  /* insect parameters */
  queries[num_queries].insectid = comm_symbol[1].params[0].value;
  for(c=X;c<=Y;c++) {
    queries[num_queries].pos[c] = tu->position[c];
    queries[num_queries].dir[c] = tu->heading[c];
  }

  num_queries++;
}

/****************************************************************************/
CELL_TYPE *GetCell(grid_type *grid, int x, int y)
{
  if(x < 0) return NULL;
  if(x >= grid->size[X]) return NULL;

  if(y < 0) return NULL;
  if(y >= grid->size[Y]) return NULL;

  return grid->data + y*grid->size[X] + x;
}


/****************************************************************************/
int LoadInitialImage(grid_type *grid)
{
  int x,y;
  float xratio, yratio;
  IMAGE *image;
  unsigned short *buf;
  CELL_TYPE *ptr;
  targa_params_type TGAspec;
  int c;
  
  
  if((c=strlen(grid->input_image_name))>3)
    if(strcmp(grid->input_image_name+c-3,"tga")==0) {
      FILE *fp;
      int xsize,ysize;
      unsigned char  *cbuf, *rowbuf, tmp, *ptrc;

      /* load in targa image */
      if(!loadTGAhead(grid->input_image_name, &TGAspec)) {
	fprintf(stderr,"meadow: cannot open input targa image %s.\n",
		grid->input_image_name);
	return 0;
      }
      xsize = TGAspec.Xres;
      ysize = TGAspec.Yres;

      if((cbuf=(unsigned char*)malloc(xsize*ysize*sizeof(unsigned char))) 
	 == NULL) {
	fprintf(stderr,"meadow: cannot allocate memory for image %s!\n",
		grid->input_image_name);
	return 0;
      }
	  
      if((rowbuf=(unsigned char*)malloc(xsize*3*sizeof(unsigned char))) 
	 == NULL) {
	fprintf(stderr,"meadow: cannot allocate memory for image %s!\n",
		grid->input_image_name);
	return 0;
      }
	  
      /* read the image in - starting with the last row */
      /* image is stored in rows of R, G, and B. We need R,G,B for each 
	 pixel */
      for(y=ysize-1; y>=0; y--) {
		loadTGArow(&TGAspec, y, rowbuf);
	
		ptrc = cbuf+y*xsize;
		for(x=0;x<xsize;x++) {
		  /* get only green */
		  *(ptrc++) = rowbuf[x*3+1];
		}
	  }

      xratio =  (float)(xsize-1) / (float)(grid->size[X]-1);
      yratio = (float)(ysize-1) / (float)(grid->size[Y]-1);


      /* set the 2d grid - assuming that it is bigger than the input image*/
      ptr = grid->data;
      
      for(y=0; y<grid->size[Y]; y++)
		for(x=0; x<grid->size[X]; x++) {
		  ptr->plants = cbuf[xsize*(int)(y*yratio) + (int)(x*xratio)];
		  ptr->visits = 0;
		  ptr++;
		}
      
      loadTGAfinish(&TGAspec);
      free(cbuf);
      free(rowbuf);

      fclose(fp);
	
      return 1;
    }

#ifndef SUN
  /* otherwise open rgb image */
  if((image = iopen(grid->input_image_name,"r",0,0,0,0,0)) == NULL) {
    fprintf(stderr,"meadow - cannot open image %s!\n", grid->input_image_name);
    return 0;
  }
  
  xratio =  (float)(image->xsize-1) / (float)(grid->size[X]-1);
  yratio = (float)(image->ysize-1) / (float)(grid->size[Y]-1);

  /* one line buffer */
  if((buf = (unsigned short*)malloc(image->xsize * image->ysize *
				    sizeof(unsigned short))) == NULL) {
    fprintf(stderr,
	    "meadow - cannot allocate memory for one channel of image %s!\n",
	    grid->input_image_name);
    return 0;
  }
  
  /* Read the image in. Only green channel is stored .*/
  /* Later on, R and B channel can be used for nutrients or obstales. */

  for(y=0; y<image->ysize; y++) {
    /* green */
    getrow(image, buf + image->ysize*y, y, 1);
  }
  
  /* set the 2d grid - assuming that it is bigger than the input image*/
  ptr = grid->data;

  for(y=0; y<grid->size[Y]; y++) {
    for(x=0; x<grid->size[X]; x++) {
      ptr->plants = buf[image->xsize*(int)(y*yratio) + (int)(x*xratio)];
	  ptr->visits = 0;
      ptr++;
    }
  }

  free(buf);
  iclose(image);
#else
  fprintf(stderr,"meadow: reading og rgb image not implemented on suns!.\n");

  ptr = grid->data;

  for(y=0; y<grid->size[Y]; y++)
    for(x=0; x<grid->size[X]; x++) {
      ptr->plants = 255;
      ptr->visits = 0;
      ptr++;
    }
#endif

  return 1;
}

/****************************************************************************/
int LoadInitialData(grid_type *grid)
{
  int x, y, c, node[2], range[2];
  char type,  input_line[255];
  float density, radius, position[2], length, width;
  CELL_TYPE *cell;
  FILE *dataFile;

  /* open data file */
  if((dataFile = fopen(grid->input_data_name,"r")) == NULL) {
    fprintf(stderr,"meadow - cannot open data file %s!\n", grid->input_data_name);
    return 0;
  }
  
  /* process the file line by line */
  while(!feof(dataFile)) {
      /* get the whole line */
      if(fgets(input_line, 255, dataFile) == NULL) break;
	
      /* get the line type */
	  sscanf(input_line, " %c", &type);
            
	  switch (type) {
	  case '1':
		if (sscanf(input_line, "1 %f %f %f %f", 
			&density, &radius, &position[X], &position[Y])!=4) break;
		range[X]=radius;
		range[Y]=radius;
		if (density>255) density=255;
		if (density<0)density=0;
		for(c=X;c<=Y;c++) {
			node[c] = floor((position[c] - grid->pos[c])
				/grid->range[c] * (float)grid->size[c]);
			range[c] = range[c] / grid->range[c] * (float)grid->size[c] + 1;
		}

		for(y = -range[Y]; y < range[Y]; y++) {
			for(x = -range[X]; x < range[X]; x++) {
				/* only mark those cells in the circle */
				if ((float)(x*x + y*y)/(float)(range[Y]*range[Y]) <= 1 &&
					(cell = GetCell(grid, node[X]+x, node[Y]+y)) != NULL) {
						cell->plants = density;
				}
			}
		}
 		break;
	  case '2':
		if (sscanf(input_line, "2 %f %f %f %f %f", 
			&density, &length, &width, &position[X], &position[Y]) !=5) break;;
		range[X]=length;
		range[Y]=width;
		if (density>255) density=255;
		if (density<0)density=0;
		for(c=X;c<=Y;c++) {
			node[c] = floor((position[c] - grid->pos[c])
				/grid->range[c] * (float)grid->size[c]);
			range[c] = range[c] / grid->range[c] * (float)grid->size[c] + 1;
		}
		for(y = 0; y < range[Y]; y++) {
			for(x = 0; x < range[X]; x++) {
				if ((cell = GetCell(grid, node[X]+x, node[Y]+y)) != NULL) {
					cell->plants = density;
				}
			}
		}
 		break;
	  case '9': /* end of file marker for cpfg */
		break;
	  default:
		fprintf(stderr,"meadow - unknown data type %c in %s\n",  type, grid->input_data_name);
		break;
	  }
  }
 
  fclose(dataFile);
  return 1;
}

/****************************************************************************/
void InitializeGrid(grid_type *grid)
{
  int x;

  if((grid->data = (CELL_TYPE*)
      malloc(grid->size[X]*grid->size[Y]*sizeof(CELL_TYPE)))
     == NULL) {
    fprintf(stderr,"meadow - cannot allocate memory for grid!\n");
    exit(0);
  }
  /* initialise to zero */
  for(x=0; x < grid->size[Y]*grid->size[X]; x++) {
      grid->data[x].plants = 0;
      grid->data[x].visits = 0;
  }
	
  if(grid->input_image_name != NULL) {
    LoadInitialImage(grid);
  }
  if(grid->input_data_name != NULL) {
    LoadInitialData(grid);
  }
  if(grid->input_image_name == NULL && grid->input_data_name == NULL ) {
	fprintf(stderr,"meadow - no input data. Maximum density of plants assumed.\n");
    
    for(x=0; x < grid->size[Y]*grid->size[X]; x++) {
      grid->data[x].plants = 255;
    }	  
  }

  if(verbose)
    fprintf(stderr,"meadow - grid initialized.\n");
}

/****************************************************************************/
void ProcessQuery(grid_type *grid, int master, unsigned long module_id,
		  Cmodule_type *comm_symbol, CTURTLE *tu)
{
  int c;
  CELL_TYPE *cell;
  int node[2];
  
  if(comm_symbol[0].num_params != 6) {
    if(verbose)
      fprintf(stderr,"meadow - six parameters for ?E required!\n");
    return; 
  }
  if(comm_symbol[1].num_params != 3) {
    if(verbose)
      fprintf(stderr,"meadow - three parameters for I required!\n");
    return; 
  }

  if(tu->positionC < 2) {
    fprintf(stderr,
	    "meadow - turtle position wasn't sent to the environment.\n");
    return;
  }

  if(tu->headingC < 2) {
    fprintf(stderr,
	    "meadow - turtle heading wasn't sent to the environment.\n");
    return;
  }

  InsertQueryInArray(master, module_id, comm_symbol, tu);
  
  /* record cells visited */
  for(c=X;c<=Y;c++) {
    node[c] = floor((tu->position[c] - grid->pos[c])
		    /grid->range[c] * (float)grid->size[c]);
  }
  cell = GetCell(grid, node[X], node[Y]);
	
  if(cell != NULL) 
	  if(cell->visits<255)		      
	    cell->visits += 1;
}


/****************************************************************************/
void DetermineResponse(grid_type *grid)
{
  int q, qq, c, node[2];
  CELL_TYPE *cell;
  Cmodule_type comm_symbol;
  
  double density, gradient[2], da, angle, len;
  double dist, tdist;
  int closest;
  
  if(verbose) {  
    fprintf(stderr, "meadow - start determining response for each of %d"
	    " queries.\n", num_queries);
  }

  comm_symbol.num_params = 6;
  comm_symbol.params[0].set = 1;
  comm_symbol.params[1].set = 1;
  comm_symbol.params[2].set = 1;
  comm_symbol.params[3].set = 1;
  comm_symbol.params[4].set = 1;
  comm_symbol.params[5].set = 1;

  /* for all queries */
  for(q=0; q<num_queries; q++) {
    /* determine density and "gradient" of plants */
		  /* get value from current position */   
	for(c=X;c<=Y;c++)
      node[c] = floor((queries[q].pos[c] - grid->pos[c])
		      /grid->range[c] * (float)grid->size[c]);
	if((cell = GetCell(grid, node[X], node[Y])) == NULL) {
	  density = 0;
	}
	else {
	  density = cell->plants;
	}
	gradient[X]=0;
	gradient[Y]=0;
	/* determine the surrounding density */
    for(da= 0;da<1;da=da+.125) {
	  angle=da*2*M_PI;
      node[X] = floor((queries[q].pos[X]+cos(angle)*VRADIUS - grid->pos[X])
		      /grid->range[X] * (float)grid->size[X]);
      node[Y] = floor((queries[q].pos[Y]+sin(angle)*VRADIUS - grid->pos[Y])
		      /grid->range[Y] * (float)grid->size[Y]);
	  if((cell = GetCell(grid, node[X], node[Y])) != NULL) {
		density=density+cell->plants;
		gradient[X]=gradient[X]+cos(angle)*cell->plants;
 		gradient[Y]=gradient[Y]+sin(angle)*cell->plants;
	  }
    }
	/* assign response */
	comm_symbol.params[0].value=density/9;
	len=hypot(gradient[X], gradient[Y]);
	if (len==0) len=1; /* watch out for divide by 0 */
	comm_symbol.params[1].value=gradient[X]/len;
	comm_symbol.params[2].value=gradient[Y]/len;
	
	/* determine nearest insect within VRADIUS */
	closest= -1;
	dist=VRADIUS;
	for(qq=0; qq<num_queries; qq++) {
		/* search through other insects for closest */
		if (qq != q) {
			tdist=hypot(queries[q].pos[X]-queries[qq].pos[X],
					  queries[q].pos[Y]-queries[qq].pos[Y]);
			if (tdist < dist) {
				closest=qq;
				dist=tdist;
			} 
		}
	}
 	comm_symbol.params[3].value=closest;
 	comm_symbol.params[4].value=queries[closest].pos[X]-queries[q].pos[X];
 	comm_symbol.params[5].value=queries[closest].pos[Y]-queries[q].pos[Y];
     
    CSSendData(queries[q].master, queries[q].dist, &comm_symbol);

    if(verbose)
      fprintf(stderr, "meadow - plant density value: %d.\n",
	      comm_symbol.params[0].value);
  }
}


/****************************************************************************/
void ProcessArguments(int argc, char **argv)
{
  FILE *fp;
  int i;
  char *keywords[] = {
    "grid size",            /*  0 */
    "grid position",        /*  1 */
    "verbose",              /*  2 */
    "input image",          /*  3 */
    "grid range",            /*  4 */
    "input data",          /*  5 */
    NULL                    /* the last item must be NULL! */
  };
  char *token, input_line[255];

  FreeFieldStructures();

  /* defaults */
  verbose = 0;
  
  grid.size[X] = 512;
  grid.size[Y] = 512;
  grid.pos[X] = -1;
  grid.pos[Y] = -1;
  grid.range[X] = 2;
  grid.range[Y] = 2;

  if(argc == 1) {
    printf("meadow - not enough arguments!\n"
	   "USAGE: meadow -e environment_file spec_file\n");

    exit(0);
  }

  /* read in environment file */
  if((fp = fopen(argv[1],"r")) == NULL)
    fprintf(stderr, "meadow - cannot open specification file %s.\n",argv[1]);
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
		"meadow - unknown directive %s in the specification file.\n",
		token);
	continue; 
      }
      
      switch(i) {
      case 0: /* grid size */
	if((token = strtok(NULL,"x,; \t:\n"))==NULL) break;
	grid.size[X] = atof(token);
	if((token = strtok(NULL,"x,; \t:\n"))==NULL) break;
	grid.size[Y] = atof(token);
	break;

      case 1: /* grid position */
	if((token = strtok(NULL,"x,; \t:\n"))==NULL) break;
	grid.pos[X] = atof(token);
	if((token = strtok(NULL,"x,; \t:\n"))==NULL) break;
	grid.pos[Y] = atof(token);
	break;

      case 2: /* verbose */
	if((token = strtok(NULL,",; \t:\n"))==NULL) break;
	if(strcmp(token,"on")==0) verbose = 1;
	break;

      case 3: /* input image */
	if((token = strtok(NULL,",; \t:\n"))==NULL) break;
	grid.input_image_name = strdup(token);
	break;

      case 4: /* grid range */
	if((token = strtok(NULL,"x,; \t:\n"))==NULL) break;
	grid.range[X] = atof(token);
	if((token = strtok(NULL,"x,; \t:\n"))==NULL) break;
	grid.range[Y] = atof(token);
	break;
 
      case 5: /* input data */
	if((token = strtok(NULL,",; \t:\n"))==NULL) break;
	grid.input_data_name = strdup(token);
	break;


      }
    }
  }

  if(verbose) {
    fprintf(stderr, "meadow - grid position: (%g, %g)\n", 
	    grid.pos[X], grid.pos[Y]);

    fprintf(stderr, "meadow - grid range: %g x %g\n", 
	    grid.range[X], grid.range[Y]);

    fprintf(stderr, "meadow - grid size: %dx%d\n", 
	    grid.size[X], grid.size[Y]);

    fprintf(stderr, "\nmeadow - specification file processed.\n\n");
  }

  InitializeGrid(&grid);
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
    CSBeginTransmission();

    if(verbose)
      fprintf(stderr, "meadow - start processing data.\n");

    InitializeQueryArray();

    for(i=0; i < grid.size[Y]*grid.size[X]; i++)
      grid.data[i].visits = 0;

    /* process the data */
    while(CSGetData(&master, &module_id, two_modules, &turtle)) {

      ProcessQuery(&grid, master, module_id, &two_modules[0], &turtle);
    }

    DetermineResponse(&grid);

    if(verbose)
      fprintf(stderr, "meadow - data processed.\n");  

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

  fprintf(stderr, "Field process %s initialized.\n", process_name);

  MainLoop();

  FreeFieldStructures();

  fprintf(stderr, "Field process %s exiting.\n", process_name);

  /* should be the last function called */
  CTerminate();

  return 1;
}
