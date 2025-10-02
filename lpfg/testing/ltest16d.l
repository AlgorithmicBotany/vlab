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


VerifyString: A B H;


C() :
{
	if (InLeftContext(A() B()))
	{
		produce H;
	}
}

