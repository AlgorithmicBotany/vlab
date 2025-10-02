#include <lpfgall.h>

derivation length: 1;


module A;
module B;
module C;
module D;
module E;

Axiom: A B C;

VerifyString: A B E;

consider: B;

A() > C() :
{
	produce D;
}

ignore: B;

A() < C() :
{
	produce E;
}

