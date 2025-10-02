#include <stdio.h>
#include <lpfgall.h>

#define STEPS 400
#define dt 0.05

#define PL 0.500000
#define MAX_INT_LEN 0.500000
#define MAX_LEAF_LEN 1.500000
#define DURATION 20

derivation length: STEPS;

module A(float);
module I(float);
module L(float);

float angle;

Start:
{
  FILE* fp = fopen("angle.txt", "rt");
  fscanf(fp, "angle: %f", &angle);
  fclose(fp);
}

Axiom: SetWidth(0.05) A(0);

A(t): { produce A(t+dt); }
I(t): { produce I(t+dt); }
L(t): { produce L(t+dt); }

decomposition:

A(t):
{
	t=t-PL;

	if(t>0)
		produce
			I(t)
			SB() Left(angle) L(t) EB()
			SB() Right(angle) L(t) EB()
			A(t); 
}

interpretation:

I(t): { produce F(MAX_INT_LEN*func(internode_length,t/DURATION)); }

L(t): { produce IncColor() F(MAX_LEAF_LEN*func(leaf_length,t/DURATION)); }
