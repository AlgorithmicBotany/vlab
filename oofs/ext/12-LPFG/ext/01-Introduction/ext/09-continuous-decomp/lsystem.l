#include <lpfgall.h>

#define STEPS 400
#define dt 0.02

#define WIDTH 3

#define D 2
#define R 1.5
#define ANGLE 45

#define RATE (pow(R,dt))

module A(float);
module I(float);
module M(float);

derivation length: STEPS;

Axiom: SetWidth(WIDTH) A(0.0);

A(t): { produce A(t+dt); }
I(x): { produce I(x*RATE); }

decomposition:
maximum depth: 2;

A(t):
{
	if(t>=D)
		produce M(t-D) A(t-1);
}

M(t):
{
	float x=0.5*pow(R,t);

	produce I(x) 
		SB() Right(ANGLE) A(t) EB()
		SB() Left(ANGLE) A(t) EB()
		I(x);
}

interpretation:

A(t) : { produce IncColor() F(t); }
I(x) : { produce F(x); }		
