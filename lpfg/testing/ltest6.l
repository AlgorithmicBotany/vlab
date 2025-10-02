#include <lpfgall.h>

derivation length: 1;


module A(float);
module B;
module C(int);
module D;
module E;
module H;


Axiom: A(1.5f) B B C(6);


Start: { Backward(); }

VerifyString: A B B H;


B() B() < C(value) :
{
	produce H;
}

