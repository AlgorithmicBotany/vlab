#include <lpfgall.h>

derivation length: 1;


module A;
module B;
module C;
module D;
module E;
module P;

Axiom: A B C D;

VerifyString: E B C P;

consider: D;

A() > D() :
{
	produce E;
}

ignore: C;

A() B() < D() :
{
	produce P;
}

