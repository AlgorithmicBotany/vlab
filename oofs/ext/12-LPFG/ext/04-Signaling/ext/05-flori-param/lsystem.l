#include <lpfgall.h>

#define PL 1 /* plastochron */
#define DEL 15 /* delay for introducing florigen at the base */

#define LENGTH 1
#define WIDTH 5

derivation length: 40;

module D(int);
module A(int);
module I(int);
module K();

Axiom: SetWidth(WIDTH) D(0) I(0) A(0);


I(s) < A(t):
{
	if (s == 0 )
/* apex not reached by florigen produces internodes with
   plastochron PL */ 
	{
		if (t < PL)
			produce A(t+1);
		else if (t == PL)
			produce I(0) A(0);
	}
	else if (s == 1)
/* apex reached by florigen is transformed into a 
   flowering bud */
		produce K();
}


/* introduction of florigen at the base after a delay */

D(t):
{
	produce D(t+1);
}

D(t) < I(s):
{
	if (t == DEL)
		produce I(1);
}

/* propagation of florigen */

I(r) < I(s):
{
	produce I(r);
}

interpretation:

I(s):
{
	produce SetColor(s+1) F(LENGTH);
}

K():
{
	produce SetColor(3) Sphere(0.85);
}
