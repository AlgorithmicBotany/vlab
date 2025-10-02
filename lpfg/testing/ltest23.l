#include <lpfgall.h>

derivation length: 1;


module A;
module B;
module C;
module D;
module E;
module H;


Axiom: A B C D E;

Start: { Forward(); }


VerifyString: A H A D E;


B() : 
{
	produce H;
}

C() > D() :
{
	if (InRightContext(E()) && InNewLeftContext(H()))
		produce A;
}
