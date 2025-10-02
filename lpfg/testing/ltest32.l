#include <lpfgall.h>

derivation length: 1;


Axiom: F(1) Left(10) f(5);


VerifyString: G Left g;


F(v) :
{
	if (v>0.9f)
		produce G(1);
}

f(v) :
{
	if (v>4.9f)
	{
		produce g(1);
	}
}
