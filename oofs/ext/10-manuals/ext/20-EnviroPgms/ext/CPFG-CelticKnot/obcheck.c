/*
  Environment defines obstacles for a knot pattern.  
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "comm_lib.h"

/* Field specific declarations and variables */

/* A line segment, given by coordinates of the endpoints */

struct segment {
	float xa, ya, xb, yb;
}; 

typedef struct segment SEGMENT;

/* Maximum number of obstacles in a field */

#define MAXOBST 100

SEGMENT obst[MAXOBST]; /* Array of obstacles */
SEGMENT turtleline; /* Segment between two turtle positions */
						
int count; /* the actual number of obstacles */

/* Function prototypes */
int intersect(SEGMENT *line1, SEGMENT *line2);


/****************************************************************************/
/* Incoming data are: 
     communication symbol plus next symbol,
     turtle (also which turtle parameters and how many were sent). 
   Returns 0 when no info is sent back, 1 when there are some changes. 
*/
int AnswerQuery(Cmodule_type *two_modules, CTURTLE *turtle)
{
  int c;
	int i;
  /* determine response - by setting parameters of communication symbol
     directly in two_modules[0]. */

  /* only some parameters will be sent back, all others have 'set' equal to 0
   */
  for(c=0;c<two_modules[0].num_params;c++)
    two_modules[0].params[c].set = 0;

  /* return 0 if the turtle does not intersect any obstacle, 1 if it does */
  if(two_modules[0].num_params > 0) {
	/* intialization of the turtleline */
	if (two_modules[0].params[0].value == 0) {
		turtleline.xa = turtle->position[0];
		turtleline.ya = turtle->position[1];
	} else {
		two_modules[0].params[0].set = 1;
		turtleline.xb = turtle->position[0];
		turtleline.yb = turtle->position[1];
		for(i=0; i<count ; i++) {
			if (intersect(&turtleline,&obst[i])) {
				two_modules[0].params[0].value = 1;
				return 1;
			};
		};
		two_modules[0].params[0].value = 0;
		turtleline.xa = turtleline.xb;
		turtleline.ya = turtleline.yb;
		};
	};
  return 1;
}

 
/****************************************************************************/
/* process additional arguments of the field process, usually a text */
/* specification file. Initialize field structures accordingly. */
void ProcessArguments(int argc, char **argv)
{
	FILE *fp_in;
	float dummy; 

	if (argc != 2) {
		fprintf(stderr, "Usage: %s obstacles.dat\n", argv[0]);
		exit(1);
	};

	if((fp_in = fopen(argv[1],"r")) == NULL) {
		fprintf(stderr, "%s: Can't open %s\n", argv[0], argv[1]);
		exit(1);
	};
	
	fprintf(stderr, "Reading %s\n", argv[1]);
	fprintf(stderr, "argc = %d\n", argc);

	fscanf(fp_in, "line width: %f\n", &dummy);
        fscanf(fp_in, "coordinates:\n");
	count=0;
	while (fscanf(fp_in, "%f %f %f %f", 
		&(obst[count].xa), &(obst[count].ya),
		&(obst[count].xb),&(obst[count].yb)) == 4) {
			fprintf(stderr, "%d %f %f %f %f \n", 
				count, obst[count].xa, obst[count].ya,
				obst[count].xb, obst[count].yb);
	 	count++;
	};
	fprintf(stderr, "Total number of obstacles: %d \n", count);
}

/****************************************************************************/
int main(int argc, char **argv)
{

  /* initialize the communication as the very first thing */
  CSInitialize(&argc, &argv);

  ProcessArguments(argc,argv);

  /* all queries can be answered immediately - provided
     function CSMainLoop(). */
  CSMainLoop(AnswerQuery);

  /* should be the last function called */
  CTerminate();

  return 1;
}

int intersect(line1,line2)
SEGMENT *line1, *line2;
{
	float val11, val12, val21, val22;
	float side();
	float epsilon=0.1;

	val11 = side(line1->xa, line1->ya, line2);
	val12 = side(line1->xb, line1->yb, line2);
	val21 = side(line2->xa, line2->ya, line1);
	val22 = side(line2->xb, line2->yb, line1);

	if ((val11 * val12 <= epsilon) && (val21 * val22 <= epsilon))
		return(1);
	else
		return(0);
}
	
float side (x, y, line)
float x, y;
SEGMENT *line;
{
	return((x-line->xa)*(line->yb-line->ya) - (y-line->ya)*(line->xb-line->xa));
}
