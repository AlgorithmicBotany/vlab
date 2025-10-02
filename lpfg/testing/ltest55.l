#include <lpfgall.h>


module A;
module X;
module Y;
module Z;

derivation length: 1;

Start: { Backward(); }


Axiom: X Y Z;

Y() Z() :
{
	produce A();
}



VerifyString: X A;


