#include <lpfgall.h>

derivation length: 1;


Axiom: F(1) Left(10) f(5);


VerifyString: G Left f;


F(v) :
{
	if (v>0.9f)
		produce G(1);
}
