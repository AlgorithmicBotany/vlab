#include <lpfgall.h>

derivation length: 1;


module A;
module B(float);
module C;
module D;
module E;
module H;


Axiom: A B(1.0f) B(2.0f) C;

Start: { Backward(); }


VerifyString: H B B C;


A() :
{
	float v1, v2;
	if (InRightContext(B(v1) B(v2)))
	{
		if (v1 + v2 > 2.9f)
			produce H;
	}
}
