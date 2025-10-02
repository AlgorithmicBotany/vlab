#include <lpfgall.h>

#define PL 1 /* plastochron */
#define DEL 15 /* delay for introducing florigen at the base */

#define LENGTH 1
#define WIDTH 5

derivation length: 40;

module A(int); // apex
module I(); // internode not reached by signal
module J(); // internode reached by signal
module D(int); // plant base (could be basal rosette) 
module K(); // flowering bud

ignore: Left Right;


Axiom: SetWidth(WIDTH) D(0) I() A(0);

/* apex not reached by florigen produces internodes with
   plastochron PL */ 

I() < A(t):
{
	if (t < PL)
		produce A(t+1);
	else if (t == PL)
		produce I() A(0);
}

/* apex reached by florigen is transformed into a 
   flowering bud */

J() < A(t):
{
	produce K();
}

/* introduction of florigen at the base after a delay */

D(t):
{
	produce D(t+1);
}

D(t) < I():
{
	if (t == DEL)
		produce J();
}


/* propagation of florigen */ 

J() < I():
{
	produce J();
}

interpretation:

I():
{
	produce SetColor(1) F(LENGTH);
}

J():
{
	produce SetColor(2) F(LENGTH);
}

K():
{
	produce SetColor(3) Sphere(0.85);
}
