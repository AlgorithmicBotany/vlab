#include <lpfgall.h>

#define LENGTH 1
#define WIDTH 2


derivation length: 15;

Axiom: 	SetWidth(WIDTH) Left(210) E1(0);

E1(x):
{
	if (x == 1)
		produce IncColor() SB() F(LENGTH) E1(0) EB() 
			Right(60) RollR(180)
			SB() F(LENGTH) E1(0) EB();
}


