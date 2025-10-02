#include <lpfgall.h>

derivation length: 1;


module A;
module B;
module C;
module D;
module E;
module H;


Axiom: A B B C;

Start: { Backward(); }


VerifyString: H B B C;


A() > B() :
{
	if (InRightContext(B() C()))
	{
		produce H;
	}
}

