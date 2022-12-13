#include <lpfgall.h>
#include <math.h>

#define STEPS 800
#define dt 0.05

#define PL 1.500000
#define DURATION 15
#define TOTAL 15

#define RANDOM_RANGE(lo, hi) ((lo) + (hi - lo) * float_rand())
static float float_rand(void) { return rand() / (float) RAND_MAX; }

#define ANGLE RANDOM_RANGE(78,82)
#define INT_SIZE(n) func(internode_pos,n/TOTAL)
#define LEAF_SIZE(n) func(leaf_pos,n/TOTAL)

derivation length: STEPS;

module A(float, float);
module I(float, float);
module L(float, float);

Axiom: Right(5) SetWidth(0.05) A(0, 0);

A(t,n): { produce A(t+dt,n); }
I(t,x): { produce I(t+dt,x); }
L(t,x): { produce L(t+dt,x); }

decomposition:

A(t, n):
{
	t=t-PL;

	if(t>0 && n<TOTAL)
		produce
			I(t, INT_SIZE(n))
			SB() Left(ANGLE) L(t, LEAF_SIZE(n)) EB()
			SB() Right(ANGLE) L(t, LEAF_SIZE(n)) EB()
			A(t, n+1); 

	else if(t>0 && n>=TOTAL)
		produce
			I(t, INT_SIZE(n)) L(t, LEAF_SIZE(n));
}

interpretation:

I(t,x): { produce F(x*func(internode_length,t/DURATION)); }
L(t,x): { produce IncColor() Surface(0, x*func(leaf_length,t/DURATION)); }
