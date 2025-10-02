#include <lpfgall.h>

#define STEPS 5

V2f v1(0, 0), v2(0, 1), v3(1, 1), v4(1, 0);

module P(V2tf);	/* A point */
module L(V2tf,V2tf) ; /* A line */

int n;

Start: {n=0;}
EndEach: {++n;}

derivation length: STEPS;

ring L-system: 1;

Axiom: P(v1) P(v2) P(v3) P(v4) ;

/* Produce lines for visualization purposes */

P(v) > P(vr) : 
{
 	if (n == STEPS-1) produce P(v) L(v,vr) ;
}

/* The subdivision rule */

P(vl) < P(v) > P(vr) :
{
	produce P(0.25*vl+0.75*v) P(0.25*vr+0.75*v);
}


interpretation:

P(v) :
{
	produce MoveTo2f(v) Circle(0.01) ; 
}

L(v1,v2) :
{
	produce Line2f(v1,v2) ; 
}
