#include <lpfgall.h>
#include <math.h>

const float dt = 0.02f;
const int STEPS = 400;

const float WIDTH = 3.0f;

const float D = 2.0f;
const float R = 1.5f;
const float ANGLE = 45.0f;

derivation length: STEPS;

module A(float);
module I(float);

Axiom: SetWidth(WIDTH) A(0.0);

A(t) :
{
	float t1 = t+dt;

	if(t1<D)
		produce A(t1);
	else
	{
		float t2 = t1-D;
		float t3 = t1-1.0;
	
		produce
			I(0.5*pow(R, t2))
			SB() Right(ANGLE) A(t2) EB()
			SB() Left(ANGLE) A(t2) EB()
			I(0.5*pow(R, t2))
			A(t3);
	}
} 

I(x) : { produce I(x*pow(R, dt)); } 
	
interpretation:

A(t) : { produce IncColor() F(t); }

I(x) : { produce F(x); }		
