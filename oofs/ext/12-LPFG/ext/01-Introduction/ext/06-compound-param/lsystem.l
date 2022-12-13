#include <lpfgall.h>

#define ANGLE 45

derivation length: 4;

module A(float);
module I(float);

Axiom: SetWidth(3) A(1.0);

A(v) :
{
	produce I(1) 
		SB() Right(ANGLE) A(v) EB() 
		SB() Left(ANGLE)  A(v) EB() 
		I(1) A(v);
} 

I(x) : { produce I(2*x); } 
	
interpretation:

A(v) : { produce IncColor() F(v); }
I(x) : { produce F(x); }
