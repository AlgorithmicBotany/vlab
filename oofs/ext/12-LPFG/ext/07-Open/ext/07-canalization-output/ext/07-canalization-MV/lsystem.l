#include <lpfgall.h>
#include <math.h>

#define RADIUS 0.45	// ray floret radius
#define PETALSIZE 1.0

module M();	// generator of flowerheads
module X();	// delay for femoving cutting previous flowerhead
module A(int,int);
module C(int,int);
module Floret(int,int);	// floret with a given number (int) from a given total (int)
module Petal();		// draws a petal
module H();		// Draws the histogram
module Score(int,int);	// Stores the record: (total, count) 

int count;
int total;

derivation length: 2;

Start: 
{
	total=0; count=0;
	UseView(FlowerHead);
	UseView(Histogram);
}
StartEach: {total++; count=0;}
EndEach: {Printf("Total = %d, ray florets = %d\n", total, count);}

Axiom: SB() SB() f(3) EB() M() EB() H();

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
		produce Floret(n,t);
	else if (x == 1)
	{
		count++;
		produce Petal() ;
	}
}

H():
{
	produce Score(total, count) H();
}

decomposition:

maximum depth: 1000;

A(n,t):
{
	if (n <= t)
		produce SB() Right(n*137.5) f(0.5*sqrt((float) n))
			C(n,t) E1(n) EB()
			A(n+1,t);
}

interpretation:

vgroup FlowerHead:
	
Floret(n,t):
{
	produce SetColor(1+ 192 * n / t) Circle(RADIUS);
}

Petal():
{
	produce	SetColor(192+count) Surface(0, PETALSIZE);
		
}

vgroup Histogram:

H():
{
	produce MoveTo(0.0f, 0.0f, 0.0f) SetColor(254)
		SB() SetWidth(1.0) F(50) EB()
		SB() Right(90) SetWidth(1.0) F(125) EB();
		
}

Score(t,c):
{
	V3f point;
	point.x = t;
	point.y = 2*c;
	point.z = 0.0;
	produce SetWidth(2.0) SetColor(192+c) LineTo3f(point);
}
