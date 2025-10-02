#include <lpfgall.h>

derivation length: 1;


module A;
module B;
module C;
module D;
module E;
module H;


Axiom: A B C D E F(1.0);


ignore: C;

VerifyString: A H C D H F;


B() :
{
	if (InRightContext(D() E()))
		produce H;
}

D() < E() :
{
	float l;
	if (InRightContext(F(l)) && InLeftContext(B()))
		produce H;
}

