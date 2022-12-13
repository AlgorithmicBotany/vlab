#include <lpfgall.h>
#include <math.h>

#define STEPS 400
#define dt 0.02

#define D 2
#define R 1.5

#define RATE pow(R,dt)

derivation length: STEPS;

module A(float);
module I(float);
module M(float);

Axiom: SetWidth(2.0) A(0);

production:

A(t): { produce A(t+dt); }

I(x): { produce I(x*RATE); }

decomposition:
maximum depth: 2;

A(t):
{
	if(t>=D)
		produce RollR(90) M(t-D) A(t-1);
}

M(t):

{
	float x = 0.5 * pow(R, t);

	produce
		I(x) 
		SB() Left(45)  A(t) EB()
		SB() Right(45) A(t) EB()
		I(x);
}

interpretation:

A(t): { produce IncColor() F(t); }

I(x): { produce F(x); }
