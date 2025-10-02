#include <lpfgall.h>


module A;
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

X() < Y() Z() :
{
	produce A;
}


VerifyString: X A Z;


