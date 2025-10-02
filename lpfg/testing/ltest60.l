#include <lpfgall.h>


module A;
module B;
module X;
module Y;
module Z;

derivation length: 1;

Axiom: A Y Z;

A() > Y() :
{
	produce X;
}

decomposition:

X() > Y():
{
	produce B;
}


VerifyString: B Y Z;


