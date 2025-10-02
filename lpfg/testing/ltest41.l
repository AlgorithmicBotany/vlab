#include <lpfgall.h>

derivation length: 1;

Start: { Backward(); }


Axiom: F(1) f(1) G(1) g(1);


G(l1)g(l2) :
{
	produce Left(l1+l2);
}


VerifyString: F f Left;

