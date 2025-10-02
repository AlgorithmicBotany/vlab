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

Axiom: SetWidth(WIDTH) D(0) I(0,0) A(0);

I(fg,a) < A(t):
{
	if(fg == 0)
/* apex not reached by florigen produces internodes with
   plastochron PL */ 
	{
		if (t < PL)
			produce A(t+1);
		else if (t == PL)
			produce I(0,0) A(0);
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


interpretation:

I(fg,a):
{
	produce SetColor(1+fg+a) F(LENGTH);
}

K():
{
	produce SetColor(4) Sphere(1.7);
}
