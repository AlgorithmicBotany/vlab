#include <lpfgall.h>
#include <math.h>

const float WIDTH = 3.0f;

const float T = 8.0f;
const float D = 2.0f;
const float R = 1.5f;
const float ANGLE = 44.0f;

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
