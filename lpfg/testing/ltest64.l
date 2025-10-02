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

interpretation:
maximum depth: 2;

X() < Y() Z() :
{
	produce A;
}

X() :
{
	produce F(1);
}

Y() :
{
	produce f(1);
}

Z() :
{
	produce Left(90);
}

A() :
{
	produce Right(90);
}


