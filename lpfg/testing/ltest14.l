#include <lpfgall.h>

derivation length: 1;


module A;
module B(float);
module C;
module D;
module E;
module H;


Axiom: A B(0.1f) B(0.9f) C;


Start: { Backward(); }

VerifyString: E B B C;


A() > B(bp1) :
{
	float bp2;
	if (InRightContext(B(bp2)))
	{
		if (bp2>0.85f)
			produce E;
	}
}

