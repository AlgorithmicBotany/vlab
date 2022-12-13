#include <lpfgall.h>

#define PL 1 /* plastochron */
#define DEL 15 /* delay for introducing florigen at the base */

#define LENGTH 1
#define WIDTH 5
#define ANGLE 45.0

derivation length: 50;

module D(int);
module A(int);
module I(int,int);
module K();
module B();
module C(int);

ignore: Left Right RollR;

Axiom: SetWidth(WIDTH) D(0) I(0,0) A(0);

I(fg,a) < A(t):
{
	if(fg == 0)
/* apex not reached by florigen produces internodes with
   and the associated (dormant) buds with plastochron PL */ 
	{
		if (t < PL)
			produce A(t+1);
		else if (t == PL)
			produce SB() Right(ANGLE) B() EB()
				I(0,0) RollR(180) A(0);
	}
	else if (fg == PL)
/* apex reached by florigen is transformed into a 
   flowering bud */
		produce K();
}

/* introduction of florigen at the base after a delay */

D(t):
{
	produce D(t+1);
}

D(t) < I(fg,a):
{
	if (t == DEL)
		produce I(1,a);
}

/* introduction of an auxin-related signal at the top */

I(fg,a) > K():
{
	produce I(fg,1);
}

/* propagation of florigen (up) and/or auxin (down) */

I(fl,al) < I(fg,a) > I(fr,ar):
{
	produce I(fl,ar);
}

/* special cases - first and last internode */

I(fl,al) < I(fg,a) > A(t):
{
	produce I(fl,a);
}

D(t) < I(fg,a) > I(fr,ar):
{
	produce I(fg,ar);
}

/* dormant bud reached by the downward signal becomes 
   active */

I(fg,a) < B():
{
	if (a == 1)
		produce C(0);
}

/* active lateral bud produces internodes with the 
   plastochron PL */

C(t):
{
	if (t < PL)
		produce C(t+1);
	else
		produce F(LENGTH) C(0);
}


interpretation:

I(fg,a):
{
	produce SetColor(1+fg+a) F(LENGTH);
}

B():
{
	produce SetColor(1) Circle(0.3);
}

K():
{
	produce SetColor(4) Circle(1.0);
}
