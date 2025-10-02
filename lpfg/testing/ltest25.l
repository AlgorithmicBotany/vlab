#include <lpfgall.h>

derivation length: 1;


module A;
module B;
module C;
module D;
module E;
module H;


Axiom: A B SB C D E EB ;

Start: { Backward(); }


VerifyString: A H SB C D E EB;


B() : 
{
	produce H;
}

C() > D() :
{
	if (InNewLeftContext(H()))
		produce A;
}
