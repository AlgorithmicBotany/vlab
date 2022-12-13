#include <lpfgall.h>
#include <math.h>

#define STEPS 450
#define dt 0.05

#define PL 0.500000
#define MAX_INT_LEN 0.500000

derivation length: STEPS;

module A(float);
module I(float);
module L(float);

Axiom: Right(5) SetWidth(0.15) A(0);

production:

A(t):{ produce A(t+dt); }
I(t):{ produce I(t+dt); }
L(t):{ produce L(t+dt); }

decomposition:

A(t):
{
	t = t - PL;
	
	if(t>0)
		produce
			I(t)
			SB() L(t) EB()
			RollL(137.5) 
			A(t);
}

interpretation:

I(t): { produce F(MAX_INT_LEN*func(internode_length, t/15)); }

L(t):
{
	float length = func(leaf_length, t/10);
	float width = func(leaf_width, t/20);
	float ang = 90*func(branching_angle, t/30);
	float col = 32+floor(31*func(color, t/20));

	produce
		Down(ang) SetColor(col) RollL(180) Surface3(0, width, length, length);
				
}
