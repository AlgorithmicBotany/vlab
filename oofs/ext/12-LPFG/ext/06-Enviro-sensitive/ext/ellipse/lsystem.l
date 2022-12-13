#include <memory.h>
#include <math.h>
#include <lpfgall.h>

#define RB 10.0
#define WIDTH 4.0
#define R 10.0

derivation length: 24;

module A();
module B();
module E(int);

Axiom: SetWidth(3.0) E(0) A;

A() : 
{
	produce 
	SB Left(60.0) B EB
	SB Right(60.0) B EB
	F(1.0) GetPos(0,0,0) A;
}

B() :
{
	produce SetColor(2) F(1.0) GetPos(0,0,0) SetColor(1) Circle(0.2) B;
}

bool Inside(double x, double y)
{
  double d = WIDTH*x*x+(y-R)*(y-R);
  return d <= R*R;
}


GetPos(x,y,z) :
{
	if (!Inside(x,y))
		produce SB Left(2*y) F(1.0) EB
			SB Right(2*y) F(1.0) EB Cut;
}


decomposition:
maximum depth: 360;

E(n) :
{
  if (n<360)
  {
    double ex = RB/2.0*sin(1.0*n);
    double ey = 10.4+10.4*cos(1.0*n);
    produce SB f(ey) Left(90) f(ex) SetColor(3) Circle(0.2) EB E(n+1);
  }
}


interpretation:

A() : { produce SetColor(2) F(1.0); }

B() : { produce SetColor(2) F(1.0); }
