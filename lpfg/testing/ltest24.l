#include <lpfgall.h>

derivation length: 1;


module A;
module B;
module C;
module D;
module E;
module H;


Axiom: A B SB C D E EB ;

Start: { Forward(); }


VerifyString: A H SB A D E EB;


B() : 
{
	produce H;
}

C() > D() :
{
	if (InRightContext(E()) && InNewLeftContext(H()))
		produce A;
}
