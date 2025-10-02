#include <lpfgall.h>


Start: { UseGroup(1); }

module Init(int);
module A;
module X;
module Y;
module Z;

derivation length: 1;

const int kCount = 10;

Axiom: Init(kCount);

ggroup 1:

A() :
{
	propensity (1) produce X;
	propensity (2) produce Y;
	propensity (3) produce Z;
}


group 0:

decomposition:

Init(items) :
{
	for (int item=0; item<items; ++item)
		nproduce A;
	produce;
}
