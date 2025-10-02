#include <lpfgall.h>
#include <math.h>
#include <stdio.h>

#define LENGTH 1
#define ANGLE 30

#define NUMBER 100
#define RADIUS 0.45


module M();
module X();
module A(int,int);
module C(int,int);

int total,count;
FILE* fp;

derivation length: NUMBER;

Start: {
	total=0; count=0;
	fp = fopen("output.txt","w");
	}
StartEach: {total++; count=0;}
EndEach: {fprintf(fp,"Total = %d, ray florets = %d\n",total, count);}
End: {fclose(fp);}


Axiom: M();

M():
{
	produce M() X() SB() A(0,total) EB();
}

X():
{
	produce Cut();
}


C(n,t) > E1(x):
{
	if (x == 0)
		produce SetColor(1+floor(253.0f*n/t)) Circle(RADIUS);
	else if (x == 1)
	{
		count++;
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
}



decomposition:

maximum depth: 1000;

A(n,t):
{
	if (n <= t)
		produce SB() Right(n*137.5) f(0.5f*sqrt(1.0f*n))
			C(n,t) E1(n) EB()
			A(n+1,t);
}

