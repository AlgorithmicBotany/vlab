#include <lpfgall.h>

derivation length: 1;


module A;
module B(float);
module C;
module D;
module E;
module H;


Axiom: A B(0.5f) C;


Start: { Backward(); }

VerifyString: D B C;


A() :
{
	float bParam;
	if (InRightContext(B(bParam)))
	{
		if (bParam>0.4f)
		{
			produce D;
		}
	}
}
