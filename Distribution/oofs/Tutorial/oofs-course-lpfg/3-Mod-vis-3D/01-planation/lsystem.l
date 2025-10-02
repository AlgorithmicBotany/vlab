#include <lpfgall.h>

#define STEPS 1000
#define INTERVAL 60

#define T0 50
#define T1 230
#define T2 310
#define T3 400
#define T4 430
#define T5 645
#define T6 725
#define T7 815
#define T8 840
#define T9 990

int n = 0;			// step counter

derivation length: STEPS;

module A();			// apex
module I();			// internode
module Rotate(float,int);	// roll(angle,case)

Start: {n=1;}
StartEach: {n=n+1;}

Axiom: Rotate(0,2) A();

A():
{
	if((n%INTERVAL)==2 && n> T0 && n<T1)
		produce
			I()
			SB() Left(45) Rotate(0,1) A() EB()
			SB() Right(45) Rotate(0,1) A() EB()
			I() A();
}

I():
{
	if((n%INTERVAL)==2 && n> T0 && n<T1)
		produce I()I();
}

Rotate(a, c):
{
	if ((c==1 && n>T2 && n<=T3) || 
	    (c==2 && n>T4 && n<=T5) ||
	    (c==2 && n>T8 && n<=T9))
		produce Rotate(a+1, c);
	if (c==1 && n>T6 && n<=T7)
		produce Rotate(a-1, c);
}

interpretation:

A(): { produce SetColor(2) SetWidth(5) F(1); }

I(): { produce SetColor(1) SetWidth(2.5) F(1); }

Rotate(a,c): { produce RollL(a); }
