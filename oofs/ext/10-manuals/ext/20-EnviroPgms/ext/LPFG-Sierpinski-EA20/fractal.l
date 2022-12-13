#include <lpfgall.h>

#define LENGTH 1
#define WIDTH 2

EA20Array ea = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};



derivation length: 15;

Axiom: 	SetWidth(WIDTH) Left(210) EA20(ea);

EA20(x):
{
	if (x[0] == 1)
		produce IncColor() SB() F(LENGTH) EA20(ea) EB() 
			Right(60) RollR(180)
			SB() F(LENGTH) EA20(ea) EB();
}


