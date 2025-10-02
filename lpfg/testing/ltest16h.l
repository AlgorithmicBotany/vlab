#include <lpfgall.h>

derivation length: 1;


module A;
module B;
module C;
module D;
module E;
module H;


Axiom: A B C;

Start: { Backward(); }


VerifyString: H B C;


A() :
{
	if (InRightContext(B() C()))
	{
		produce H;
	}
}

