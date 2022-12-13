/*
  Environmental process - Chiba (et al.)'s light environment
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI	3.14159265358979323846f
#endif


#include "chiba.h"
#include "sphere.h"
#include "comm_lib.h"


/**** field specific variables ****/
/* leaf clusters */
#define LEAF_ARRAY_SIZE 1000

struct LEAF_UNIT_TYPE {
  int master;
  float pos[3];
  float radius;
  float area;                /* PI*rad^2 */
  float num_params;          /* number of parameters of comm. symbol */
  unsigned long dist;        /* position in L-system string */
  unsigned long ray_signature;
};
typedef struct LEAF_UNIT_TYPE LEAF_UNIT_TYPE;

LEAF_UNIT_TYPE *leaves = NULL;
int num_leaves;
int leaf_array_size;

/* grid */
struct LEAF_LIST_TYPE {
  unsigned int leaf_index;
  struct LEAF_LIST_TYPE *next;
};
typedef struct LEAF_LIST_TYPE LEAF_LIST_TYPE;

struct CELL_TYPE {
  LEAF_LIST_TYPE *list;
};
typedef struct CELL_TYPE CELL_TYPE;


#define X 0
#define Y 1
#define Z 2

struct grid_type {
  int size[3];         /* size of the grid (in nodes) */
  float range[3];      /* size in coordinates */
  float pos[3];        /* position of lower left front corner */
  CELL_TYPE *data;
};
typedef struct grid_type grid_type;

/* light samples */
struct SAMPLE_TYPE {
  float dir[3];
  float intensity;
};
typedef struct SAMPLE_TYPE SAMPLE_TYPE;


SAMPLE_TYPE *samples = NULL;
int num_samples;


/* light sources */
#define MAX_NUM_SOURCES 100

SAMPLE_TYPE sources[MAX_NUM_SOURCES];

int num_sources;


/* prototypes */
CELL_TYPE *GetCell(grid_type *grid, int x, int y, int z);
void FreeFieldStructures(void);


grid_type grid = {0};

char verbose;

float LtoU, transmittance, radius, beam_radius, max_radius;
char estimate_area, use_CIE_formula;

int current_step;

unsigned long ray_signature;

/****************************************************************************/
void Normalize(float *norm)
{
  float len;
  int i;

  /* normalize */
  len = sqrt(norm[0]*norm[0] + norm[1]*norm[1] + norm[2]*norm[2]);

  if(len>0)
    for(i=0;i<3;i++)
      norm[i] /= len;
}

/****************************************************************************/
void FreeGridAndLeaves(void)
{
  int i;
  LEAF_LIST_TYPE *ptr, *ptr2;

  if(grid.data != NULL) {
    for(i=0;i < grid.size[X]*grid.size[Y]*grid.size[Z];i++) {
      /* free the linked list */ 
      ptr = grid.data[i].list;

      while(ptr != NULL) {
	ptr2 = ptr->next;
	free(ptr);
	ptr = ptr2;
      }
    }
    
    free(grid.data);
    grid.data = NULL;
  }

  if(leaves != NULL) {
    free(leaves);
    leaves = NULL;
  }
  num_leaves = 0;
  leaf_array_size = LEAF_ARRAY_SIZE;
}

/****************************************************************************/
void FreeFieldStructures(void)
{
  FreeGridAndLeaves();
 
  if(samples != NULL) {
    free(samples);
    samples = NULL;
  }
  num_samples = 0;
}

/****************************************************************************/
void InitializeLeafArray(void)
{
  if((leaves = (LEAF_UNIT_TYPE*)malloc(leaf_array_size*sizeof(LEAF_UNIT_TYPE)))
     == NULL) {
    fprintf(stderr,"Chiba - cannot allocate memory for leaf array!\n");
    exit(0);
  }
}

/****************************************************************************/
float Distance2(float *p1, float *p2)
{
  int i;
  float vec[3];

  for(i=0;i<3;i++)
    vec[i] = p2[i] - p1[i];

  return vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2];
}

