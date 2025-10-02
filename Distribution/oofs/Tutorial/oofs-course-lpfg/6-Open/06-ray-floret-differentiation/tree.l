#include <lpfgall.h>
#include <math.h>

#define LENGTH 1
#define ANGLE 30

#define NUMBER 500
#define RADIUS 0.45

derivation length: 2;

module A(int);
module C(int);

Axiom: A(0);

C(n) > E1(x):
{
	if (x == 0)
		produce SetColor(1+floor(253.0f*n/NUMBER))
			Circle(RADIUS);
	else if (x == 1)
		produce SetColor(255)
			SP() Right(ANGLE) F(LENGTH)
			      Left(ANGLE) F(LENGTH)
			      Left(ANGLE) F(LENGTH)
			     Right(ANGLE) TurnAround()
			     Right(ANGLE) F(LENGTH)
			      Left(ANGLE) F(LENGTH)
			      Left(ANGLE) F(LENGTH)
			EP();
}



decomposition:

maximum depth: 1000;

A(n):
{
	if (n < NUMBER)
		produce SB() Right(n*137.5) f(0.5f*sqrt(1.0f*n))
			     C(n) E1(n) EB()
			A(n+1);
}

