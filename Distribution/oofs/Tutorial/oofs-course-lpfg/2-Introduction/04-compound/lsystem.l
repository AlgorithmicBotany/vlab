#include <lpfgall.h>

#define ANGLE 45

derivation length: 4;

module A(float);

Axiom: SetWidth(3) A(1.0);

A(v) :
{
	produce 		
		F(v) 
		SB() Right(ANGLE) A(v) EB() 
		SB() Left(ANGLE) A(v) EB() 
		F(v) 
		A(v);
} 

F(v) :
{
	produce 
		F(v) F(v);
} 
					
