#include <lpfgall.h>

derivation length: 1;

Start: { Backward(); }


Axiom: F(-1) Left(-10) f(-5);


VerifyString: G Right g;

F(v) :
{ produce G(v / 2.0); }

Left(angle) >> g(rv):
{
	float fv;
	if (rv > 0.9 && InRightContext(f(fv)))
	{
		produce Right(angle/2.0);
	}
}

Left(angle) < f(v):
{
	if (v<-4.9f && angle<-9.9f)
		produce g(1);
}
