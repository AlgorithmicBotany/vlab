#include <lpfgall.h>
#include <math.h>

#define STEPS 300
#define dt 0.02

#define D 2
#define R 1.5

derivation length: STEPS;

module A(float);
module I(float);
module M(float);

float rate;

Start: {
	rate = pow(R,dt);
}

Axiom: SetWidth(0.5) A(0);

production:

A(t): { produce A(t+dt); }

I(x): { produce I(x*rate); }

decomposition:
maximum depth: 2;

A(t): 
{ 
	if(t>=D)
		produce M(t-D) A(t-1);
}

M(t):
{
	float x = 0.5 * pow(R, t);

	produce
		I(x) RollR(90)
		SB() Left(45)  A(t) EB()
		SB() Right(45) A(t) EB()
		I(x);
}

interpretation:

A(t): { produce IncColor() F(t); }

I(x): { produce F(x); }