/****************************************************************************/
/* adds a new item to the beginning of a linked list */
void AddToList(LEAF_LIST_TYPE **first, unsigned int item_index)
{
  LEAF_LIST_TYPE *ptr;

  if((ptr = (LEAF_LIST_TYPE *)malloc(sizeof(LEAF_LIST_TYPE))) == NULL) {
    fprintf(stderr,"Chiba - cannot allocate memory for leaf item!\n");
    exit(0);
  }
  
  ptr->next = (*first);
  ptr->leaf_index = item_index;
  *first = ptr;
}

/****************************************************************************/
void InitializeGrid(grid_type *grid)
{
  CELL_TYPE *ptr;
  int x,y,z, c;
  unsigned int lv;
  float range[3][2];
  float pt[3], half, aux;

  if((grid->data = (CELL_TYPE*)
      malloc(grid->size[X]*grid->size[Y]*grid->size[Z]*sizeof(CELL_TYPE)))
     == NULL) {
    fprintf(stderr,"Chiba - cannot allocate memory for grid!\n");
    exit(0);
  }

  half = 0;
  for(c=X;c<=Z;c++) {
    aux = grid->range[c]/(float)grid->size[c];
    half += aux*aux;
  }
  half = 0.5 *sqrt(half);

  if(verbose)
    fprintf(stderr,"Chiba - initializing grid at (%g,%g,%g) of range %g x %g x %g.\n",
	    grid->pos[X],grid->pos[Y],grid->pos[Z],
	    grid->range[X],grid->range[Y],grid->range[Z]);

  /* set all lists to NULL */
  ptr = grid->data;
  for(z=0;z<grid->size[Z];z++)
    for(y=0;y<grid->size[Y];y++)
      for(x=0;x<grid->size[X];x++)
	(ptr++)->list = NULL;
    
  /* for all leaves */
  for(lv=0;lv<(size_t)num_leaves;lv++) {

    /* get the range of voxels possibly intersecting the cluster sphere */
    for(c=X; c<=Z; c++) {
      range[c][0] =
	floor((leaves[lv].pos[c]-leaves[lv].radius-max_radius*beam_radius - grid->pos[c])
	      /grid->range[c] * (float)grid->size[c]) - 1;
      if(range[c][0] < 0)
	range[c][0] = 0;
      
      range[c][1] = 1 +
	floor((leaves[lv].pos[c]+leaves[lv].radius+max_radius*beam_radius - grid->pos[c])
	      /grid->range[c] * (float)grid->size[c]);
      if(range[c][1] >= grid->size[c])
	range[c][1] = grid->size[c]-1;
    }

    /* for all nodes in the range */
    for(z=range[Z][0]; z<=range[Z][1]; z++) {
      pt[Z] = grid->pos[Z] + ((float)z+0.5)/(float)grid->size[Z]*
	grid->range[Z];

      for(y=range[Y][0]; y<=range[Y][1]; y++) {
	pt[Y] = grid->pos[Y] + ((float)y+0.5)/(float)grid->size[Y]*
	  grid->range[Y];
	
	for(x=range[X][0]; x<=range[X][1]; x++) {
	  /* pt is the voxel's centre */
	  pt[X] = grid->pos[X] + ((float)x+0.5)/(float)grid->size[X]*
	    grid->range[X];
	  
	  if(Distance2(pt,leaves[lv].pos) <= 
	     (leaves[lv].radius + max_radius*beam_radius + half)*
	     (leaves[lv].radius + max_radius*beam_radius + half))
	    AddToList(&(GetCell(grid,x,y,z)->list), lv);
	}
      }
    }
  }
  if(verbose)
    fprintf(stderr,"Chiba - grid initialized.\n");
}

/****************************************************************************/
CELL_TYPE *GetCell(grid_type *grid, int x, int y, int z)
{
  if(x < 0)  x = 0;
  if(x >= grid->size[X]) x = grid->size[X] - 1;

  if(y < 0)  y = 0;
  if(y >= grid->size[Y]) y = grid->size[Y] - 1;

  if(z < 0)  z = 0;
  if(z >= grid->size[Z]) z = grid->size[Z] - 1;

  return grid->data + z*grid->size[Y]*grid->size[X] + y*grid->size[X] + x;
}


