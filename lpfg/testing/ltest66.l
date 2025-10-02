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


B() :
{
	if (InLeftContext(A()) && InNewLeftContext(D()))
		produce E;
}

VerifyString: D E C;


