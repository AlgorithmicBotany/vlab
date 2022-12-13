#include <lpfgall.h>
#include <math.h>

#define LENGTH 1
#define ANGLE 30

#define NUMBER 10
#define RADIUS 0.45

module M();	// generator of flowerheads
module X();
module A(int,int);
module C(int,int);
module Petal();		// draws a petal

int count;
int total;

derivation length: NUMBER;

Start: {total=0; count=0;}
StartEach: {total++; count=0;}
EndEach: {Printf("Total = %d, ray florets = %d\n",total, count);}

Axiom: M();

M():
{
	produce M() X() A(0,total);
}

X():
{
	produce Cut();
}

C(n,t) > E1(x):
{
	if (x == 0)
		produce SetColor(1+ (int) floor(float(253*n/t))) Circle(RADIUS);
	else if (x == 1)
	{
		count++;
		produce Petal() ;
	}
}

decomposition:

maximum depth: 1000;

A(n,t):
{
	if (n <= t)
		produce SB() Right(n*137.5) f(0.5*sqrt(float(n)))
			C(n,t) E1(n) EB()
			A(n+1,t);
}

interpretation:

Petal():
{
	produce	SetColor(255)
		SP() Right(ANGLE) F(LENGTH) Left(ANGLE) 
		F(LENGTH) Left(ANGLE) F(LENGTH) Right(ANGLE) 
		TurnAround()
		Right(ANGLE) F(LENGTH) Left(ANGLE) 
		F(LENGTH) Left(ANGLE) F(LENGTH)
		EP();
}
