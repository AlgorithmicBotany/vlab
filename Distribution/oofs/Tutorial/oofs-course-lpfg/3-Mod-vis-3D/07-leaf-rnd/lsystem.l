#include <lpfgall.h>
#include <math.h>

#define STEPS 800
#define dt 0.05

#define PL 1.500000
#define DURATION 15
#define TOTAL 10

#define RANDOM_RANGE(lo, hi) ((lo) + (hi - lo) * float_rand())
static float float_rand(void) { return rand() / (float) RAND_MAX; }

#define ANGLE RANDOM_RANGE(78,82)
#define INT_SIZE RANDOM_RANGE(0.8,1.2)
#define LEAF_SIZE RANDOM_RANGE(0.6,0.8)

derivation length: STEPS;

module A(float, float);
module I(float, float);
module L(float, float, float);

Axiom: SetWidth(0.1) A(0, 0);

production:

A(t, n): { produce A(t+dt, n); }
I(t, x): { produce I(t+dt, x); }
L(t, n, x): { produce L(t+dt, n, x); }

decomposition:

A(t, n):
{
	t=t-PL;

	if(t>0 && n<TOTAL)
		produce
			I(t, INT_SIZE)
			SB() Left(ANGLE) L(t, n, LEAF_SIZE) EB()
			SB() Right(ANGLE) L(t, n, LEAF_SIZE) EB()
			A(t, n+1); 

	else if(t>0 && n>=TOTAL)
		produce
			I(t, 0.5) L(t, n, LEAF_SIZE);
}

interpretation:

I(t, x): { produce F(x*func(internode_length,t/DURATION)); }
L(t, n, x): { produce IncColor() Surface(0, x*func(leaf_length,t/DURATION)); }

