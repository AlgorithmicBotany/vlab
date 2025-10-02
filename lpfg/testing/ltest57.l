#include <lpfgall.h>


module A;
module X;
module Y;
module Z;

derivation length: 1;

Axiom: X Y Z;


decomposition:

Y() > Z() :
{
	produce A;
}


VerifyString: X A Z;