/****************************************************************************/
void StoreLeafCluster(grid_type *grid, int master,
		      unsigned long module_id,
		      Cmodule_type *two_modules, CTURTLE *tu)
{
  int i;
  float aux;

  if((two_modules[0].num_params != 1)&&(two_modules[0].num_params < 4)) {
    if(verbose)
      fprintf(stderr,"Chiba - one or four parameters for ?E required!\n");
    return; 
  }

  if(tu->positionC < 3) {
    fprintf(stderr,
	    "Chiba - turtle position wasn't sent to the environment.\n");
    return;
  }

  if(num_leaves >= leaf_array_size) {
    /* rellocate the array */
    leaf_array_size *= 2;
    if((leaves = (LEAF_UNIT_TYPE*)
	realloc(leaves, leaf_array_size*sizeof(LEAF_UNIT_TYPE))) == NULL) {
      fprintf(stderr,"Chiba - cannot reallocate memory for leaf array!\n");
      exit(0);
    }
    if(verbose)
      fprintf(stderr,"Chiba - leaf array reallocated to size %d.\n",
	      leaf_array_size);
  }

  leaves[num_leaves].num_params = two_modules[0].num_params;
  leaves[num_leaves].dist = module_id;
  leaves[num_leaves].master = master;

  if(two_modules[0].params[0].value>0)
    leaves[num_leaves].radius = two_modules[0].params[0].value;
  else
    leaves[num_leaves].radius = radius;
    
  leaves[num_leaves].area = (M_PI*leaves[num_leaves].radius*
			     leaves[num_leaves].radius);
  
  for(i=X;i<=Z;i++) {
    leaves[num_leaves].pos[i] = tu->position[i];
  }

  /* update the grid so it encloses all leaf clusters */
  if(num_leaves == 0) {
    /* first leaf cluster */
    for(i=X;i<=Z;i++) {
      grid->pos[i] = tu->position[i]-leaves[num_leaves].radius;
      grid->range[i] = 2*leaves[num_leaves].radius;
    }
    max_radius = leaves[num_leaves].radius;
  }
  else {
    for(i=X;i<=Z;i++) {
      if((aux = grid->pos[i] - (tu->position[i]-leaves[num_leaves].radius)) 
	 > 0) {
	grid->pos[i] -= aux;
	grid->range[i] += aux;
      }
      
      if((aux = (tu->position[i]+leaves[num_leaves].radius)-
	  (grid->pos[i]+grid->range[i])) > 0) {
	grid->range[i] += aux;
      }
    }
    
    if(leaves[num_leaves].radius > max_radius)
      max_radius = leaves[num_leaves].radius;
  }

  num_leaves++;
}

/****************************************************************************/
float ComputeCoveredArea(float *pt, float *dir,
			 LEAF_UNIT_TYPE *leaf,
			 float original_radius)
{
  float vec[3], aux, d, r1,r2,r1s,r2s, y,x1,x2, area;
  int c;

  for(c=X;c<=Z;c++)
    vec[c] = pt[c] - leaf->pos[c];   
  
  aux = dir[0]*vec[0] + dir[1]*vec[1] + dir[2]*vec[2];  /* dir.(pt-C) */

  if(aux>0)
    /* disk in C is father away from the source than the tested leaf disk */
    return 0;

  /* distance of C from the ray: d^2 = (pt-C).(pt-C) - (dir.(pt-C))^2*/ 
  if((d = vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2] - aux*aux) < 
     (leaf->radius + original_radius)*(leaf->radius + original_radius)) { 
    /* there is an intersection */

    /* r1 bigger radius, r2 smaller one */
    if(leaf->radius < original_radius) {
      r1 = original_radius;
      r2 = leaf->radius;
    }
    else {
      r2 = original_radius;
      r1 = leaf->radius;
    }
    
    /* square radii */
    r2s = r2*r2;
    r1s = r1*r1;

    /* distance of centers */
    d = sqrt(d);

    if(d+r2<=r1) {
      /* smaller disc is inside the bigger one */
      area = M_PI * r2s;
    }
    else {
      x1 = (r1s + d*d - r2s)/(2*d);
      x2 = (r2s + d*d - r1s)/(2*d);
      
      y = sqrt(r2s - x2*x2);

      if(verbose) 
	fprintf(stderr,"Chiba - x1=%g, x2=%g, y=%g\n", x1,x2,y);
      
      area = r1s*acos(x1/r1) + r2s*acos(x2/r2) - y*d;
    }
    if(verbose) 
      fprintf(stderr,"Chiba - d=%g, r1=%g, r2=%g, area=%g\n", d,r1,r2,area);
    
    return area;
  }
  /* otherwise return 0 */
  return 0;
}

