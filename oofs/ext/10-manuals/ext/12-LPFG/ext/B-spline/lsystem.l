#include <lpfgall.h>

#define NUMBER_OF_SUBDIVISIONS 3
#define DEGREE 3

V2f v1(0, 0), v2(0, 1), v3(1, 1), v4(1, 0);

module P(V2f);	// Vertex
module E();		// Edge

int n;			// Step counter

#define SUBDIVIDE 1
#define AVERAGE 2

Start: {n=0;}
StartEach: { UseGroup ((n++ % DEGREE == 0)? SUBDIVIDE : AVERAGE); }

ring L-system: 1;

derivation length: NUMBER_OF_SUBDIVISIONS*DEGREE;

Axiom: P(v1) E() P(v2) E() P(v3) E() P(v4) E();

group SUBDIVIDE:
P(vl) < E() > P(vr) : { produce E() P(0.5*vl+0.5*vr) E() ; }

group AVERAGE:
		P(v) :	      { produce E() ; }
P(vl) < E() > P(vr) : { produce P(0.5*vl+0.5*vr) ;}

interpretation:
group 0: 		// Visualize of all groups 	
P(v) :  { produce MoveTo2f(v) SetColor(1) Sphere(0.02); }
P(vl) < E() > P(vr) : { produce SetColor(2) SetWidth(2) Line2f(vl,vr); }
