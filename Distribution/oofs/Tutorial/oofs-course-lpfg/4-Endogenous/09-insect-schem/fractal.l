#include <lpfgall.h>

#define STEPS 200
#define IL 3	/* number of internodes in a mature left branch */
#define IR 5	/* number of internodes in a mature right branch */
#define P 5	/* plastochron (in simulation steps) */
#define DEL 35	/* delay before insect appears at the base */

#define WIDTH 5
#define LENGTH 1
#define ANGLE 45

derivation length: STEPS;

module W(int);
module A(int,int);
module U();
module D();
module V();

Axiom:  SetWidth(WIDTH) W(DEL) F(LENGTH) A(0,IL);


/* Create a branching structure */

F(x) < A(t,n):
{
	if (t > 0)
		produce A(t-1,n);
	else if (t == 0)
	{
		if (n > 0)
			produce F(x) A(P,n-1);
		else if (n == 0)
			produce SB() Left(ANGLE) F(x) A(P,IL) EB()
				SB() Right(ANGLE) F(x) A(P,IR) EB();
	}
}

/* Insert an insect at the base after a wait period */

W(t):
{
	if (t > 0)
		produce W(t-1);
	else if (t == 0)
		produce U();
}

/* Insect U moves upwards */

U() < F(x):
{
	produce F(x) U();
}

/* In a branching point, insect U goes into left branch */

U() < Left(angle):
{
	produce Left(angle) U();
}

/* Having reached an apex, insect U removes it and 
   becomes a D */

U() < A(t,n):
{
	produce D();
}

/* Insect D moves downwards */

F(x) > D():
{
	produce D() F(x);
}

/* D insect returning from the left branch becomes 
   temporarily V and goes into the right branch */

F(x) > SB() Left(angle) D() EB():
{
	produce F(x) V();
}

V() < Right(angle):
{
	produce Right(angle) U();
}

/* D insect returning from the right branch keeps 
   going down */

F(x) > SB() EB() SB() Right(angle) D() EB():
{
	produce F(x) D();
}

/* houskeeping - removing the insect from the previous step from the string */

U(): { produce; }
V(): { produce; }
D(): { produce; }

interpretation:

A(t,n): { produce SB() SetColor(2) Sphere(0.5) EB(); }

U(): { produce SB() SetColor(3) Sphere(0.7) EB(); }

V(): { produce SB() SetColor(4) Sphere(0.7) EB(); }

D(): { produce SB() SetColor(5) Sphere(0.7) EB(); }

