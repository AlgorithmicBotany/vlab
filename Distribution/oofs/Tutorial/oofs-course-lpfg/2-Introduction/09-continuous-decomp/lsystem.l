#include <lpfgall.h>
#include <math.h>

const int STEPS = 400;
const float dt = 0.02f;

const float WIDTH = 3.0f;

const float D = 2.0f;
const float R = 1.5f;
const float ANGLE = 44.0f;

const float RATE = pow(R,dt);

derivation length: STEPS;

module A(float);
module I(float);
module M(float);

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

	produce
		I(x) 
		SB() Right(ANGLE) A(t) EB()
		SB() Left(ANGLE) A(t) EB()
		I(x);
}

interpretation:

A(t) : { produce IncColor() F(t); }

I(x) : { produce F(x); }		