/****************************************************************************/
float IsClusterIntersection(float *pt, float *dir,
			    LEAF_UNIT_TYPE *leaf,
			    float oradius)
{
  float vec[3], aux, d;
  int c;

  for(c=X;c<=Z;c++)
    vec[c] = pt[c] - leaf->pos[c];   
  
  aux = dir[0]*vec[0] + dir[1]*vec[1] + dir[2]*vec[2];  /* dir.(pt-C) */

  /* discriminant/4 = aux^2 - (pt-C).(pt-C) + rad^2 */ 
  if((d = aux*aux - (vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2]) + 
      (leaf->radius + oradius*beam_radius)*
      (leaf->radius + oradius*beam_radius)) >=0) 
    /* there is an intersection */
    if(-aux+(d=sqrt(d)) >0) {
      /* and is behind pt */
      
      if(-aux-d >=0)
	return 2*d;
      else
	return -aux+d;
    }
  
  /* otherwise return 0 */
  return 0;
}

/****************************************************************************/
float IntensityFromSample(LEAF_UNIT_TYPE *leaf, SAMPLE_TYPE *sample, 
			  grid_type *grid)
{
  float pt[3], node_size[3], middle[3];
  float reduction, red, aux, smallest;
  LEAF_LIST_TYPE *ptr;
  int i,c, node[3], ind, cell_step[3];
  CELL_TYPE *cell;

  if(sample -> intensity == 0)
    return 0.0;

  cell_step[X] = 1;
  cell_step[Y] = grid->size[X];
  cell_step[Z] = grid->size[X] * grid->size[Y];

  for(c=X;c<=Z;c++) {
    /* current point in a voxel */
    pt[c] = leaf->pos[c];
    
    /* half size of a node */
    node_size[c] = 0.5*grid->range[c]/(float)grid->size[c];
    
    /* adjusted according to the sample direction */
    if(sample->dir[c]<0) {
      node_size[c] = -node_size[c];
      cell_step[c] = -cell_step[c];
    }
  }

  for(c=X;c<=Z;c++) {
    /* initial voxel */
    node[c] = floor((pt[c] - grid->pos[c])
		    /grid->range[c] * (float)grid->size[c]);

    /* voxel center */
    middle[c] = ((float)node[c]+0.5)/(float)grid->size[c]*grid->range[c];
  }
  
  cell = GetCell(grid, node[X], node[Y], node[Z]);

  reduction = 1;
  red = transmittance;

  /* increase the current ray signature */
  if(++ray_signature == 0) {
    ray_signature = 1;

    for(i=0;i<num_leaves;i++)
      leaves[i].ray_signature = 0;
  }
  leaf->ray_signature = ray_signature;

  for(;;) {
    /* go through the list associated with the node and check for 
       intersection with each sphere */
    ptr = cell->list;
    
    while(ptr != NULL) {
      if(leaves[ptr->leaf_index].ray_signature != ray_signature) {
	/* make sure that the direction is of unit lenght! */
	if(estimate_area) {
	  if((aux = ComputeCoveredArea(pt, sample->dir, 
				       leaves + ptr->leaf_index,
				       leaf->radius)) > 0) {
	    leaves[ptr->leaf_index].ray_signature = ray_signature;
	    /* when covered area ratio is 0, multiply by 1, when it is 1,
	       multiply by 'red' */
	    reduction *= 1 - (1-red)*aux/leaf->area;
	  }
	}
	else
	  if((aux = IsClusterIntersection(pt, sample->dir, 
					  leaves + ptr->leaf_index,
					  leaf->radius)) > 0) {
	    leaves[ptr->leaf_index].ray_signature = ray_signature;
	    /* if intersects, multiply by the reduction factor */
	    reduction *= pow(red,aux/(2.0*(leaves[ptr->leaf_index].radius +
					   beam_radius*leaf->radius)));
	  }
      }
      ptr = ptr->next;
    }
    
    /* determine the next node intersected by the ray */
    
    /* find the intersected face */
    smallest = fabs(middle[X]+node_size[X] - pt[X]);
    ind = X;
    
    for(c=Y;c<=Z;c++)
      if(fabs((aux = (middle[c]+node_size[c] - pt[c]))*sample->dir[ind]) < 
	 fabs(smallest * sample->dir[c])) {
	smallest = aux;
	ind = c;
      }

    /* get the next middle point */
    middle[ind] += 2*node_size[ind];

    cell += cell_step[ind];

    if(sample->dir[ind] > 0)
      node[ind]++;
    else
      node[ind]--;

    /* if the ray leaves the grid, go out of the loop */
    if((node[ind] < 0) || (node[ind] >= grid->size[ind])) {
      return sample->intensity * reduction;
    }
  }
}


