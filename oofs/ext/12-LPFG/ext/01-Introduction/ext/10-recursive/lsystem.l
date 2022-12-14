#include <lpfgall.h>
#include <math.h>

#define WIDTH 	3

#define	T	8	// age of the structure
#define D	2	// delay in producing laterals
#define R	1.5	// growth rate
#define ANGLE	45	// branching angle

derivation length: 1;

module A(float);
module I(float);
module M(float);

Axiom: SetWidth(WIDTH) A(T);

interpretation:
maximum depth: 100;

A(t) : 
{ 
	if(t<D)
		produce IncColor() F(t);
	else
		produce M(t-D) A(t-1);
}

M(t) :
{
	float x = 0.5*pow(R, t);

	produce
		I(x)
		SB() Right(ANGLE) A(t) EB()
		SB() Left(ANGLE) A(t) EB()
		I(x);
}

I(x) : { produce F(x); }		
