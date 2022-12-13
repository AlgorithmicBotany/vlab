#include <lpfgall.h>
#include <math.h>

#define NUMBER 500
#define RADIUS 0.45

module A(int);
module C(int);

derivation length: 2;

Axiom: A(0);

C(n):
{
	produce SetColor(1+floor(254*n/NUMBER)) Circle(RADIUS);
}


decomposition:
maximum depth: 1000;

A(n):
{
	if (n < NUMBER)
		produce SB() Right(n*137.5) f(0.5*sqrt(n))
			C(n) EB()
			A(n+1);
}
