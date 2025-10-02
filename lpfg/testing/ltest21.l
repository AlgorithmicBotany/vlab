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


VerifyString: D E C;


A() :
{
	produce D;
}


A() < B() :
{
	if (InNewLeftContext(D()))
	{
		produce E;
	}
}
