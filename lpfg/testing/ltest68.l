#include <lpfgall.h>

derivation length: 1;


module A;
module B;
module C;
module D;
module E;
module H;


Axiom: A B C;

Start: { Forward(); }

A() : { produce D; }


bool TestContext()
{
	return InLeftContext(A()) && InNewRightContext(D());
}

B() :
{
	if (TestContext())
		produce E;
}

VerifyString: D B C;


