#include <lpfgall.h>

#define ANGLE 45

derivation length: 4;

module A(float);
module I(float);

Axiom: SetWidth(3) A(1.0);

A(v) :
{
	produce 
		I(v) 
		SB() Right(ANGLE) A(v) EB() 
		SB() Left(ANGLE)  A(v) EB() 
		I(v) 
		A(v);
} 

I(v) : { produce I(v) I(v); } 

interpretation:

A(v) : { produce IncColor() F(v); }
I(v) : { produce F(v); }		
