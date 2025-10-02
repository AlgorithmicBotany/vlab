#include <lpfgall.h>

/* Sample parameter values: 
	STEPS== 4, D==0, R==2
	STEPS==11, D==1, R==1.5
	STEPS==18, D==3, R==1.28
*/

#define WIDTH 3

#define ANGLE 45
#define STEPS 18
#define D 3
#define R 1.28

derivation length: STEPS;

module A(float);
module I(float);

Axiom: SetWidth(WIDTH) A(1.0);

A(v) :
{
	if(v<D)
		produce A(v+1);
	else if(v==D)
		produce
			I(1) 
			SB() Right(ANGLE) A(0) EB() 
			SB() Left(ANGLE)  A(0) EB() 
			I(1) 
			A(v);
} 

I(x) : { produce I(R*x); } 
	
interpretation:

A(v) : { produce IncColor() F(v); }
I(x) : { produce F(x); }		