/****************************************************************************/
void DetermineResponse(grid_type *grid)
{
  int lv,src,c;
  float sum, a;
  float br_dir[3];     /* brightest direction */
  Cmodule_type comm_symbol;

  if(verbose) {  
    fprintf(stderr, "Chiba - start determining response for each out of %d"
	    " leaves.\n", num_leaves);
  }

  ray_signature = 0;
  for(lv=0; lv<num_leaves; lv++)
    leaves[lv].ray_signature = 0;

  for(c=0;c<4;c++)
    comm_symbol.params[c].set = 1;

  for(c=4;c<CMAXPARAMS;c++)
    comm_symbol.params[c].set = 0;

  /* for all leaves */
  for(lv=0;lv<num_leaves;lv++) {
    comm_symbol.num_params = leaves[lv].num_params;

    sum = 0;
    
    for(c=X;c<=Z;c++)
      br_dir[c] = 0;

      
    if(num_sources>0)
      /* for all ligth sources */
      for(src=0;src<num_sources;src++) {
	a = IntensityFromSample(leaves+lv, sources+src, grid);
	sum += a;
	
	for(c=X;c<=Z;c++)
	  br_dir[c] += a*sources[src].dir[c];
      }
    else
      /* for all samples */
      for(src=0;src<num_samples;src++) {
	a = IntensityFromSample(leaves+lv, samples+src, grid);
	sum += a;
	
	for(c=X;c<=Z;c++)
	  br_dir[c] += a*samples[src].dir[c];
      }

    /* set resulting percentage of the amount of sunlight perceived */
    comm_symbol.params[0].value = sum;

    /* brightest direction, if required */
    if(comm_symbol.num_params >=4) {
      Normalize(br_dir);
      for(c=X;c<=Z;c++)
	comm_symbol.params[1+c].value = br_dir[c];
    }

    CSSendData(leaves[lv].master, leaves[lv].dist, &comm_symbol);

    if(verbose) {
      fprintf(stderr, "Chiba - perceived sunlight: %g.\n",
	      comm_symbol.params[0].value);

      if(comm_symbol.num_params >=4)
	fprintf(stderr, "Chiba - brightest direction: (%g,%g,%g)\n",
		br_dir[0], br_dir[1], br_dir[2]);
    }
  }
}

/***************************************************************************/
/* The CIE Standard sky luminance function.
   Only the first angle (phi - angle from zenith) is used 
   parameter sample is ignored. 
   The lower the angle[, the higher the value of luminance */
double OvercastSky(double cosangle)
{

  return (1+cosangle)/3.0 * (8.6*cosangle + 0.123)/0.203;
}

