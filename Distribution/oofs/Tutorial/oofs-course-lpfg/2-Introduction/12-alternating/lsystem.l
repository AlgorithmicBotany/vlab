#include <lpfgall.h>
#include <math.h>

const float WIDTH = 2.0f;

const float ANGLE = 45.0f;
const int STEPS = 800;
const float dt = 0.02f;
const float D = 2.5f;
const float R = 1.3f;
const float RATE = pow(R, dt);

module A(float);
module B(float);
module M(float);
module N(float);
module I(float);

derivation length: STEPS;

Axiom: SetWidth(WIDTH) A(0);

A(t) : { produce A(t+dt); }
B(t) : { produce B(t+dt); }
I(x) : { produce I(x*RATE); }

decomposition:
maximum depth: 2;

A(t) : 
{ 
	if(t>=D)
		produce M(t-D) B(t-1);
}

B(t) : 
{
	if(t>=D)
		produce N(t-D) A(t-1);
}

M(t) : 
{
	float x = 0.5*pow(R,t);

	produce
		I(x)
		SB() Right(ANGLE) A(t) EB()
		I(x);	 
}

N(t) : 
{ 
	float x = 0.5*pow(R,t);
	
	produce
		I(x)
		SB() Left(ANGLE) B(t) EB()
		I(x);	
}

interpretation:

A(t) : { produce IncColor() F(t); }
B(t) : { produce IncColor() IncColor() F(t); }
I(x) : { produce F(x); }		

