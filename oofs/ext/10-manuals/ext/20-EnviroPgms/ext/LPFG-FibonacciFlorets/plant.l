#include <lpfgall.h>
#include <math.h>

#define RADIUS 0.45	// ray floret radius
#define PETALSIZE 1.0

#define SCALEX 0.5
#define SCALEY 3.0

module M();	// generator of flowerheads
module X();	// delay for femoving cutting previous flowerhead
module A(int,int);
module C(int,int);
module Floret(int,int);	// floret with a given number (int) from a given total (int)
module Petal();		// draws a petal
module Disc(float);	// draws the receptacle
module H();		// Draws the histogram
module Score(int,int);	// Stores the record: (total, count) 
module Axes();		// Draws the axes
module PlotXAxis(float,float,float);
module PlotYAxis(float,float,float);
module MyLabel(float);

int count;
int total;

derivation length: 2;

Start: 
{
	total=0; count=0;
	UseView(FlowerHead);
	UseView(Histogram);
}
StartEach: {count=0;}
EndEach: {Printf("Total = %d, ray florets = %d\n", total, count); total++;}

Axiom: SB() Axes() EB() SB() SB() TurnAround() f(3) EB() SB() f(1) Left(90) f(3) EB() M() EB() H();

M():
{
	produce M() X() Disc(0.5*sqrt(total)) A(0,total);
}

X():
{
	produce Cut();
}

C(n,t) E1(x):
{
	if (x == 0)
		produce Floret(n,t);
	else
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
		produce SB() Right((t-n)*137.5) f(0.5*sqrt((float) n))
			C(t-n-1,t) E1(n) EB()
			A(n+1,t);
}

interpretation:
maximum depth: 25;

vgroup FlowerHead:
	
Floret(n,t):
{
	produce SetColor(1+ 192 * n / t) Circle(RADIUS);
}

Petal():
{
	produce	SetColor(192+count) Left(0) Circle(RADIUS) Surface(0, PETALSIZE);
		
}

Disc(r):
{
	produce SB() SetColor(253) Circle(r) EB();
}

vgroup Histogram:

Axes():
{
	produce MoveTo(0.0f, 0.0f, 0.0f) SetColor(254)
		PlotXAxis(0,120,10)
		PlotYAxis(0,22,1);
		
}

Score(t,c):
{
	V3f point;
	point.x = SCALEX*t;
	point.y = SCALEY*c;
	point.z = 0.0;
	if (t==0)
		produce MoveTo3f(point);
	else
		produce SetWidth(2.0) SetColor(192+c) LineTo3f(point);
}

MyLabel(x) :
{
		static char textstr[64];
		sprintf(textstr, "%.0f", x);
		produce Label(textstr);
}

PlotXAxis(xmin,xmax,xstep) :
{
	float x;	
	nproduce SB SetColor(254) SetWidth(2) Right(90) ;
	for (x=xstep; x<=xmax; x+=xstep) {
		nproduce F(SCALEX*xstep) 
                SB SetWidth(1) Left(90) SetColor (255) F(66) EB
		SB SetWidth(1) Left(90) F(0.35) EB 
		SB SetWidth(1) Right(90) F(0.35) EB
		SB Right (90) f(2.8) Right (90) f(0.25) SetColor(254) MyLabel(x) EB;
	 }
	nproduce F(SCALEX*xstep*0.6) SP PP SB Right (160) F(1.0) EB
		 SB Left(160) F(1.0) EB EP 
		 Left(180) f(9.0) Right (90) f(1.5) SetColor(254) Label("total florets");
	nproduce EB ;
	nproduce SB SetColor(254) SetWidth(2) Left(90) ;
	for (x=-xstep; x>=xmin; x-=xstep) {
		nproduce F(SCALEX*xstep) 
		SB Left(90) F(0.35) EB 
		SB Right(90) F(0.35) EB
		SB Left(90) f(1.8) Right(90) f(0.5) SetColor(254) MyLabel(x) EB;
	 }
	nproduce F(1) ;
	produce EB ;
}
		
PlotYAxis(ymin,ymax,ystep) :
{
	float y;	
	nproduce SB SetColor(254) SetWidth(2) ;
	for (y=ystep; y<=ymax; y+=ystep) {
		nproduce F(SCALEY*ystep) 
		SB SetWidth(1) Left(90) F(0.35) EB 
		SB SetWidth(1) Right(90) SetColor (255) F(60) EB
		SB SetWidth(1) Right(90) F(0.35) EB
		SB Right(90) f(-3.0) Right (90) f(0.25) Left(180) SetColor(254) MyLabel(y) EB;
	 }
	nproduce F(3) SP PP SB Right (160) F(1.0) EB
		 SB Left(160) F(1.0) EB EP
		 f(-0.9) Right (90) f(1.5) SetColor(254) Label("ray florets");
	nproduce EB ;
	nproduce SB SetColor(254) SetWidth(2) TurnAround() ;
	for (y=-ystep; y>=ymin; y-=ystep) {
		nproduce F(SCALEY*ystep) 
		SB Left(90) F(0.35) EB 
		SB Right(90) F(0.35) EB
		SB Left(90) f(1.0) Right (90) f(0.25) Left(180) SetColor(254) MyLabel(y) EB;
	 }
	nproduce F(ystep) ;
	produce EB ;
}