/****************************************************************************/
void GenerateSamples(void)
{
  int i,c, polygonization_level, noSphereTris, total_up;
  sphtri *unitSphere;

  /* closest higher power of four for num_samples/8 */
  polygonization_level = ceil(log((double)num_samples/8.0)/log((double)4.0))+1;

  noSphereTris = (int)floor(0.5+8.0*pow(4.0,(double)(polygonization_level-1)));

  if((unitSphere = (sphtri *)malloc(sizeof(sphtri)*noSphereTris)) ==NULL) {
    fprintf(stderr,"Chiba - cannot allocate array for triangles.\n");
    exit(0);
  }

  makePolySphere(unitSphere, polygonization_level);

  if(verbose)
     fprintf(stderr,"Chiba - %d samples (%d required).\n",
	     noSphereTris, num_samples);

  num_samples = noSphereTris;
  
  if((samples = (SAMPLE_TYPE*)malloc(num_samples * sizeof(SAMPLE_TYPE)))
     == NULL) {
    fprintf(stderr,"Chiba - cannot allocate array for samples.\n");
    exit(0);
  }
  
  total_up = 0; /* number of points up */

  if(use_CIE_formula) {
    float total_intensity = 0;

    for(i=0;i<num_samples;i++) {
      /* take triangle centres */
      for(c=X;c<=Z;c++)
	samples[i].dir[c] = (unitSphere[i][0][c] + unitSphere[i][1][c] +
			     unitSphere[i][2][c])/3.0;
      
      Normalize(samples[i].dir);  /* necessary for IsClusterIntersection() */
      
      if(samples[i].dir[1] <=0) samples[i].intensity = 0.0;
      else
	total_intensity += samples[i].intensity = 
	  OvercastSky(samples[i].dir[1]/
		      sqrt(samples[i].dir[0]*samples[i].dir[0] +
			   samples[i].dir[1]*samples[i].dir[1]));
    }
    
    for(i=0;i<num_samples;i++) 
      samples[i].intensity /= total_intensity;
  }
  else {
    for(i=0;i<num_samples;i++) {
      /* take triangle centres */
      for(c=X;c<=Z;c++)
	samples[i].dir[c] = (unitSphere[i][0][c] + unitSphere[i][1][c] +
			     unitSphere[i][2][c])/3.0;
      
      Normalize(samples[i].dir);  /* necessary for IsClusterIntersection() */
      
      if(samples[i].dir[1] >=0) total_up++;
    }
    
    /* distribute intensities keeping LtoU ratio in mind */
    for(i=0;i<num_samples;i++) 
      if(samples[i].dir[1] >=0) 
	/* up */
	samples[i].intensity = 1.0/(float)total_up/(1.0+LtoU);
      else
	/* down */
	samples[i].intensity = LtoU/(float)(num_samples - total_up)/(1.0+LtoU);
  }

  if(verbose) {
    for(i=0;i<num_samples;i++)
      fprintf(stderr,"Chiba - %d. direction (%g,%g,%g) with intensity %g.\n",
	      i+1, samples[i].dir[0], samples[i].dir[1], samples[i].dir[2],
	      samples[i].intensity);
  }

  free(unitSphere);
}

/****************************************************************************/
void NormalizeSources(void)
{
  int i;
  float total;
  
  total = 0;
  for(i=0;i<num_sources;i++)
    total += sources[i].intensity;

  for(i=0;i<num_sources;i++) {
    sources[i].intensity /= total;
    Normalize(sources[i].dir);
  }

  if(verbose) {
    fprintf(stderr,"Chiba - %d light sources.\n", num_sources);

    for(i=0;i<num_sources;i++)
      fprintf(stderr,"Chiba - %d. direction (%g,%g,%g) with intensity %g.\n",
	      i+1, sources[i].dir[0], sources[i].dir[1], sources[i].dir[2],
	      sources[i].intensity);
  } 
}

