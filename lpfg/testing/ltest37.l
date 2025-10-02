#include <lpfgall.h>

derivation length: 1;


Axiom: F(-1) Left(-10) f(-5);


VerifyString: G Right g;

F(v) :
{ produce G(v / 2.0); }

G(lv) << Left(angle) :
{
	float fv;
	if (lv < -0.1 && InRightContext(f(fv)))
	{
		produce Right(angle/2.0);
	}
}

Left(angle) < f(v):
{
	if (v<-4.9f && angle<-9.9f)
		produce g(1);
}
