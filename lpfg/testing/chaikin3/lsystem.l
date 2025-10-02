#include <lpfgall.h>

V2f v1(0, 0), v2(0, 1), v3(1, 1), v4(1, 0);

module P(V2tf);	/* A point */
module L() ; /* A line */

derivation length: 3;

ignore: L;
ring L-system: 1;

Axiom: L P(v1) L P(v2) L P(v3) L P(v4) ;

//P(vl) < P(v) > P(vr) :
P(v) :
{
	V2tf vl, vr;
	if (InLeftContext(P(vl)) && InRightContext(P(vr)))
		produce P(0.25*vl+0.75*v) L  P(0.25*vr+0.75*v);
}

interpretation:

P(v) :
{
	produce MoveTo2f(v) Circle(0.01) ; 
}

P(vl) < L() > P(vr) :
{
	produce Line2f(vl,vr) ; 
}