/****************************************************************************/
void ProcessArguments(int argc, char **argv)
{
  FILE *fp;
  int i;
  char *keywords[] = {
    "grid size",            /*  0 */
    "verbose",              /*  1 */
    "lower to upper ratio", /*  2 */
    "transmittance",        /*  3 */
    "number of samples",    /*  4 */
    "radius",               /*  5 */
    "beam radius",          /*  6 */
    "source direction",     /*  7 */
    "estimate intersection area", /* 8 */
    "use CIE formula",      /* 9 */
    NULL                    /* the last item must be NULL! */
  };
  char *token, input_line[255];

  FreeFieldStructures();

  /* defaults */
  verbose = 0;
  
  grid.size[X] = 1;
  grid.size[Y] = 1;
  grid.size[Z] = 1;

  LtoU = 0.7;
  transmittance = 0.6;
  radius = 25;
  beam_radius = 0;
  estimate_area = 0;
  use_CIE_formula = 0;

  num_samples = 20;
  num_sources = 0;

  if(argc == 1) {
    printf("Chiba - not enough arguments!\n"
	   "USAGE: Chiba -e environment_file light_spec_file\n");

    exit(0);
  }

  /* read in environment file */
  if((fp = fopen(argv[1],"r")) == NULL)
    fprintf(stderr, "Chiba - cannot open specification file %s.\n",argv[1]);
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
		"Chiba - unknown directive %s in the specification file.\n",
		token);
	continue; 
      }
      
      switch(i) {
      case 0: /* grid size */
	if((token = strtok(NULL,"x,; \t:\n"))==NULL) break;
	grid.size[X] = atof(token);
	if((token = strtok(NULL,"x,; \t:\n"))==NULL) break;
	grid.size[Y] = atof(token);
	if((token = strtok(NULL,"x,; \t:\n"))==NULL) break;
	grid.size[Z] = atof(token);
	break;

      case 1: /* verbose */
	if((token = strtok(NULL,"x,; \t:\n"))==NULL) break;
	if(strcmp(token,"on")==0) verbose = 1;
	break;

      case 2: /* Lower to Upper intensity ratio */
	if((token = strtok(NULL,",; \t:\n"))==NULL) break;
	LtoU = atof(token);
	break;

      case 3: /* transmittance */
	if((token = strtok(NULL,",; \t:\n"))==NULL) break;
	transmittance = atof(token);
	break;

      case 4: /* num samples */
	if((token = strtok(NULL,"x,; \t:\n"))==NULL) break;
	num_samples = atoi(token);
	break;

      case 5: /* radius */
	if((token = strtok(NULL,",; \t:\n"))==NULL) break;
	radius = atof(token);
	break;

      case 6: /* beam radius */
	if((token = strtok(NULL,",; \t:\n"))==NULL) break;
	beam_radius = atof(token);
	break;

      case 7: /* source */
	if(num_sources >= MAX_NUM_SOURCES) {
	  fprintf(stderr,"Takenaka - too many light sources. Ignored.\n");
	  break;
	}

	/* default */
	for(i=0;i<3;i++)
	  sources[num_sources].dir[i] = 0;
	sources[num_sources].intensity = 1; 

	for(i=0;i<3;i++) {
	  if((token = strtok(NULL,",; \t:\n"))==NULL) break;
	  sources[num_sources].dir[i] = atof(token); 
	}
	
	if((token = strtok(NULL,"x,; \t:\n"))==NULL) break;
	sources[num_sources].intensity = atof(token);

	num_sources++;
	break;

      case 8: /* estimate intersection area */
	if((token = strtok(NULL,"x,; \t:\n"))==NULL) break;
	if(strcmp(token,"on")==0) estimate_area = 1;
	break;

      case 9: /* use CIE formula */
	if((token = strtok(NULL,"x,; \t:\n"))==NULL) break;
	if(strcmp(token,"yes")==0) use_CIE_formula = 1;
	break;

      }
    }
  }

  if(verbose) {
    fprintf(stderr, "Chiba - grid size: %dx%dx%d\n", 
	    grid.size[X], grid.size[Y], grid.size[Z]);

    fprintf(stderr, "Chiba - LtoU: %g\n", 
	    LtoU);

    fprintf(stderr, "Chiba - radius: %g\n", 
	    radius);

    fprintf(stderr, "Chiba - transmittance: %g\n", 
	    transmittance);

    fprintf(stderr, "\nChiba - specification file processed.\n\n");
  }
  
  if(num_sources>0)
    /* normalize light sources */
    NormalizeSources();
  else
    /* generate samples */
    GenerateSamples();

  if(estimate_area)
    beam_radius = 1;
}

/****************************************************************************/
void MainLoop(void)
{
  Cmodule_type two_modules[2];
  unsigned long module_id;
  CTURTLE turtle;
  int master;
  int in;

  /* infinite loop - until signal 'exit' comes */
  for(;;) {
    in = 0;

    CSBeginTransmission();

    if(verbose)
      fprintf(stderr, "Chiba - start processing data.\n");  

    FreeGridAndLeaves();
    InitializeLeafArray();

    /* process the data */
    while(CSGetData(&master, &module_id, two_modules, &turtle)) {
      in = 1;

      StoreLeafCluster(&grid, master, module_id, two_modules, &turtle);
    }
    /* doesn't send anything to cpfg */

    if(in) {
      InitializeGrid(&grid);
    
      DetermineResponse(&grid);
    }

    if(verbose)
      fprintf(stderr, "Chiba - data processed.\n");  

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

